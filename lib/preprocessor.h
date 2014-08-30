/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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
#ifndef preprocessorH
#define preprocessorH
//---------------------------------------------------------------------------

#include <map>
#include <istream>
#include <string>
#include <list>
#include <set>
#include <stack>
#include "config.h"
#include "tokenize.h"

class ErrorLogger;
class Settings;
class TestPreprocessor;
class Token;
class Tokenizer;


CPPCHECKLIB std::string join(const std::set<std::string>& list, char separator);

/// @addtogroup Core
/// @{

class CPPCHECKLIB Preprocessor2 {
    friend class TestPreprocessor;
public:
    class CPPCHECKLIB Configuration {
        Configuration(const Configuration&);
        Configuration& operator=(const Configuration&);

        static bool replaceMacro_inner(Configuration& config, Token* tok, const std::string& definition, std::map<const Token*, std::set<std::string>>& replacementMap);
    public:
        enum AnalysisResult {Unhandled, New, Known, Conflict};
        enum SplitMode {AllowSplit, AllDefined, AllUndefined};

        std::map<std::string, std::string> assumptedDefs;
        std::set<std::string> assumptedNdefs;
        std::map<std::string, std::string> defs;
        std::set<std::string> undefs;
        std::stack<bool> ifDecisions;
        std::set<std::string> includedOnce;
        Tokenizer tokenizer;
        Token* tok;

        Configuration(Settings* settings, ErrorLogger *errorLogger) : tokenizer(settings, errorLogger), tok(0) {}
        Configuration(Preprocessor2::Configuration& oldcfg, Token* _tok);
        AnalysisResult analyze(const std::string& name, SplitMode splitMode) const;
        std::string toString() const;
        void preDefine(const std::set<std::string>& newDefs);
        void preUndef(const std::set<std::string>& newUndefs);

        static Token* simplifyIf(Token* start, Token* end, Configuration& config, SplitMode splitMode);
        static AnalysisResult analyzeIf(Token* tok, Configuration& config, std::string& configName, SplitMode splitMode);
        static Token* replaceMacro(Configuration& config, Token* tok, const std::string& name);
    };

    Preprocessor2(Settings *settings = 0, ErrorLogger *errorLogger = 0);
    ~Preprocessor2();

    void preprocess(std::istream& code, const std::string& filename, const std::list<std::string> &includePaths = std::list<std::string>());
    void finish(const std::string& cfg);
    static void getErrorMessages(ErrorLogger *errorLogger, Settings *settings);
private:
    static std::string readCode(std::istream &istr);
    void simplifyString(std::string& str, const std::string& filename);
    static void replaceDiTrigraphs(std::string& str);
    static void concatenateLines(std::string& str);
    static void removeWhitespaces(std::string& str);
    void removeComments(std::string& str, const std::string& filename);
    void tokenize(const std::string& str, Tokenizer& tokenizer, const std::string& filename, Token* next);
    void getConfigurations(const std::string& code, const std::string& filename, const std::list<std::string> &includePaths = std::list<std::string>());
    static void uniformizeIfs(Tokenizer& tokenizer);
    static void createLinkage(Tokenizer& tokenizer);
    static void handlePragma(Token* tok, std::set<std::string>& includedOnce, const std::string& file);
    void handleInclude(Token* tok, Tokenizer& tokenizer, const std::set<std::string>& includedOnce, const std::list<std::string> &includePaths);

    void error_missingInclude(const Token* tok, const Tokenizer& tokenizer, const std::string &header, bool userheader);
    void error_tooManyConfigs(const std::string &file);

    static void writeError(const std::string &fileName, const unsigned int linenr, ErrorLogger *errorLogger, const std::string &errorType, const std::string &errorText);

public:
    std::map<std::string, Configuration*> cfg;

    /** character that is inserted in expanded macros */
    static char macroChar;

    static bool missingIncludeFlag;
    static bool missingSystemIncludeFlag;

private:
    Configuration::SplitMode splitMode;
    Settings *_settings;
    ErrorLogger *_errorLogger;
};

/// @}

//---------------------------------------------------------------------------
#endif
