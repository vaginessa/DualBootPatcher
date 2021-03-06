/*
 * Copyright (C) 2014  Andrew Gunnerson <andrewgunnerson@gmail.com>
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

namespace mb
{
namespace util
{

__attribute__((format(printf, 1, 2)))
std::string format(const char *fmt, ...);

bool starts_with(const std::string &string, const std::string &prefix);
bool starts_with(const char *string, const char *prefix);
bool ends_with(const std::string &string, const std::string &suffix);
bool ends_with(const char *string, const char *suffix);

void replace(std::string *source,
             const std::string &from, const std::string &to);
void replace_all(std::string *source,
                 const std::string &from, const std::string &to);

std::string join(std::vector<std::string> &list, std::string delim);

std::string hex_string(unsigned char *data, size_t size);

}
}
