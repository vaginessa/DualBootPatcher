/*
 * Copyright (C) 2015  Andrew Gunnerson <andrewgunnerson@gmail.com>
 *
 * This file is part of MultiBootPatcher
 *
 * MultiBootPatcher is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MultiBootPatcher is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MultiBootPatcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <string>
#include <vector>

// zu, zx, etc. are not supported until VS2015
#ifdef _WIN32
#define PRIzu "Iu"
#else
#define PRIzu "zu"
#endif

class StringUtils
{
public:
    __attribute__((format(printf, 1, 2)))
    static std::string format(const char *fmt, ...);

    static bool starts_with(const std::string &string, const std::string &prefix);
    static bool starts_with(const char *string, const char *prefix);
    static bool ends_with(const std::string &string, const std::string &suffix);
    static bool ends_with(const char *string, const char *suffix);
    static bool istarts_with(const std::string &string, const std::string &prefix);
    static bool istarts_with(const char *string, const char *prefix);
    static bool iends_with(const std::string &string, const std::string &suffix);
    static bool iends_with(const char *string, const char *suffix);

    static std::vector<std::string> splitData(const std::vector<unsigned char> &data,
                                              unsigned char delim);
    static std::vector<std::string> split(const std::string &str, char delim);
    static std::vector<unsigned char> joinData(std::vector<std::string> &list,
                                               unsigned char delim);
    static std::string join(std::vector<std::string> &list, char delim);

    static void replace(std::string *source,
                        const std::string &from, const std::string &to);
    static void replace_all(std::string *source,
                            const std::string &from, const std::string &to);
};
