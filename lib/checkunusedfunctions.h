/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2019 Cppcheck team.
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
#ifndef checkunusedfunctionsH
#define checkunusedfunctionsH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"

#include <list>
#include <string>

class ErrorLogger;
class Settings;
class Tokenizer;

/// @addtogroup Checks
/** @brief Check for functions never called */
/// @{

class CPPCHECKLIB CheckUnusedFunctions : public Check {
public:
    /** @brief This constructor is used when registering the CheckUnusedFunctions */
    CheckUnusedFunctions() : Check(myName()) {
    }

    /** @brief This constructor is used when running checks. */
    CheckUnusedFunctions(const Tokenizer* tokenizer, const Settings* settings, ErrorLogger* errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    /** @brief Parse current TU and extract file info */
    Check::FileInfo* getFileInfo(const Tokenizer *tokenizer, const Settings *settings) const override;

    Check::FileInfo* loadFileInfoFromXml(const tinyxml2::XMLElement* xmlElement) const override;

    /** @brief Analyse all file infos for all TU */
    bool analyseWholeProgram(const CTU::CTUInfo *ctu, AnalyzerInformation& analyzerInformation, const Settings& settings, ErrorLogger &errorLogger) override;

private:

    void getErrorMessages(ErrorLogger *errorLogger, const Settings * settings) const override {
        CheckUnusedFunctions c(nullptr, settings, errorLogger);
        c.unusedFunctionError(errorLogger, emptyString, 0, "funcName");
    }

    void runChecks(const Tokenizer * /*tokenizer*/, const Settings * /*settings*/, ErrorLogger * /*errorLogger*/) override {
    }

    void unusedFunctionError(ErrorLogger* const errorLogger, const std::string &filename, unsigned int lineNumber, const std::string &funcname);

    static const char* myName() {
        return "UnusedFunction";
    }

    std::string classInfo() const override {
        return "Check for functions that are never called\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkunusedfunctionsH
