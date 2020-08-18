/*
 * LCppC - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
 * Copyright (C) 2020 LCppC project.
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

//---------------------------------------------------------------------------

#include "check.h"

#include "errorlogger.h"
#include "settings.h"

#include <iostream>

//---------------------------------------------------------------------------

Check::Check(const char* aname)
    : mName(aname)
{
    for (std::list<Check*>::iterator i = instances().begin(); i != instances().end(); ++i) {
        if ((*i)->name() > aname) {
            instances().insert(i, this);
            return;
        }
    }
    instances().push_back(this);
}

void Check::reportError(const ErrorMessage &errmsg)
{
    std::cout << errmsg.toXML() << std::endl;
}


void Check::reportError(const Token* tok, Severity::SeverityType severity, const char id[], const std::string& msg, CWE cwe, Certainty::CertaintyLevel certainty)
{
    const ErrorMessage errmsg(tok, mCtx.tokenizer ? &mCtx.tokenizer->list : nullptr, severity, id, msg, certainty, cwe);
    if (mCtx.errorLogger)
        mCtx.errorLogger->reportErr(errmsg);
    else
        reportError(errmsg);
}

void Check::reportError(const Token* tok, Severity::SeverityType severity, const std::string& id, const std::string& msg, CWE cwe, Certainty::CertaintyLevel certainty)
{
    const ErrorMessage errmsg(tok, mCtx.tokenizer ? &mCtx.tokenizer->list : nullptr, severity, id, msg, certainty, cwe);
    if (mCtx.errorLogger)
        mCtx.errorLogger->reportErr(errmsg);
    else
        reportError(errmsg);
}

void Check::reportError(const std::vector<const Token*>& callstack, Severity::SeverityType severity, const std::string& id, const std::string& msg, CWE cwe, Certainty::CertaintyLevel certainty)
{
    const ErrorMessage errmsg(callstack, mCtx.tokenizer ? &mCtx.tokenizer->list : nullptr, severity, id, msg, certainty, cwe);
    if (mCtx.errorLogger)
        mCtx.errorLogger->reportErr(errmsg);
    else
        reportError(errmsg);
}

void Check::reportError(const ErrorPath &errorPath, Severity::SeverityType severity, const char id[], const std::string &msg, CWE cwe, Certainty::CertaintyLevel certainty)
{
    const ErrorMessage errmsg(errorPath, mCtx.tokenizer ? &mCtx.tokenizer->list : nullptr, severity, id, msg, certainty, cwe);
    if (mCtx.errorLogger)
        mCtx.errorLogger->reportErr(errmsg);
    else
        reportError(errmsg);
}

bool Check::wrongData(const Token *tok, bool condition, const char *str)
{
#if defined(DACA2) || defined(UNSTABLE)
    if (condition) {
        reportError(tok, Severity::debug, "DacaWrongData", "Wrong data detected by condition " + std::string(str));
    }
#else
    (void)tok;
    (void)str;
#endif
    return condition;
}

std::list<Check *> &Check::instances()
{
    static std::list<Check *> _instances;
    return _instances;
}

std::string Check::getMessageId(const ValueFlow::Value &value, const char id[])
{
    if (value.condition != nullptr)
        return id + std::string("Cond");
    if (value.safe)
        return std::string("safe") + (char)std::toupper(id[0]) + (id + 1);
    return id;
}

ErrorPath Check::getErrorPath(const Token* errtok, const ValueFlow::Value* value, const std::string& bug) const
{
    ErrorPath errorPath;
    if (!value) {
        errorPath.emplace_back(errtok, bug);
    } else if (mCtx.settings->verbose || mCtx.settings->xml || !mCtx.settings->templateLocation.empty()) {
        errorPath = value->errorPath;
        errorPath.emplace_back(errtok, bug);
    } else {
        if (value->condition)
            errorPath.emplace_back(value->condition, "condition '" + value->condition->expressionString() + "'");
        //else if (!value->isKnown() || value->defaultArg)
        //    errorPath = value->callstack;
        errorPath.emplace_back(errtok, bug);
    }
    return errorPath;
}