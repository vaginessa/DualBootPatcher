/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * This is a slight adaptation of TWRP's implementation. Original source code is
 * available at the following link: https://gerrit.omnirom.org/#/c/6019/
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdarg.h>
#include <dirent.h>
#include <limits.h>
#include <errno.h>

#include "legacy_properties.h"

#include <linux/futex.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#include "legacy_property_service.h"


// NDK platform 21 no longer has <sys/atomics.h>
extern "C" int
__futex_wake(volatile void *ftx, int count)
{
    int saved_errno = errno;
    int result = syscall(__NR_futex, ftx, FUTEX_WAKE, count, NULL);
    if (__builtin_expect(result == -1, 0)) {
        result = -errno;
        errno = saved_errno;
    }
    return result;
}


// Copied from libcutils/properties.h because we don't link against or even
// have libcutils

typedef void (*propfn)(const char *key, const char *value, void *cookie);

struct property_list_callback_data
{
    propfn fn;
    void *cookie;
};

static void property_list_callback(const prop_info *pi, void *cookie)
{
    char name[PROP_NAME_MAX];
    char value[PROP_VALUE_MAX];
    property_list_callback_data *data = (property_list_callback_data *) cookie;

    __system_property_read(pi, name, value);
    data->fn(name, value, data->cookie);
}

static int property_list(propfn fn, void *cookie)
{
    property_list_callback_data data = { fn, cookie };
    return __system_property_foreach(property_list_callback, &data);
}


static int property_area_inited = 0;

typedef struct {
    void *data;
    size_t size;
    int fd;
} workspace;

static int init_workspace(workspace *w, size_t size)
{
    void *data;
    int fd;

    // dev is a tmpfs that we can use to carve a shared workspace
    // out of, so let's do that...
    fd = open("/dev/__legacy_properties__", O_RDWR | O_CREAT, 0600);
    if (fd < 0) {
        return -1;
    }

    if (ftruncate(fd, size) < 0) {
        goto out;
    }

    data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        goto out;
    }

    close(fd);

    fd = open("/dev/__legacy_properties__", O_RDONLY);
    if (fd < 0) {
        return -1;
    }

    unlink("/dev/__legacy_properties__");

    w->data = data;
    w->size = size;
    w->fd = fd;
    return 0;

out:
    close(fd);
    return -1;
}

/* (8 header words + 247 toc words) = 1020 bytes */
/* 1024 bytes header and toc + 247 prop_infos @ 128 bytes = 32640 bytes */

#define PA_COUNT_MAX  247
#define PA_INFO_START 1024
#define PA_SIZE       32768

static workspace pa_workspace;
static prop_info *pa_info_array;

prop_area *__legacy_property_area__;

static int init_property_area(void)
{
    prop_area *pa;

    if (pa_info_array) {
        return -1;
    }

    if (init_workspace(&pa_workspace, PA_SIZE)) {
        return -1;
    }

    fcntl(pa_workspace.fd, F_SETFD, FD_CLOEXEC);

    pa_info_array = (prop_info *)
            (((char *) pa_workspace.data) + PA_INFO_START);

    pa = (prop_area *) pa_workspace.data;
    memset(pa, 0, PA_SIZE);
    pa->magic = PROP_AREA_MAGIC;
    pa->version = PROP_AREA_VERSION;

    // plug into the lib property services
    __legacy_property_area__ = pa;
    property_area_inited = 1;
    return 0;
}

static void update_prop_info(prop_info *pi, const char *value, unsigned len)
{
    pi->serial = pi->serial | 1;
    memcpy(pi->value, value, len + 1);
    pi->serial = (len << 24) | ((pi->serial + 1) & 0xffffff);
    __futex_wake(&pi->serial, INT32_MAX);
}

static const prop_info *__legacy_property_find(const char *name)
{
    prop_area *pa = __legacy_property_area__;
    unsigned count = pa->count;
    unsigned *toc = pa->toc;
    unsigned len = strlen(name);
    prop_info *pi;

    while (count--) {
        unsigned entry = *toc++;
        if (TOC_NAME_LEN(entry) != len) {
            continue;
        }

        pi = TOC_TO_INFO(pa, entry);
        if (memcmp(name, pi->name, len) != 0) {
            continue;
        }

        return pi;
    }

    return 0;
}

static int legacy_property_set(const char *name, const char *value)
{
    prop_area *pa;
    prop_info *pi;

    int namelen = strlen(name);
    int valuelen = strlen(value);

    if (namelen >= PROP_NAME_MAX
            || valuelen >= PROP_VALUE_MAX
            || namelen < 1) {
        return -1;
    }

    pi = (prop_info *) __legacy_property_find(name);

    if (pi != 0) {
        // ro.* properties may NEVER be modified once set
        if (strncmp(name, "ro.", 3) == 0) {
            return -1;
        }

        pa = __legacy_property_area__;
        update_prop_info(pi, value, valuelen);
        pa->serial++;
        __futex_wake(&pa->serial, INT32_MAX);
    } else {
        pa = __legacy_property_area__;
        if (pa->count == PA_COUNT_MAX) {
            return -1;
        }

        pi = pa_info_array + pa->count;
        pi->serial = (valuelen << 24);
        memcpy(pi->name, name, namelen + 1);
        memcpy(pi->value, value, valuelen + 1);

        pa->toc[pa->count] =
                (namelen << 24) | (unsigned) ((uint64_t) pi - (uint64_t) pa);

        pa->count++;
        pa->serial++;
        __futex_wake(&pa->serial, INT32_MAX);
    }

    return 0;
}

void legacy_get_property_workspace(int *fd, int *sz)
{
    *fd = pa_workspace.fd;
    *sz = pa_workspace.size;
}

static void copy_property_to_legacy(const char *key, const char *value,
                                    void *cookie __attribute__((unused)))
{
    legacy_property_set(key, value);
}

void legacy_properties_init()
{
    init_property_area();
    property_list(copy_property_to_legacy, 0);
}
