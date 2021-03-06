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
#ifndef ctuH
#define ctuH
//---------------------------------------------------------------------------

#include "check.h"
#include "errorlogger.h"
#include "valueflow.h"

#include <map>
#include <list>

class Function;

/// @addtogroup Core
/// @{


/** @brief Whole program analysis (ctu=Cross Translation Unit) */
namespace CTU {
    class CPPCHECKLIB CTUInfo {
    public:
        enum class InvalidValueType { null, uninit, bufferOverflow };

        struct Location {
            Location() = default;
            Location(const Tokenizer *tokenizer, const Token *tok);
            Location(const std::string &fileName, unsigned int lineNumber, unsigned int column) : fileName(fileName), lineNumber(lineNumber), column(column) {}
            std::string fileName;
            unsigned int lineNumber;
            unsigned int column;
        };

        struct UnsafeUsage {
            UnsafeUsage() = default;
            UnsafeUsage(const std::string &myId, unsigned int myArgNr, const std::string &myArgumentName, const Location &location, MathLib::bigint value) : myId(myId), myArgNr(myArgNr), myArgumentName(myArgumentName), location(location), value(value) {}
            std::string myId;
            unsigned int myArgNr;
            std::string myArgumentName;
            Location location;
            MathLib::bigint value;
            tinyxml2::XMLElement* toXMLElement(tinyxml2::XMLDocument* doc) const;
        };

        class CallBase {
        public:
            CallBase() = default;
            CallBase(const std::string &callId, unsigned int callArgNr, const std::string &callFunctionName, const Location &loc)
                : callId(callId), callArgNr(callArgNr), callFunctionName(callFunctionName), location(loc)
            {}
            CallBase(const Tokenizer *tokenizer, const Token *callToken);
            virtual ~CallBase() {}
            std::string callId;
            unsigned int callArgNr;
            std::string callFunctionName;
            Location location;
        protected:
            bool loadBaseFromXml(const tinyxml2::XMLElement *xmlElement);
        };

        class FunctionCall : public CallBase {
        public:
            std::string callArgumentExpression;
            MathLib::bigint callArgValue;
            ValueFlow::Value::ValueType callValueType;
            std::vector<ErrorMessage::FileLocation> callValuePath;
            bool warning;

            tinyxml2::XMLElement* toXMLElement(tinyxml2::XMLDocument* doc) const;
            bool loadFromXml(const tinyxml2::XMLElement *xmlElement);
        };

        class NestedCall : public CallBase {
        public:
            NestedCall() = default;

            NestedCall(const std::string &myId, unsigned int myArgNr, const std::string &callId, unsigned int callArgnr, const std::string &callFunctionName, const Location &location)
                : CallBase(callId, callArgnr, callFunctionName, location),
                  myId(myId),
                  myArgNr(myArgNr) {
            }

            NestedCall(const Tokenizer *tokenizer, const Function *myFunction, const Token *callToken);

            tinyxml2::XMLElement* toXMLElement(tinyxml2::XMLDocument* doc) const;
            bool loadFromXml(const tinyxml2::XMLElement *xmlElement);

            std::string myId;
            unsigned int myArgNr;
        };

        CTUInfo(const std::string& sourcefile_, std::size_t filesize_, const std::string& analyzerfile_)
            : sourcefile(sourcefile_)
            , analyzerfile(analyzerfile_)
            , analyzerfileExists(false)
            , filesize(filesize_)
            , mChecksum(false) {
        }
        ~CTUInfo();
        void addCheckInfo(const std::string& check, Check::FileInfo* fileInfo);
        void parseTokens(const Tokenizer* tokenizer);
        Check::FileInfo* getCheckInfo(const std::string& check) const;
        void reportErr(const ErrorMessage& msg);
        bool tryLoadFromFile(uint32_t checksum);
        void writeFile();

        std::string sourcefile;
        std::string analyzerfile;
        bool analyzerfileExists;
        std::size_t filesize;
        uint32_t mChecksum;

        std::list<ErrorMessage> mErrors;

        std::list<FunctionCall> functionCalls;
        std::list<NestedCall> nestedCalls;

        void loadFromXml(const tinyxml2::XMLElement *xmlElement);
        std::map<std::string, std::vector<const CallBase *>> getCallsMap() const;

        std::list<ErrorMessage::FileLocation> getErrorPath(InvalidValueType invalidValue,
                const UnsafeUsage &unsafeUsage,
                const std::map<std::string, std::vector<const CallBase *>> &callsMap,
                const char info[],
                const FunctionCall * * const functionCallPtr,
                bool warning) const;

        std::map<std::string, Check::FileInfo*> mCheckInfo;
    };

    extern int maxCtuDepth;

    std::list<CTUInfo::UnsafeUsage> getUnsafeUsage(const Context& ctx, const Check *check, bool (*isUnsafeUsage)(const Check *check, const Token *argtok, MathLib::bigint *_value));

    std::list<CTUInfo::UnsafeUsage> loadUnsafeUsageListFromXml(const tinyxml2::XMLElement *xmlElement);
}

/// @}
//---------------------------------------------------------------------------
#endif // ctuH
