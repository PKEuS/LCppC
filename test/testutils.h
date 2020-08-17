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

#ifndef TestUtilsH
#define TestUtilsH

#include "settings.h"
#include "tokenize.h"
#include "errorlogger.h"

class Token;

class givenACodeSampleToTokenize {
private:
    Tokenizer tokenizer;
    static const Settings settings;
    static const Project project;

public:
    explicit givenACodeSampleToTokenize(const char sample[], bool createOnly = false, bool cpp = true)
        : tokenizer(&settings, &project, nullptr) {
        std::istringstream iss(sample);
        if (createOnly)
            tokenizer.list.createTokens(iss, cpp ? "test.cpp" : "test.c");
        else
            tokenizer.tokenize(iss, cpp ? "test.cpp" : "test.c");
    }

    const Token* tokens() const {
        return tokenizer.tokens();
    }
};


class SimpleSuppressor : public ErrorLogger {
public:
    SimpleSuppressor(Project& project, ErrorLogger *next)
        : project(project), next(next) {
    }
    void reportOut(const std::string &outmsg) override {
        next->reportOut(outmsg);
    }
    void reportErr(const ErrorMessage &msg) override {
        if (!msg.callStack.empty() && !project.nomsg.isSuppressed(msg.toSuppressionsErrorMessage()))
            next->reportErr(msg);
    }
private:
    Project& project;
    ErrorLogger *next;
};

#endif // TestUtilsH
