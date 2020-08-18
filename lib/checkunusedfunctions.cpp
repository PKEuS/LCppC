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


//---------------------------------------------------------------------------
#include "checkunusedfunctions.h"

#include "analyzerinfo.h"
#include "astutils.h"
#include "errorlogger.h"
#include "library.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"

#include <tinyxml2.h>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <utility>
#include <map>
//---------------------------------------------------------------------------



// Register CheckClass..
namespace {
    CheckUnusedFunctions instance;
}

static const struct CWE CWE561(561U);   // Dead Code


//---------------------------------------------------------------------------
// FUNCTION USAGE - Check for unused functions etc
//---------------------------------------------------------------------------

class CUF_FileInfo : public Check::FileInfo {
public:
    class CPPCHECKLIB FunctionUsage {
    public:
        FunctionUsage() : lineNumber(0), usedSameFile(false), usedOtherFile(false), isOperator(false) {
        }

        std::string filename;
        unsigned int lineNumber;
        bool   usedSameFile;
        bool   usedOtherFile;
        bool   isOperator;
    };

    std::map<std::string, FunctionUsage> mFunctions;

    void fromXMLElement(const tinyxml2::XMLElement* elem) {
        for (const tinyxml2::XMLElement* e = elem->FirstChildElement(); e; e = e->NextSiblingElement()) {
            if (std::strcmp(e->Name(), "function") == 0) {
                const char* name = e->Attribute("name");
                if (!name)
                    continue;

                FunctionUsage& fu = mFunctions[name];
                const char* filename = e->Attribute("filename");
                if (filename)
                    fu.filename = filename;
                fu.lineNumber = e->UnsignedAttribute("lineNumber");
                fu.usedSameFile = e->BoolAttribute("usedSameFile");
                fu.usedOtherFile = e->BoolAttribute("usedOtherFile");
                fu.isOperator = e->BoolAttribute("isOperator");
            }
        }
    }
private:
    tinyxml2::XMLElement* toXMLElement(tinyxml2::XMLDocument* doc) const override {
        tinyxml2::XMLElement* root = doc->NewElement(instance.name().c_str());

        for (auto fu = mFunctions.cbegin(); fu != mFunctions.cend(); ++fu) {
            tinyxml2::XMLElement* entry = doc->NewElement("function");
            entry->SetAttribute("name", fu->first.c_str());
            entry->SetAttribute("filename", fu->second.filename.c_str());
            entry->SetAttribute("lineNumber", fu->second.lineNumber);
            entry->SetAttribute("usedSameFile", fu->second.usedSameFile);
            entry->SetAttribute("usedOtherFile", fu->second.usedOtherFile);
            entry->SetAttribute("isOperator", fu->second.isOperator);
            root->InsertEndChild(entry);
        }
        return root;
    }
};



void CheckUnusedFunctions::unusedFunctionError(ErrorLogger * const errorLogger,
        const std::string &filename, unsigned int lineNumber,
        const std::string &funcname)
{
    std::list<ErrorMessage::FileLocation> locationList;
    if (!filename.empty()) {
        ErrorMessage::FileLocation fileLoc;
        fileLoc.setfile(filename);
        fileLoc.line = lineNumber;
        locationList.push_back(fileLoc);
    }

    const ErrorMessage errmsg(locationList, emptyString, Severity::style, "$symbol:" + funcname + "\nThe function '$symbol' is never used.", "unusedFunction", Certainty::safe, CWE561);
    if (errorLogger)
        errorLogger->reportErr(errmsg);
    else
        reportError(errmsg);
}

Check::FileInfo* CheckUnusedFunctions::loadFileInfoFromXml(const tinyxml2::XMLElement* xmlElement) const
{
    CUF_FileInfo* fi = new CUF_FileInfo;
    fi->fromXMLElement(xmlElement);
    return fi;
}

Check::FileInfo *CheckUnusedFunctions::getFileInfo(Context ctx) const
{
    if (!ctx.project->severity.isEnabled(Severity::style))
        return nullptr;

    CUF_FileInfo* fi = new CUF_FileInfo;

    const std::string& FileName = ctx.tokenizer->list.getFiles().front();
    const bool doMarkup = ctx.project->library.markupFile(FileName);

    // Function declarations..
    for (const Scope* scope : ctx.symbolDB->functionScopes) {
        const Function* func = scope->function;
        if (!func || !func->token || scope->bodyStart->fileIndex() != 0)
            continue;

        // Don't warn about functions that are marked by __attribute__((constructor)) or __attribute__((destructor))
        if (func->isAttributeConstructor() || func->isAttributeDestructor() || func->type != Function::eFunction || func->isOperator())
            continue;

        // Don't care about templates
        if (ctx.tokenizer->isCPP()) {
            const Token* retDef = func->retDef;
            while (retDef && retDef->isName())
                retDef = retDef->previous();
            if (retDef && retDef->str() == ">")
                continue;
        }

        CUF_FileInfo::FunctionUsage& usage = fi->mFunctions[func->name()];

        usage.isOperator = func->isOperator();

        if (!usage.lineNumber)
            usage.lineNumber = func->token->linenr();

        // No filename set yet..
        if (usage.filename.empty()) {
            usage.filename = ctx.tokenizer->list.getSourceFilePath();
        }
        // Multiple files => filename = "+"
        else if (usage.filename != ctx.tokenizer->list.getSourceFilePath()) {
            usage.usedOtherFile |= usage.usedSameFile;
        }
    }

    // Function usage..
    const Token* lambdaEndToken = nullptr;
    for (const Token* tok = ctx.tokenizer->tokens(); tok; tok = tok->next()) {

        if (tok->scope()->type == Scope::eEnum)
            continue;

        if (tok == lambdaEndToken)
            lambdaEndToken = nullptr;
        else if (!lambdaEndToken && tok->str() == "[")
            lambdaEndToken = findLambdaEndToken(tok);

        // parsing of library code to find called functions
        if (ctx.project->library.isexecutableblock(FileName, tok->str())) {
            const Token* markupVarToken = tok->tokAt(ctx.project->library.blockstartoffset(FileName));
            // not found
            if (!markupVarToken)
                continue;
            int scope = 0;
            bool start = true;
            // find all function calls in library code (starts with '(', not if or while etc)
            while ((scope || start) && markupVarToken) {
                if (markupVarToken->str() == ctx.project->library.blockstart(FileName)) {
                    scope++;
                    if (start) {
                        start = false;
                    }
                } else if (markupVarToken->str() == ctx.project->library.blockend(FileName))
                    scope--;
                else if (!ctx.project->library.iskeyword(FileName, markupVarToken->str())) {
                    if (fi->mFunctions.find(markupVarToken->str()) != fi->mFunctions.end())
                        fi->mFunctions[markupVarToken->str()].usedOtherFile = true;
                    else if (markupVarToken->next()->str() == "(") {
                        CUF_FileInfo::FunctionUsage& func = fi->mFunctions[markupVarToken->str()];
                        func.filename = ctx.tokenizer->list.getSourceFilePath();
                        if (func.filename.empty())
                            func.usedOtherFile = true;
                        else
                            func.usedSameFile = true;
                    }
                }
                markupVarToken = markupVarToken->next();
            }
        }

        if (!doMarkup // only check source files
            && ctx.project->library.isexporter(tok->str()) && tok->next() != nullptr) {
            const Token* propToken = tok->next();
            while (propToken && propToken->str() != ")") {
                if (ctx.project->library.isexportedprefix(tok->str(), propToken->str())) {
                    const Token* nextPropToken = propToken->next();
                    const std::string& value = nextPropToken->str();
                    if (fi->mFunctions.find(value) != fi->mFunctions.end()) {
                        fi->mFunctions[value].usedOtherFile = true;
                    }
                }
                if (ctx.project->library.isexportedsuffix(tok->str(), propToken->str())) {
                    const Token* prevPropToken = propToken->previous();
                    const std::string& value = prevPropToken->str();
                    if (value != ")" && fi->mFunctions.find(value) != fi->mFunctions.end()) {
                        fi->mFunctions[value].usedOtherFile = true;
                    }
                }
                propToken = propToken->next();
            }
        }

        if (doMarkup && ctx.project->library.isimporter(FileName, tok->str()) && tok->next()) {
            const Token* propToken = tok->next();
            if (propToken->next()) {
                propToken = propToken->next();
                while (propToken && propToken->str() != ")") {
                    const std::string& value = propToken->str();
                    if (!value.empty() && fi->mFunctions.find(value) != fi->mFunctions.end()) {
                        fi->mFunctions[value].usedOtherFile = true;
                        break;
                    }
                    propToken = propToken->next();
                }
            }
        }

        if (ctx.project->library.isreflection(tok->str())) {
            const int argIndex = ctx.project->library.reflectionArgument(tok->str());
            if (argIndex >= 0) {
                const Token* funcToken = tok->next();
                int index = 0;
                std::string value;
                while (funcToken) {
                    if (funcToken->str() == ",") {
                        if (++index == argIndex)
                            break;
                        value.clear();
                    } else
                        value += funcToken->str();
                    funcToken = funcToken->next();
                }
                if (index == argIndex) {
                    value = value.substr(1, value.length() - 2);
                    if (fi->mFunctions.find(value) != fi->mFunctions.end())
                        fi->mFunctions[value].usedOtherFile = true;
                }
            }
        }

        const Token* funcname = nullptr;

        if ((lambdaEndToken || tok->scope()->isExecutable()) && Token::Match(tok, "%name% (")) {
            funcname = tok;
        } else if ((lambdaEndToken || tok->scope()->isExecutable()) && Token::Match(tok, "%name% @< (")) {
            funcname = tok;
        } else if (Token::Match(tok, "[;{}.,()[=+-/|!?:]")) {
            funcname = tok->next();
            if (funcname && funcname->str() == "&")
                funcname = funcname->next();
            if (funcname && funcname->str() == "::")
                funcname = funcname->next();
            while (Token::Match(funcname, "%name% :: %name%"))
                funcname = funcname->tokAt(2);

            if (!Token::Match(funcname, "%name% [(),;]:}>]"))
                continue;
        }

        if (!funcname)
            continue;

        if (!funcname->isName() || funcname->isKeyword())
            continue;

        // funcname ( => Assert that the end parentheses isn't followed by {
        if (Token::Match(funcname, "%name% (|<")) {
            const Token* ftok = funcname->next();
            if (ftok->str() == "<")
                ftok = ftok->link();
            if (Token::Match(ftok->linkAt(1), ") const|throw|{"))
                funcname = nullptr;
        }

        if (funcname) {
            CUF_FileInfo::FunctionUsage& func = fi->mFunctions[funcname->str()];
            const std::string& called_from_file = ctx.tokenizer->list.getSourceFilePath();

            if (func.filename.empty() || func.filename != called_from_file)
                func.usedOtherFile = true;
            else
                func.usedSameFile = true;
        }
    }
    return fi;
}

bool CheckUnusedFunctions::analyseWholeProgram(const CTU::CTUInfo* ctu, AnalyzerInformation& analyzerInformation, Context ctx)
{
    if (!ctx.project->severity.isEnabled(Severity::style))
        return false;

    bool errors = false;
    for (auto it = analyzerInformation.getCTUs().begin(); it != analyzerInformation.getCTUs().end(); ++it) {
        CUF_FileInfo* fi = dynamic_cast<CUF_FileInfo*>(it->getCheckInfo(name()));
        if (!fi)
            continue;

        for (std::map<std::string, CUF_FileInfo::FunctionUsage>::iterator it2 = fi->mFunctions.begin(); it2 != fi->mFunctions.end(); ++it2) {
            CUF_FileInfo::FunctionUsage& func = it2->second;
            if (func.lineNumber == 0 || func.filename.empty())
                continue;
            if (it2->first == "main" || (ctx.project->isWindowsPlatform() && (it2->first == "WinMain" || it2->first == "_tmain")))
                continue;

            // Collect usages from other files
            for (auto it3 = analyzerInformation.getCTUs().cbegin(); !func.usedOtherFile && it3 != analyzerInformation.getCTUs().cend(); ++it3) {
                CUF_FileInfo* fi2 = dynamic_cast<CUF_FileInfo*>(it3->getCheckInfo(name()));
                if (!fi2 || fi == fi2)
                    continue;
                for (std::map<std::string, CUF_FileInfo::FunctionUsage>::const_iterator it4 = fi2->mFunctions.begin(); !func.usedOtherFile && it4 != fi2->mFunctions.end(); ++it4) {
                    if (it4->first != it2->first)
                        continue;
                    const CUF_FileInfo::FunctionUsage& func2 = it4->second;
                    if (func2.usedOtherFile || func2.usedSameFile) {
                        func.usedOtherFile = true;
                    }
                }
            }

            if (func.usedOtherFile)
                continue;
            if (!func.usedSameFile) {
                if (it2->second.isOperator)
                    continue;
                unusedFunctionError(ctx.errorLogger, func.filename, func.lineNumber, it2->first);
                errors = true;
            } else {
                /** @todo add error message "function is only used in <file> it can be static" */
                /*
                std::ostringstream errmsg;
                errmsg << "The function '" << it->first << "' is only used in the file it was declared in so it should have local linkage.";
                mErrorLogger->reportErr( errmsg.str() );
                errors = true;
                */
            }
        }
    }
    return errors;
}
