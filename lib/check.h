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
#ifndef checkH
#define checkH
//---------------------------------------------------------------------------

#include "config.h"
#include "errortypes.h"

#include <list>
#include <string>
#include <vector>

namespace tinyxml2 {
    class XMLElement;
    class XMLDocument;
}

namespace CTU {
    class CTUInfo;
}
class AnalyzerInformation;

namespace ValueFlow {
    class Value;
}

class Settings;
class Token;
class ErrorLogger;
class ErrorMessage;
class Tokenizer;

/** Use WRONG_DATA in checkers to mark conditions that check that data is correct */
#define WRONG_DATA(COND, TOK)  (wrongData((TOK), (COND), #COND))

/// @addtogroup Core
/// @{

/**
 * @brief Interface class that cppcheck uses to communicate with the checks.
 * All checking classes must inherit from this class
 */
class CPPCHECKLIB Check {
public:
    /** This constructor is used when registering the CheckClass */
    explicit Check(const char* aname);

    /** This constructor is used when running checks. */
    Check(const char* aname, const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : mTokenizer(tokenizer), mSettings(settings), mErrorLogger(errorLogger), mName(aname) {
    }

    virtual ~Check() {
        if (!mTokenizer)
            instances().remove(this);
    }

    /** List of registered check classes. This is used by Cppcheck to run checks and generate documentation */
    static std::list<Check *> &instances();

    /** run checks, the token list is not simplified */
    virtual void runChecks(const Tokenizer *, const Settings *, ErrorLogger *) = 0;

    /** get error messages */
    virtual void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const = 0;

    /** class name, used to generate documentation */
    const std::string& name() const {
        return mName;
    }

    /** get information about this class, used to generate documentation */
    virtual std::string classInfo() const = 0;

    /**
     * Write given error to errorlogger or to out stream in xml format.
     * This is for for printout out the error list with --errorlist
     * @param errmsg Error message to write
     */
    static void reportError(const ErrorMessage &errmsg);

    /** Base class used for whole-program analysis */
    class CPPCHECKLIB FileInfo {
    public:
        FileInfo() {}
        virtual ~FileInfo() {}
        virtual tinyxml2::XMLElement* toXMLElement(tinyxml2::XMLDocument* doc) const {
            (void)doc;
            return nullptr;
        }
    };

    virtual FileInfo * getFileInfo(const Tokenizer *tokenizer, const Settings *settings) const {
        (void)tokenizer;
        (void)settings;
        return nullptr;
    }

    virtual FileInfo * loadFileInfoFromXml(const tinyxml2::XMLElement *xmlElement) const {
        (void)xmlElement;
        return nullptr;
    }

    // Return true if an error is reported.
    virtual bool analyseWholeProgram(const CTU::CTUInfo* ctu, AnalyzerInformation& analyzerInformation, const Settings& /*settings*/, ErrorLogger &/*errorLogger*/) {
        (void)ctu;
        (void)analyzerInformation;
        //(void)settings;
        //(void)errorLogger;
        return false;
    }

    static std::string getMessageId(const ValueFlow::Value &value, const char id[]);

protected:
    const Tokenizer * const mTokenizer;
    const Settings * const mSettings;
    ErrorLogger * const mErrorLogger;

    /** report an error */
    void reportError(const Token* tok, const Severity::SeverityType severity, const char id[], const std::string& msg, CWE cwe = CWE(0U), Certainty::CertaintyLevel certainty = Certainty::safe);
    void reportError(const Token* tok, const Severity::SeverityType severity, const std::string& id, const std::string& msg, CWE cwe = CWE(0U), Certainty::CertaintyLevel certainty = Certainty::safe);

    /** report an error */
    void reportError(const std::vector<const Token*>& callstack, Severity::SeverityType severity, const std::string& id, const std::string& msg, CWE cwe, Certainty::CertaintyLevel certainty);

    void reportError(const ErrorPath &errorPath, Severity::SeverityType severity, const char id[], const std::string &msg, CWE cwe, Certainty::CertaintyLevel certainty);

    ErrorPath getErrorPath(const Token* errtok, const ValueFlow::Value* value, const std::string& bug) const;

    /**
     * Use WRONG_DATA in checkers when you check for wrong data. That
     * will call this method
     */
    bool wrongData(const Token *tok, bool condition, const char *str);

    /** disabled assignment operator and copy constructor */
    void operator=(const Check &) = delete;
    Check(const Check &) = delete;
private:
    const std::string mName;
};

/// @}
//---------------------------------------------------------------------------
#endif //  checkH
