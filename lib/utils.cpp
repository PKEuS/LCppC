/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "utils.h"

#include <utility>
#include <stack>
#include <cctype>
#include <array>


bool isPrefixStringCharLiteral(const std::string& str, char q, const std::string& p)
{
    if (!endsWith(str, q))
        return false;
    if ((str.length() + 1) > p.length() && (str.compare(0, p.size() + 1, p + q) == 0))
        return true;
    return false;
}

static const std::array<std::string, 5> literal_suffixes{ "", "u8", "u", "U", "L" };
bool isStringCharLiteral(const std::string& str, char q)
{
    for (const std::string& p : literal_suffixes) {
        if (isPrefixStringCharLiteral(str, q, p))
            return true;
    }
    return false;
}

const char* getOrdinalText(int i)
{
    if ((i % 10) == 1 && (i % 100) != 11)
        return "st";
    if ((i % 10) == 2 && (i % 100) != 12)
        return "nd";
    if ((i % 10) == 3 && (i % 100) != 13)
        return "rd";
    return "th";
}

int caseInsensitiveStringCompare(const std::string &lhs, const std::string &rhs)
{
    if (lhs.size() != rhs.size())
        return (lhs.size() < rhs.size()) ? -1 : 1;
    for (unsigned int i = 0; i < lhs.size(); ++i) {
        const int c1 = std::toupper(lhs[i]);
        const int c2 = std::toupper(rhs[i]);
        if (c1 != c2)
            return (c1 < c2) ? -1 : 1;
    }
    return 0;
}

bool isValidGlobPattern(const std::string& pattern)
{
    for (std::string::const_iterator i = pattern.begin(); i != pattern.end(); ++i) {
        if (*i == '*' || *i == '?') {
            std::string::const_iterator j = i + 1;
            if (j != pattern.end() && (*j == '*' || *j == '?')) {
                return false;
            }
        }
    }
    return true;
}

bool matchglob(const std::string& pattern, const std::string& name)
{
    const char* p = pattern.c_str();
    const char* n = name.c_str();
    std::stack<std::pair<const char*, const char*> > backtrack;

    for (;;) {
        bool matching = true;
        while (*p != '\0' && matching) {
            switch (*p) {
            case '*':
                // Step forward until we match the next character after *
                while (*n != '\0' && *n != p[1]) {
                    n++;
                }
                if (*n != '\0') {
                    // If this isn't the last possibility, save it for later
                    backtrack.push(std::make_pair(p, n));
                }
                break;
            case '?':
                // Any character matches unless we're at the end of the name
                if (*n != '\0') {
                    n++;
                } else {
                    matching = false;
                }
                break;
            default:
                // Non-wildcard characters match literally
                if (*n == *p) {
                    n++;
                } else if (*n == '\\' && *p == '/') {
                    n++;
                } else if (*n == '/' && *p == '\\') {
                    n++;
                } else {
                    matching = false;
                }
                break;
            }
            p++;
        }

        // If we haven't failed matching and we've reached the end of the name, then success
        if (matching && *n == '\0') {
            return true;
        }

        // If there are no other paths to try, then fail
        if (backtrack.empty()) {
            return false;
        }

        // Restore pointers from backtrack stack
        p = backtrack.top().first;
        n = backtrack.top().second;
        backtrack.pop();

        // Advance name pointer by one because the current position didn't work
        n++;
    }
}
