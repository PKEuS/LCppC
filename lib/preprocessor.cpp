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


#include "preprocessor.h"
#include "tokenize.h"
#include "token.h"
#include "path.h"
#include "errorlogger.h"
#include "settings.h"
#include "version.h"

#include <algorithm>
#include <sstream>
#include <fstream>
#include <cctype>
#include <vector>
#include <set>
#include <stack>
#include <queue>

static unsigned char readChar(std::istream &istr, unsigned int bom);
static bool hasbom(const std::string &str);
static bool isFallThroughComment(std::string comment);
static bool openHeader(std::string &filename, const std::list<std::string> &includePaths, const std::string &filePath, std::ifstream &fin);

//-----------------------------------------------------------------------------

bool Preprocessor2::missingIncludeFlag = false;
bool Preprocessor2::missingSystemIncludeFlag = false;
char Preprocessor2::macroChar = char(1);


Preprocessor2::Preprocessor2(Settings *settings, ErrorLogger *errorLogger)
    : _settings(settings), _errorLogger(errorLogger)
{
    if (settings && settings->userDefined && !settings->userUndefined)
        splitMode = Configuration::AllDefined;
    else if (settings && !settings->userDefined && settings->userUndefined)
        splitMode = Configuration::AllUndefined;
    else
        splitMode = Configuration::AllowSplit;
}

Preprocessor2::~Preprocessor2()
{
    for (std::map<std::string, Configuration*>::iterator i = cfg.begin(); i != cfg.end(); ++i)
        delete i->second;
}

void Preprocessor2::preprocess(std::istream& is, const std::string& filename, const std::list<std::string> &includePaths)
{
    // Read file
    std::string data = readCode(is);
    std::string internal_filename = Path::fromNativeSeparators(filename);

    // Do basic replacements and cleanup
    simplifyString(data, internal_filename);

    // Tokenize
    getConfigurations(data, internal_filename, includePaths);
}

std::string Preprocessor2::readCode(std::istream &istr)
{
    // The UTF-16 BOM is 0xfffe or 0xfeff.
    unsigned int bom = 0;
    if (istr.peek() >= 0xfe) {
        bom = ((unsigned int)istr.get() << 8);
        if (istr.peek() >= 0xfe)
            bom |= (unsigned int)istr.get();
    }

    std::ostringstream oss;
    while (istr.good()) {
        char c = readChar(istr, bom);
        if (!istr.eof())
            oss << c;
    }

    return oss.str();
}

void Preprocessor2::simplifyString(std::string& str, const std::string& filename)
{
    replaceDiTrigraphs(str);

    // for gcc-compatibility the trailing spaces should be ignored
    // for vs-compatibility the trailing spaces should be kept
    // See tickets #640 and #1869
    // The solution for now is to have a compiler-dependent behaviour.
#ifdef __GNUC__
    for (unsigned int i = 0; i < str.size()-1;)
        if (str[i] == '\\' && str[i+1] == ' ')
            str.erase(i+1, 1);
        else
            i++;
#endif
    concatenateLines(str);

    removeComments(str, filename);
    removeWhitespaces(str);
}

void Preprocessor2::removeWhitespaces(std::string& str)
{
    // Replace all tabs with spaces..
    std::replace(str.begin(), str.end(), '\t', ' ');

    for (unsigned int i = 0; i < str.size();) {
        if (str[i] == ' ' &&
            (i == 0 || i + 1 >= str.size() || str[i-1] == '\n' || str[i+1] == '\n')
           ) {
            // remove space that has new line in either side of it
            str.erase(i, 1);
        } else
            i++;
    }
}

void Preprocessor2::replaceDiTrigraphs(std::string& str)
{
    if (str.length() < 3)
        return;

    for (std::string::size_type i = 0; i < str.size()-1; i++) {
        if (str[i] == '?' && str[i+1] == '?' && i+2 < str.size()) {
            switch (str[i+2]) {
            case '=':
                str.replace(i, 3, 1, '#');
                break;
            case '/':
                str.replace(i, 3, 1, '\\');
                break;
            case '\'':
                str.replace(i, 3, 1, '^');
                break;
            case '(':
                str.replace(i, 3, 1, '[');
                break;
            case ')':
                str.replace(i, 3, 1, ']');
                break;
            case '!':
                str.replace(i, 3, 1, '|');
                break;
            case '<':
                str.replace(i, 3, 1, '{');
                break;
            case '>':
                str.replace(i, 3, 1, '}');
                break;
            case '-':
                str.replace(i, 3, 1, '~');
                break;
            }
        } else if (str[i] == '<' && str[i+1] == ':')
            str.replace(i, 2, 1, '[');
        else if (str[i] == ':' && str[i+1] == '>')
            str.replace(i, 2, 1, ']');
        else if (str[i] == '<' && str[i+1] == '%')
            str.replace(i, 2, 1, '{');
        else if (str[i] == '%' && str[i+1] == '>')
            str.replace(i, 2, 1, '}');
        else if (str[i] == '%' && str[i+1] == ':')
            str.replace(i, 2, 1, '#');
    }
}

void Preprocessor2::concatenateLines(std::string& str)
{
    if (str.length()<3)
        return;

    unsigned int counter = 0;
    for (std::string::size_type i = 0; i < str.size()-1; i++) {
        if (str[i] == '\\' && str[i+1] == '\n') {
            str.erase(i, 2);
            counter++;
        } else if (str[i] == '\n' && counter != 0) {
            str.insert(i, counter, '\n'); // Append all removed physical newlines at the end of the logical line to make the whole line numbers consistent
            counter = 0;
        }
    }
}

void Preprocessor2::removeComments(std::string& str, const std::string& filename)
{
    if (str.empty())
        return;

    // For the error report
    unsigned int lineno = 1;

    // handling <backslash><newline>
    // when this is encountered the <backslash><newline> will be "skipped".
    // on the next <newline>, extra newlines will be added
    unsigned int newlines = 0;
    std::ostringstream code;
    unsigned char previous = 0;
    bool inPreprocessorLine = false;
    std::vector<std::string> suppressionIDs;
    bool fallThroughComment = false;

    for (std::string::size_type i = hasbom(str) ? 3U : 0U; i < str.length(); ++i) {
        unsigned char ch = static_cast<unsigned char>(str[i]);
        if (ch & 0x80) {
            std::ostringstream errmsg;
            errmsg << "The code contains characters that are unhandled. "
                   << "Neither unicode nor extended ASCII are supported. "
                   << "(line=" << lineno << ", character code=" << std::hex << (int(ch) & 0xff) << ")";
            writeError(filename, lineno, _errorLogger, "syntaxError", errmsg.str());
        }

        // First skip over any whitespace that may be present
        if (std::isspace(ch)) {
            if (ch == ' ' && previous == ' ') {
                // Skip double white space
            } else {
                code << char(ch);
                previous = ch;
            }

            // if there has been <backslash><newline> sequences, add extra newlines..
            if (ch == '\n') {
                if (previous != '\\')
                    inPreprocessorLine = false;
                ++lineno;
                if (newlines > 0) {
                    code << std::string(newlines, '\n');
                    newlines = 0;
                    previous = '\n';
                }
            }

            continue;
        }

        // Remove comments..
        if (str.compare(i, 2, "//", 0, 2) == 0) {
            size_t commentStart = i + 2;
            i = str.find('\n', i);
            if (i == std::string::npos)
                break;
            std::string comment(str, commentStart, i - commentStart);

            if (_settings && _settings->_inlineSuppressions) {
                std::istringstream iss(comment);
                std::string word;
                iss >> word;
                if (word == "cppcheck-suppress") {
                    iss >> word;
                    if (iss)
                        suppressionIDs.push_back(word);
                }
            }

            if (isFallThroughComment(comment)) {
                fallThroughComment = true;
            }

            code << '\n';
            previous = '\n';
            ++lineno;
        } else if (str.compare(i, 2, "/*", 0, 2) == 0) {
            size_t commentStart = i + 2;
            unsigned char chPrev = 0;
            ++i;
            while (i < str.length() && (chPrev != '*' || ch != '/')) {
                chPrev = ch;
                ++i;
                ch = static_cast<unsigned char>(str[i]);
                if (ch == '\n') {
                    ++newlines;
                    ++lineno;
                }
            }
            std::string comment(str, commentStart, i - commentStart - 1);

            if (isFallThroughComment(comment)) {
                fallThroughComment = true;
            }

            if (_settings && _settings->_inlineSuppressions) {
                std::istringstream iss(comment);
                std::string word;
                iss >> word;
                if (word == "cppcheck-suppress") {
                    iss >> word;
                    if (iss)
                        suppressionIDs.push_back(word);
                }
            }
        } else if (ch == '#' && previous == '\n') {
            code << ch;
            previous = ch;
            inPreprocessorLine = true;

            // Add any pending inline suppressions that have accumulated.
            if (!suppressionIDs.empty()) {
                if (_settings != NULL) {
                    // Add the suppressions.
                    for (size_t j(0); j < suppressionIDs.size(); ++j) {
                        const std::string errmsg(_settings->nomsg.addSuppression(suppressionIDs[j], Path::fromNativeSeparators(filename), lineno));
                        if (!errmsg.empty()) {
                            writeError(filename, lineno, _errorLogger, "cppcheckError", errmsg);
                        }
                    }
                }
                suppressionIDs.clear();
            }
        } else {
            if (!inPreprocessorLine) {
                // Not whitespace, not a comment, and not preprocessor.
                // Must be code here!

                // First check for a "fall through" comment match, but only
                // add a suppression if the next token is 'case' or 'default'
                if (fallThroughComment && _settings->experimental && _settings->isEnabled("style")) {
                    std::string::size_type j = str.find_first_not_of("abcdefghijklmnopqrstuvwxyz", i);
                    std::string tok = str.substr(i, j - i);
                    if (tok == "case" || tok == "default")
                        suppressionIDs.push_back("switchCaseFallThrough");
                    fallThroughComment = false;
                }

                // Add any pending inline suppressions that have accumulated.
                if (!suppressionIDs.empty()) {
                    if (_settings != NULL) {
                        // Add the suppressions.
                        for (size_t j(0); j < suppressionIDs.size(); ++j) {
                            const std::string errmsg(_settings->nomsg.addSuppression(suppressionIDs[j], Path::fromNativeSeparators(filename), lineno));
                            if (!errmsg.empty()) {
                                writeError(filename, lineno, _errorLogger, "cppcheckError", errmsg);
                            }
                        }
                    }
                    suppressionIDs.clear();
                }
            }

            // String or char constants..
            if (ch == '\"' || ch == '\'') {
                code << char(ch);
                char chNext;
                do {
                    ++i;
                    chNext = str[i];
                    if (chNext == '\\') {
                        ++i;
                        char chSeq = str[i];
                        if (chSeq == '\n')
                            ++newlines;
                        else {
                            code << chNext;
                            code << chSeq;
                            previous = static_cast<unsigned char>(chSeq);
                        }
                    } else {
                        code << chNext;
                        previous = static_cast<unsigned char>(chNext);
                    }
                } while (i < str.length() && chNext != ch && chNext != '\n');
            }

            // Rawstring..
            else if (str.compare(i,2,"R\"")==0) {
                std::string delim;
                for (std::string::size_type i2 = i+2; i2 < str.length(); ++i2) {
                    if (i2 > 16 ||
                        std::isspace(str[i2]) ||
                        std::iscntrl(str[i2]) ||
                        str[i2] == ')' ||
                        str[i2] == '\\') {
                        delim = " ";
                        break;
                    } else if (str[i2] == '(')
                        break;

                    delim += str[i2];
                }
                const std::string::size_type endpos = str.find(")" + delim + "\"", i);
                if (delim != " " && endpos != std::string::npos) {
                    unsigned int rawstringnewlines = 0;
                    code << '"';
                    for (std::string::size_type p = i + 3 + delim.size(); p < endpos; ++p) {
                        if (str[p] == '\n') {
                            rawstringnewlines++;
                            code << "\\n";
                        } else if (std::iscntrl((unsigned char)str[p]) ||
                                   std::isspace((unsigned char)str[p])) {
                            code << ' ';
                        } else if (str[p] == '\\') {
                            code << '\\';
                        } else if (str[p] == '\"' || str[p] == '\'') {
                            code << '\\' << (char)str[p];
                        } else {
                            code << (char)str[p];
                        }
                    }
                    code << '"';
                    if (rawstringnewlines > 0)
                        code << std::string(rawstringnewlines, '\n');
                    i = endpos + delim.size() + 1;
                } else {
                    code << 'R';
                    previous = 'R';
                }
            } else {
                code << (char)ch;
                previous = ch;
            }
        }
    }

    str = code.str();
}

void Preprocessor2::tokenize(const std::string& str, Tokenizer& tokenizer, const std::string& filename, Token* next)
{
    std::istringstream iss(str);
    tokenizer.list.tokenizeFile(iss, filename, next);

    uniformizeIfs(tokenizer);
    createLinkage(tokenizer);
}

void Preprocessor2::uniformizeIfs(Tokenizer& tokenizer)
{
    for (Token* tok = tokenizer.list.front(); tok; tok = tok->next()) {
        if (tok->str() == "#" && tok->next()) {
            tok = tok->next();
            if (tok->str() == "ifdef") {
                tok->str("if");
                tok->insertToken("(");
                tok->insertToken("defined");
                tok->getLineEnd()->insertToken(")");
                continue;
            } else if (tok->str() == "ifndef") {
                tok->str("if");
                tok->insertToken("(");
                tok->insertToken("defined");
                tok->insertToken("!");
                tok->getLineEnd()->insertToken(")");
                continue;
            } else if (Token::simpleMatch(tok, "else if")) {
                tok->str("elif");
                tok->deleteNext();
            }

            if (Token::Match(tok, "if|elif")) {
                tok = tok->next();
                for (const Token* end = tok->getLineEnd(); tok && tok != end; tok = tok->next()) {
                    if (Token::Match(tok, "defined !!(")) { // Add brackets
                        tok->insertToken("(");
                        tok->tokAt(2)->insertToken(")");
                    }
                }
            }
        }
    }
}

void Preprocessor2::createLinkage(Tokenizer& tokenizer)
{
    // (Re-)Create linkage
    std::stack<Token*> ifs;
    for (Token* tok = tokenizer.list.front(); tok; tok = tok->next()) {
        if (tok->str() == "#" && tok->next()) {
            if (tok->next()->str() == "if") {
                ifs.push(tok);
            } else if (tok->next()->str() == "else" || tok->next()->str() == "elif") {
                if (ifs.empty())
                    ifs.push(tok);
                else {
                    Token::createMutualLinks(ifs.top(), tok);
                    ifs.top() = tok;
                }
            } else if (tok->next()->str() == "endif" && !ifs.empty()) {
                Token::createMutualLinks(ifs.top(), tok);
                ifs.pop();
            }
        }
    }
}

void Preprocessor2::handleInclude(Token* tok, Tokenizer& tokenizer, const std::set<std::string>& includedOnce, const std::list<std::string> &includePaths)
{
    std::string native_filename = Path::toNativeSeparators(tok->next()->strValue());
    std::string internal_filename = Path::fromNativeSeparators(tok->next()->strValue());
    const std::string& filePath = tokenizer.list.file(tok);
    bool userHeader = tok->next()->str()[0] == '"';
    std::string filepath = userHeader ? filePath.substr(0, 1 + filePath.find_last_of("\\/")) : "";

    if (!native_filename.empty() && includedOnce.find(internal_filename) == includedOnce.end()) {
        // Read file
        std::ifstream ifs;
        if (openHeader(native_filename, includePaths, filepath, ifs)) {
            std::string data = readCode(ifs);

            // Do basic replacements and cleanup
            replaceDiTrigraphs(data);
            simplifyString(data, internal_filename);

            // Tokenize
            tokenize(data, tokenizer, internal_filename, tok->tokAt(2));
        } else {
            missingIncludeFlag = true;
            if (_settings && (userHeader || _settings->debugwarnings) && !_settings->nomsg.isSuppressed("missingInclude", Path::fromNativeSeparators(tokenizer.list.file(tok)), tok->linenr()) &&
                ((_settings->isEnabled("information") && userHeader) || _settings->debugwarnings)) {
                error_missingInclude(tok, tokenizer, internal_filename, userHeader);
            }
        }
    }
}

void Preprocessor2::handlePragma(Token* tok, std::set<std::string>& includedOnce, const std::string& file)
{
    const std::string& name = tok->strAt(1);
    if (name == "asm") {
        tok->str("asm");
        tok->next()->str("(");
    } else if (name == "endasm") { // TODO: Handle parameters.
        tok->str(")");
        tok->next()->str(";");
    } else if (name == "once") {
        tok->deleteNext();
        tok->deleteThis();
        includedOnce.insert(file);
    } else
        Token::eraseTokens(tok->previous(), tok->getLineEnd()->next());
}

static void handleAsm(Token* tok)
{
    if (tok->str() == "asm") {
        tok->str("asm");
        tok->insertToken("(");
    } else { // TODO: Handle parameters.
        tok->str(")");
        tok->insertToken(";");
    }
}

Token* Preprocessor2::Configuration::replaceMacro(Preprocessor2::Configuration& config, Token* tok, const std::string& name)
{
    // Is the macro defined?
    const std::string* definition;
    if (config.defs.find(name) != config.defs.end())
        definition = &config.defs[name];
    else if (config.assumptedDefs.find(name) != config.assumptedDefs.end() && !config.assumptedDefs[name].empty())
        definition = &config.assumptedDefs[name];
    else
        return tok;

    // Can we replace it?
    if (definition->at(0) == '>' || definition->at(0) == '<' || definition->at(0) == '!') {
        if (definition->size() > 1 && definition->at(1) != '=' && definition->at(1) != ' ')
            return tok;
        else if (definition->size() > 2 && definition->at(1) == '=' && definition->at(2) != ' ')
            return tok;
    }

    Token* temp = tok->previous();
    std::map<const Token*, std::set<std::string>> map;
    replaceMacro_inner(config, tok, *definition, map);
    return (temp?temp:config.tokenizer.list.front()); // Return token before replacement
}

bool Preprocessor2::Configuration::replaceMacro_inner(Preprocessor2::Configuration& config, Token* tok, const std::string& definition, std::map<const Token*, std::set<std::string>>& replacementMap)
{
    if (definition.empty()) { // Empty simple macro
        if (tok->previous())
            tok->previous()->deleteNext();
        else
            tok->deleteThis();
    } else if (definition[0] != '(') { // Simple macro
        bool found = replacementMap.find(tok) != replacementMap.end();
        std::string name = tok->str();
        if (found && replacementMap[tok].find(name) != replacementMap[tok].end()) // Avoid recursion
            return false;

        Token* start = tok->previous(); // Points to token before expanded code
        config.tokenizer.list.tokenizeString(definition, tok); // expand
        Token* tok2 = tok->previous(); // Points to last token of expanded code
        tok->previous()->deleteNext();
        while (start != tok2) {
            tok2->isExpandedMacro(true);
            replacementMap[tok2].insert(name); // tok2 was expanded from macro with name tok->str()
            if (found)
                replacementMap[tok2].insert(replacementMap[tok].begin(), replacementMap[tok].end());

            if (config.defs.find(tok2->str()) != config.defs.end()) { // Look for further replacements (-> recursion)
                Token* temp = tok2->next();
                if (replaceMacro_inner(config, tok2, config.defs.at(tok2->str()), replacementMap))
                    tok2 = (temp&&temp->previous())?temp->tokAt(-2):config.tokenizer.list.back();
                else // No replacement done, because recursion was detected
                    tok2 = tok2->previous();
            } else
                tok2 = tok2->previous();
        }
        if (found)
            replacementMap.erase(tok);
    } else { // "Function" macro
        if (replacementMap.find(tok) != replacementMap.end() && replacementMap[tok].find(tok->str()) != replacementMap[tok].end()) // Avoid recursion
            return false;

        unsigned int bracketCount = 1;
        std::string::size_type i = 1;
        for (; i < definition.size(); i++) { // Find end of parameter list
            if (definition[i] == '(')
                bracketCount++;
            else if (definition[i] == ')') {
                bracketCount--;
                if (bracketCount == 0)
                    break;
            }
        }
        std::string definedAs = definition.substr(i+1);
        std::string definedFrom = definition.substr(0, i+1);
    }
    return true;
}



Preprocessor2::Configuration::Configuration(Preprocessor2::Configuration& oldcfg, Token* _tok)
    : assumptedDefs(oldcfg.assumptedDefs), assumptedNdefs(oldcfg.assumptedNdefs), defs(oldcfg.defs),
      undefs(oldcfg.undefs), ifDecisions(oldcfg.ifDecisions), includedOnce(oldcfg.includedOnce),
      tokenizer(oldcfg.tokenizer, _tok)
{
    tok = _tok;
}

Preprocessor2::Configuration::AnalysisResult Preprocessor2::Configuration::analyze(const std::string& name, SplitMode splitMode) const
{
    if (undefs.find(name) != undefs.end())
        return Conflict;
    if (defs.find(name) != defs.end())
        return Known;
    if (assumptedNdefs.find(name) != assumptedNdefs.end())
        return Conflict;
    if (assumptedDefs.find(name) != assumptedDefs.end())
        return Known;

    switch (splitMode) {
    case AllDefined:
        return Conflict;
    case AllUndefined:
        return Known;
    case AllowSplit:
    default:
        return New;
    }
}

std::string Preprocessor2::Configuration::toString() const
{
    std::set<std::string> temp;
    for (std::map<std::string, std::string>::const_iterator i = assumptedDefs.begin(); i != assumptedDefs.end(); ++i)
        temp.insert(i->first);
    return join(temp, ';');
}

static void insertCfgToMap(std::map<std::string, std::string>& map, const std::string& cfg)
{
    std::string::size_type i = cfg.find_first_of("=><!");
    if (i != std::string::npos) {
        if (cfg[i] == '=')
            map[cfg.substr(0, i)] = cfg.substr(i+1);
        else
            map[cfg.substr(0, i)] = cfg.substr(i);
    } else
        map[cfg] = "";
}

void Preprocessor2::Configuration::preDefine(const std::set<std::string>& newDefs)
{
    for (std::set<std::string>::const_iterator i = newDefs.begin(); i != newDefs.end(); ++i)
        insertCfgToMap(defs, *i);
}
void Preprocessor2::Configuration::preUndef(const std::set<std::string>& newUndefs)
{
    undefs.insert(newUndefs.begin(), newUndefs.end());
}

static void removeConditionEnd(Token* start, Token* end)
{
    unsigned int indent = 0;
    for (Token* tok = start; tok->next() != end;) {
        if (tok->next()->str() == "(")
            indent++;
        else if (tok->next()->str() == ")") {
            if (indent == 0)
                break;
            indent--;
        }
        tok->deleteNext();
    }
    start->deleteThis();
}

static Token* removePrevCondition(Token* start, Token* end)
{
    unsigned int indent = 0;
    Token* tok = start->previous();
    for (; tok->next() != end; tok = tok->previous()) {
        if (tok->str() == ")")
            indent++;
        else if (tok->str() == "(") {
            if (indent == 0)
                break;
            indent--;
        } else if (indent == 0 && (tok->str() == "&&" || tok->str() == "||"))
            break;
        tok->deleteNext();
    }

    tok->deleteNext();
    return tok;
}

Token* Preprocessor2::Configuration::simplifyIf(Token* start, Token* end, Preprocessor2::Configuration& config, SplitMode splitMode)
{
    end = end->next();
    bool neg = false;
    Preprocessor2::Configuration::AnalysisResult decision = New;
    for (Token* tok = start; tok && tok != end->previous(); tok = tok->next()) {
        if (tok->str() == "!") {
            neg = !neg;
            tok->deleteThis();
        }
        if (Token::Match(tok, "defined ( %var% )")) {
            AnalysisResult result = config.analyze(tok->strAt(2), splitMode);
            if (result != New) {
                if ((result == Known && neg) || (result == Conflict && !neg)) {
                    tok->str("0");
                } else
                    tok->str("1");
                tok->deleteNext(3);
            } else {
                if (neg)
                    tok->previous()->insertToken("!");
                tok = tok->tokAt(3);
            }
            neg = false;
        } else if (tok->str() == "(" && tok->strAt(-1) != "defined") {
            Token* endBracket = simplifyIf(tok->next(), end->previous(), config, splitMode);
            if (tok->strAt(2) == ")") {
                tok->deleteThis();
                tok->deleteNext();
                if (neg && tok->str() == "0")
                    tok->str("1");
                else if (neg && tok->isNumber()) // 1 or higher
                    tok->str("0");
            } else {
                if (neg)
                    tok->previous()->insertToken("!");
                tok = endBracket;
            }
            neg = false;
        } else if (neg && tok->str() == "0") {
            tok->str("1");
            neg = false;
        } else if (neg && tok->isNumber()) { // 1 or higher
            tok->str("0");
            neg = false;
        } else if (tok->str() == "&&") {
            if (decision == Conflict) // Just remove until end, condition is false anyway
                removeConditionEnd(tok, end->previous());
            else if (decision == Known)
                tok = removePrevCondition(tok, start);
        } else if (tok->str() == "||") {
            if (decision == Known) // Just remove until end, condition is true anyway
                removeConditionEnd(tok, end->previous());
            else if (decision == Conflict)
                tok = removePrevCondition(tok, start);
        } else if (tok->str() == ")") {
            return tok;
        } else if (!tok->isExpandedMacro())
            tok = replaceMacro(config, tok, tok->str());


        if (tok->str() == "0") {
            decision = Conflict;
            for (;;) {
                if (tok->strAt(-1) == "&&") // Previous condition irrelevant for result
                    tok = removePrevCondition(tok->previous(), start)->next();
                else if (tok->strAt(-1) == "||") { // Next condition irrelevant for result
                    tok = tok->tokAt(-2);
                    tok->deleteNext(2);
                } else
                    break;
            }
        } else if (tok->isNumber()) { // 1 or higher
            decision = Known;
            for (;;) {
                if (tok->strAt(-1) == "||") // Previous condition irrelevant for result
                    tok = removePrevCondition(tok->previous(), start)->next();
                else if (tok->strAt(-1) == "&&") { // Next condition irrelevant for result
                    tok = tok->tokAt(-2);
                    tok->deleteNext(2);
                } else
                    break;
            }
        }
        if (tok == end->previous())
            break;
    }
    return end->previous();
}

Preprocessor2::Configuration::AnalysisResult Preprocessor2::Configuration::analyzeIf(Token* tok, Preprocessor2::Configuration& config, std::string& configName, SplitMode splitMode)
{
    if (tok->str() == "else") // #if was chosen. #else is bailed out
        if (config.ifDecisions.empty() || config.ifDecisions.top())
            return Conflict;
        else
            return Known;
    else if (tok->str() == "elif" && (config.ifDecisions.empty() || config.ifDecisions.top())) // #elif can't be true, when previous #if was true
        return Conflict;

    else if (Token::Match(tok, "if ! defined ( %any% ) # define %any%") && tok->tokAt(8) == tok->tokAt(7)->getLineEnd() && Token::simpleMatch(tok->previous()->link(), "# endif")) {
        const std::string& strAt4 = tok->strAt(4);
        if (strAt4 == tok->strAt(8) && (!tok->tokAt(-2) || (tok->tokAt(-2)->fileIndex() != tok->fileIndex()))) {
            if (config.defs.find(strAt4) == config.defs.end())
                return Known; // Guarded header. Return Known to avoid that cppcheck generates a configuration split.
            else
                return Conflict; // Guarded header already included. Return Conflict to avoid that cppcheck includes it a second time.
        }
    }
    simplifyIf(tok->next(), tok->getLineEnd()->next(), config, splitMode);

    tok = tok->next();

    const Token* lineEnd = tok->getLineEnd();
    // No split necessary, either true or false
    if (lineEnd == tok) {
        if (tok->str() == "0")
            return Conflict;
        if (tok->isNumber()) // 1 or higher
            return Known;
    }

    // Split. Find name of configuration.
    while (tok->str() == "(" || tok->str() == "!")
        tok = tok->next();

    if (tok->str() == "defined") {
        tok = tok->next();
        if (tok->str() == "(")
            tok = tok->next();

        configName = tok->str();
        return New;
    }
    if (tok->isName() && (tok == lineEnd || tok->str() == "||" || tok->str() == "&&")) { // Assume macro to be defined and not zero
        configName = tok->str();
        configName += "!=0";
        return New;
    } else if (Token::Match(tok, "%var% %op% %num%")) {
    } else if (Token::Match(tok, "%num% %op% %var%")) {
    }
    return Unhandled;
}

static std::string getOpposite(const std::string& s, std::string::size_type pos)
{
    std::string retVal = s.substr(0, pos);
    if (s[pos] == '!') // !=
        retVal += "=";
    else if (s[pos] == '=')
        retVal += "!=";
    else if (s[pos] == '>') {
        if (s[pos+1] == '=')
            retVal += "<";
        else
            retVal += "<=";
    } else if (s[pos] == '<') {
        if (s[pos+1] == '=')
            retVal += ">";
        else
            retVal += ">=";
    }
    retVal.append(s.substr(s.find_last_of("=><;")+1));
    return retVal;
}

void Preprocessor2::getConfigurations(const std::string& code, const std::string& filename, const std::list<std::string> &includePaths)
{
    std::queue<Configuration*> configs;
    configs.push(new Configuration(_settings, _errorLogger));
    tokenize(code, configs.back()->tokenizer, filename, 0);
    configs.back()->tok = configs.back()->tokenizer.list.front();
    if (_settings) {
        configs.back()->preDefine(_settings->userDefines);
        configs.back()->preUndef(_settings->userUndefs);
        configs.back()->defs["_CPPCHECK_MAJOR"] = CPPCHECK_MAJOR;
        configs.back()->defs["_CPPCHECK_MINOR"] = CPPCHECK_MINOR;
        if (Path::isCPP(filename)) {
            if (_settings->standards.cpp == Standards::CPP11)
                configs.back()->defs["__cplusplus"] = "201103";
            else
                configs.back()->defs["__cplusplus"] = "199711";
            configs.back()->undefs.insert("__STDC__");
            configs.back()->undefs.insert("__STDC_VERSION__");
        } else {
            configs.back()->defs["__STDC__"] = "1";
            if (_settings->standards.c == Standards::C11)
                configs.back()->defs["__STDC_VERSION__"] = "201112";
            else if (_settings->standards.c == Standards::C99)
                configs.back()->defs["__STDC_VERSION__"] = "199901";
            else
                configs.back()->defs["__STDC_VERSION__"] = "199409";
            configs.back()->undefs.insert("__cplusplus");
        }
    }

    //configs.back().tokenizer->tokens()->printOut();
    Configuration* currentCfg;
    do {
        currentCfg = configs.front();
        Token* tok = currentCfg->tok;
        bool error = false;

        while (tok) {
            if (tok->str() == "#") { // Preprocessor directive
Start:
                //print(tok, tok->getLineEnd()->next());
                //std::cout << std::endl;
                Token* next = tok->next();
                bool erase = true; // erase directive
                if (Token::Match(next, "if|elif|else")) { // Detect configurations.
                    // Analyze configuration
                    std::string configName;
                    Configuration::AnalysisResult result = Configuration::analyzeIf(next, *currentCfg, configName, splitMode);

                    bool decision = false;
                    // Handle analysis result: Either keep or remove branch or split up configuration
                    switch (result) {
                    case Configuration::Conflict: // #if conflicts to previous assumptions and defines.
                        Token::eraseTokens(next->getLineEnd(), tok->link());
                        decision = false;
                        break;
                    case Configuration::Known: // #if fits to previous assumptions. No split necessary.
                        decision = true;
                        break; // Just continue
                    case Configuration::New: { // New configuration found. Split.
                        configs.push(new Configuration(*currentCfg, tok)); // Create copy, to continue with later
                        createLinkage(configs.back()->tokenizer);

                        insertCfgToMap(configs.back()->assumptedDefs, configName); // New one assumes, its defined
                        std::string::size_type separatorPos = configName.find_first_of("=><;!");
                        if (separatorPos == std::string::npos)
                            currentCfg->assumptedNdefs.insert(configName); // Old one assumes, its not defined
                        else
                            insertCfgToMap(currentCfg->assumptedDefs, getOpposite(configName, separatorPos)); // Old one assumes, the opposite is defined

                        goto Start; // Recheck directive
                        break;
                    }
                    case Configuration::Unhandled: // Debug warning. Unhandled is not evil, since it indicates unsupported code pattern (shouldn't happen)
                        // TODO: Keep or remove?
                        decision = true;
                        break;
                    }
                    if (next->str() == "if")
                        currentCfg->ifDecisions.push(decision);
                    else if (decision && !currentCfg->ifDecisions.top())
                        currentCfg->ifDecisions.top() = true;
                } else if (next->str() == "endif") {
                    if (!currentCfg->ifDecisions.empty())
                        currentCfg->ifDecisions.pop();
                } else if (next->str() == "include") {
                    handleInclude(next, currentCfg->tokenizer, currentCfg->includedOnce, includePaths);
                } else if (next->str() == "define") {
                    bool explNoFuncMacro = !next->isExpandedMacro(); // "#define FOO (" (with space -> no function macro)
                    next = next->next();
                    Token* second = next->next();
                    if (second && (second->linenr() != next->linenr() || second->fileIndex() != next->fileIndex() || (!std::isalpha(next->str()[0]) && next->str()[0] != '_')))
                        second = 0;
                    explNoFuncMacro = explNoFuncMacro && second && second->str()[0] == '('; // Don't add ' ' if its obvious that this is no function macro
                    if (explNoFuncMacro)
                        currentCfg->defs[next->str()] = " "; // Add ' ' to indicate that this is definitely no function macro
                    else
                        currentCfg->defs[next->str()] = "";
                    if (second)
                        currentCfg->defs[next->str()] += second->stringifyList(second->getLineEnd()->next());
                    currentCfg->undefs.erase(next->str());
                } else if (next->str() == "undef") {
                    currentCfg->undefs.insert(next->strAt(1));
                    currentCfg->defs.erase(next->strAt(1));
                } else if (next->str() == "pragma") {
                    handlePragma(next, currentCfg->includedOnce, currentCfg->tokenizer.list.file(tok));
                    erase = false; // Pragma replacement reuses old tokens
                } else if (next->str() == "error") {
                    error = true; // Don't check cfg's that result in an error, since they wouldn't compile. TODO: Print error message as a compiler would? (information severity?)
                    break;
                } else if (next->str() == "asm" || next->str() == "endasm") {
                    handleAsm(next);
                    erase = false; // Pragma replacement reuses old tokens
                }

                // Remove directive
                if (erase)
                    Token::eraseTokens(tok, tok->getLineEnd()->next());
                if (tok->previous()) { // Avoid ; at the end of token list.
                    tok = tok->previous();
                    tok->deleteNext();
                    tok = tok->next();
                } else
                    tok->deleteThis();
            } else if (!tok->isExpandedMacro()) {
                tok = Preprocessor2::Configuration::replaceMacro(*currentCfg, tok, tok->str())->next();
            } else
                tok = tok->next();
        }

        if (error)
            delete currentCfg;
        else
            cfg[currentCfg->toString()] = currentCfg;
        configs.pop();
        if (_settings && !_settings->_force && cfg.size() >=_settings->_maxConfigs) { // If config limit reached: break
            error_tooManyConfigs(filename);
            break;
        }
    } while (!configs.empty()); // If all cfg's handled: break.

    while (!configs.empty()) { // Remove remaining cfg's
        delete configs.front();
        configs.pop();
    }
}

void Preprocessor2::writeError(const std::string &fileName, const unsigned int linenr, ErrorLogger *errorLogger, const std::string &errorType, const std::string &errorText)
{
    if (!errorLogger)
        return;

    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    ErrorLogger::ErrorMessage::FileLocation loc;
    loc.line = linenr;
    loc.setfile(fileName);
    locationList.push_back(loc);
    errorLogger->reportErr(ErrorLogger::ErrorMessage(locationList,
                           Severity::error,
                           errorText,
                           errorType,
                           false));
}

// Report that include is missing
void Preprocessor2::error_missingInclude(const Token* tok, const Tokenizer& tokenizer, const std::string &header, bool userheader)
{
    const std::string fname = Path::fromNativeSeparators(tokenizer.list.file(tok));
    if (_settings->nomsg.isSuppressed("missingInclude", fname, tok->linenr()))
        return;
    if (!userheader && _settings->nomsg.isSuppressed("missingIncludeSystem", fname, tok->linenr()))
        return;

    if (!userheader)
        missingSystemIncludeFlag = true;
    else
        missingIncludeFlag = true;
    if (_errorLogger && _settings->checkConfiguration) {

        std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
        if (tok && !tokenizer.list.file(tok).empty()) {
            ErrorLogger::ErrorMessage::FileLocation loc;
            loc.line = tok->linenr();
            loc.setfile(tokenizer.list.file(tok));
            locationList.push_back(loc);
        }
        ErrorLogger::ErrorMessage errmsg(locationList, Severity::information,
                                         (!userheader) ?
                                         "Include file: <" + header + "> not found. Please note: Cppcheck does not need standard library headers to get proper results." :
                                         "Include file: \"" + header + "\" not found.",
                                         (!userheader) ? "missingIncludeSystem" : "missingInclude",
                                         false);
        errmsg.file0 = tokenizer.list.getFiles().front();
        _errorLogger->reportInfo(errmsg);
    }
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/**
 * Try to open header
 * @param filename header name (in/out)
 * @param includePaths paths where to look for the file
 * @param fin file input stream (in/out)
 * @return if file is opened then true is returned
 */
static bool openHeader(std::string &filename, const std::list<std::string> &includePaths, const std::string &filePath, std::ifstream &fin)
{
    std::list<std::string> includePaths2(includePaths);
    includePaths2.push_front(filePath);

    for (std::list<std::string>::const_iterator iter = includePaths2.begin(); iter != includePaths2.end(); ++iter) {
        const std::string nativePath(Path::toNativeSeparators(*iter));
        fin.open((nativePath + filename).c_str());
        if (fin.is_open()) {
            filename = nativePath + filename;
            return true;
        }
        fin.clear();
    }

    return false;
}

static unsigned char readChar(std::istream &istr, unsigned int bom)
{
    unsigned char ch = (unsigned char)istr.get();

    // For UTF-16 encoded files the BOM is 0xfeff/0xfffe. If the
    // character is non-ASCII character then replace it with 0xff
    if (bom == 0xfeff || bom == 0xfffe) {
        unsigned char ch2 = (unsigned char)istr.get();
        int ch16 = (bom == 0xfeff) ? (ch<<8 | ch2) : (ch2<<8 | ch);
        ch = (unsigned char)((ch16 >= 0x80) ? 0xff : ch16);
    }

    // Handling of newlines..
    if (ch == '\r') {
        ch = '\n';
        if (bom == 0 && (char)istr.peek() == '\n')
            (void)istr.get();
        else if (bom == 0xfeff || bom == 0xfffe) {
            int c1 = istr.get();
            int c2 = istr.get();
            int ch16 = (bom == 0xfeff) ? (c1<<8 | c2) : (c2<<8 | c1);
            if (ch16 != '\n') {
                istr.unget();
                istr.unget();
            }
        }
    }

    return ch;
}

// Concatenates a list of strings, inserting a separator between parts
std::string join(const std::set<std::string>& list, char separator)
{
    std::string s;
    for (std::set<std::string>::const_iterator it = list.begin(); it != list.end(); ++it) {
        if (!s.empty())
            s += separator;

        s += *it;
    }
    return s;
}


static bool hasbom(const std::string &str)
{
    return bool(str.size() >= 3 &&
                static_cast<unsigned char>(str[0]) == 0xef &&
                static_cast<unsigned char>(str[1]) == 0xbb &&
                static_cast<unsigned char>(str[2]) == 0xbf);
}


// This wrapper exists because Sun's CC does not allow a static_cast
// from extern "C" int(*)(int) to int(*)(int).
static int tolowerWrapper(int c)
{
    return std::tolower(c);
}

static bool isspaceWrapper(char c)
{
    return std::isspace(c) != 0;
}

static bool isFallThroughComment(std::string comment)
{
    // convert comment to lower case without whitespace
    std::remove_if(comment.begin(), comment.end(), &isspaceWrapper);
    std::transform(comment.begin(), comment.end(), comment.begin(), &tolowerWrapper);

    return comment.find("fallthr") != std::string::npos ||
           comment.find("fallsthr") != std::string::npos ||
           comment.find("fall-thr") != std::string::npos ||
           comment.find("dropthr") != std::string::npos ||
           comment.find("passthr") != std::string::npos ||
           comment.find("nobreak") != std::string::npos ||
           comment == "fall";
}


/// -----------------------------------------------------------



void Preprocessor2::error_tooManyConfigs(const std::string &file)
{
    if (!_settings || !_settings->isEnabled("information"))
        return;

    std::list<ErrorLogger::ErrorMessage::FileLocation> loclist;
    if (!file.empty()) {
        ErrorLogger::ErrorMessage::FileLocation location;
        location.setfile(file);
        loclist.push_back(location);
    }

    std::ostringstream msg;
    msg << "Too many #ifdef configurations - cppcheck only checks " << _settings->_maxConfigs
        << " configurations. Use --force to check all configurations. For more details, use --enable=information.\n"
        "The checking of the file will be interrupted because there are too many "
        "#ifdef configurations. Checking of all #ifdef configurations can be forced "
        "by --force command line option or from GUI preferences. However that may "
        "increase the checking time.";


    ErrorLogger::ErrorMessage errmsg(loclist,
                                     Severity::information,
                                     msg.str(),
                                     "toomanyconfigs",
                                     false);

    _errorLogger->reportErr(errmsg);
}

void Preprocessor2::getErrorMessages(ErrorLogger *errorLogger, Settings *settings)
{
    Preprocessor2 preprocessor(settings, errorLogger);
    /* preprocessor.missingInclude("", 1, "");
     preprocessor.validateCfgError("X");
     preprocessor.error("", 1, "#error message");   // #error ..*/
}