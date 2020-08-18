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
#ifndef checkbufferoverrunH
#define checkbufferoverrunH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"
#include "ctu.h"
#include "mathlib.h"
#include "symboldatabase.h"
#include "valueflow.h"

#include <cstddef>
#include <list>
#include <map>
#include <string>
#include <vector>

namespace tinyxml2 {
    class XMLElement;
}

/// @addtogroup Checks
/// @{

/**
 * @brief buffer overruns and array index out of bounds
 *
 * Buffer overrun and array index out of bounds are pretty much the same.
 * But I generally use 'array index' if the code contains []. And the given
 * index is out of bounds.
 * I generally use 'buffer overrun' if you for example call a strcpy or
 * other function and pass a buffer and reads or writes too much data.
 */
class CPPCHECKLIB CheckBufferOverrun : public Check {
public:

    /** This constructor is used when registering the CheckClass */
    CheckBufferOverrun() : Check(myName()) {
    }

    /** This constructor is used when running checks. */
    explicit CheckBufferOverrun(Context ctx)
        : Check(myName(), ctx) {
    }

    void runChecks(Context ctx) override {
        CheckBufferOverrun checkBufferOverrun(ctx);
        checkBufferOverrun.arrayIndex();
        checkBufferOverrun.pointerArithmetic();
        checkBufferOverrun.bufferOverflow();
        checkBufferOverrun.arrayIndexThenCheck();
        checkBufferOverrun.stringNotZeroTerminated();
        checkBufferOverrun.objectIndex();
        checkBufferOverrun.negativeArraySize();
        checkBufferOverrun.checkInsecureCmdLineArgs();
    }

    void getErrorMessages(Context ctx) const override {
        CheckBufferOverrun c(ctx);
        c.arrayIndexError(nullptr, std::vector<Dimension>(), std::vector<const ValueFlow::Value *>());
        c.pointerArithmeticError(nullptr, nullptr, nullptr);
        c.negativeIndexError(nullptr, std::vector<Dimension>(), std::vector<const ValueFlow::Value *>());
        c.arrayIndexThenCheckError(nullptr, "i");
        c.bufferOverflowError(nullptr, nullptr);
        c.objectIndexError(nullptr, nullptr, true);
        c.negativeMemoryAllocationSizeError(nullptr);
        c.negativeArraySizeError(nullptr);
        c.negativeMemoryAllocationSizeError(nullptr);
        c.cmdLineArgsError(nullptr);
    }

    /** @brief Parse current TU and extract file info */
    Check::FileInfo* getFileInfo(Context ctx) const override;

    Check::FileInfo* loadFileInfoFromXml(const tinyxml2::XMLElement* xmlElement) const override;

    /** @brief Analyse all file infos for all TU */
    bool analyseWholeProgram(const CTU::CTUInfo* ctu, AnalyzerInformation& analyzerInformation, Context ctx) override;

private:

    void arrayIndex();
    void arrayIndexError(const Token *tok, const std::vector<Dimension> &dimensions, const std::vector<const ValueFlow::Value *> &indexes);
    void negativeIndexError(const Token *tok, const std::vector<Dimension> &dimensions, const std::vector<const ValueFlow::Value *> &indexes);

    void pointerArithmetic();
    void pointerArithmeticError(const Token *tok, const Token *indexToken, const ValueFlow::Value *indexValue);

    void bufferOverflow();
    void bufferOverflowError(const Token *tok, const ValueFlow::Value *value);

    void arrayIndexThenCheck();
    void arrayIndexThenCheckError(const Token *tok, const std::string &indexName);

    void stringNotZeroTerminated();
    void terminateStrncpyError(const Token *tok, const std::string &varname);
    void bufferNotZeroTerminatedError(const Token *tok, const std::string &varname, const std::string &function);

    void negativeArraySize();
    void negativeArraySizeError(const Token* tok);
    void negativeMemoryAllocationSizeError(const Token* tok); // provide a negative value to memory allocation function

    void checkInsecureCmdLineArgs();
    void cmdLineArgsError(const Token* tok);

    void objectIndex();
    void objectIndexError(const Token *tok, const ValueFlow::Value *v, bool known);

    ValueFlow::Value getBufferSize(const Token *bufTok) const;

    // CTU

    static bool isCtuUnsafeBufferUsage(const Check *check, const Token *argtok, MathLib::bigint *offset, int type);
    static bool isCtuUnsafeArrayIndex(const Check *check, const Token *argtok, MathLib::bigint *offset);
    static bool isCtuUnsafePointerArith(const Check *check, const Token *argtok, MathLib::bigint *offset);

    bool analyseWholeProgram1(const CTU::CTUInfo *ctu, const std::map<std::string, std::vector<const CTU::CTUInfo::CallBase *>> &callsMap, const CTU::CTUInfo::UnsafeUsage &unsafeUsage, int type, ErrorLogger &errorLogger);


    static const char* myName() {
        return "BufferOverrun";
    }

    std::string classInfo() const override {
        return "Out of bounds checking:\n"
               "- Array index out of bounds\n"
               "- Pointer arithmetic overflow\n"
               "- Buffer overflow\n"
               "- Dangerous usage of strncat()\n"
               "- Using array index before checking it\n"
               "- Partial string write that leads to buffer that is not zero terminated.\n"
               "- Allocating memory with a negative size\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkbufferoverrunH
