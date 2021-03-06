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
#ifndef checkmemoryleakH
#define checkmemoryleakH
//---------------------------------------------------------------------------

/**
 * @file
 *
 * %Check for memory leaks
 *
 * The checking is split up into three specialized classes.
 * - CheckMemoryLeakInFunction can detect when a function variable is allocated but not deallocated properly.
 * - CheckMemoryLeakInClass can detect when a class variable is allocated but not deallocated properly.
 * - CheckMemoryLeakStructMember checks allocation/deallocation of structs and struct members
 */

#include "check.h"
#include "config.h"
#include "errortypes.h"
#include "tokenize.h"

#include <list>
#include <string>

class Function;
class Scope;
class Variable;

/// @addtogroup Core
/// @{

/** @brief Base class for memory leaks checking */
class CPPCHECKLIB CheckMemoryLeak {
private:
    /** For access to the tokens */
    Context mCtx_;

    /**
     * Report error. Similar with the function Check::reportError
     * @param tok the token where the error occurs
     * @param severity the severity of the bug
     * @param id type of message
     * @param msg text
     * @param cwe cwe number
     */
    void reportErr(const Token *tok, Severity::SeverityType severity, const std::string &id, const std::string &msg, CWE cwe) const;

    /**
     * Report error. Similar with the function Check::reportError
     * @param callstack callstack of error
     * @param severity the severity of the bug
     * @param id type of message
     * @param msg text
     * @param cwe cwe number
     */
    void reportErr(const std::vector<const Token *> &callstack, Severity::SeverityType severity, const std::string &id, const std::string &msg, CWE cwe) const;

public:
    CheckMemoryLeak() = default;
    CheckMemoryLeak(const CheckMemoryLeak &) = delete;
    CheckMemoryLeak& operator=(const CheckMemoryLeak &) = delete;

    explicit CheckMemoryLeak(const Context& ctx)
        : mCtx_(ctx) {
    }

    /** @brief What type of allocation are used.. the "Many" means that several types of allocation and deallocation are used */
    enum AllocType { No, Malloc, New, NewArray, File, Fd, Pipe, OtherMem, OtherRes, Many };

    void memoryLeak(const Token *tok, const std::string &varname, AllocType alloctype) const;

    /**
     * @brief Get type of deallocation at given position
     * @param tok position
     * @param varid variable id
     * @return type of deallocation
     */
    AllocType getDeallocationType(const Token *tok, unsigned int varid) const;

    /**
     * @brief Get type of allocation at given position
     */
    AllocType getAllocationType(const Token *tok2, unsigned int varid, std::vector<const Function*> *callstack = nullptr) const;

    /**
     * @brief Get type of reallocation at given position
     */
    AllocType getReallocationType(const Token *tok2, unsigned int varid) const;

    /**
     * Check if token reopens a standard stream
     * @param tok token to check
     */
    bool isReopenStandardStream(const Token *tok) const;
    /**
     * Report that there is a memory leak (new/malloc/etc)
     * @param tok token where memory is leaked
     * @param varname name of variable
     */
    void memleakError(const Token *tok, const std::string &varname) const;

    /**
     * Report that there is a resource leak (fopen/popen/etc)
     * @param tok token where resource is leaked
     * @param varname name of variable
     */
    void resourceLeakError(const Token *tok, const std::string &varname) const;

    /**
     * @brief Report error: deallocating a deallocated pointer
     * @param tok token where error occurs
     * @param varname name of variable
     */
    void deallocDeallocError(const Token *tok, const std::string &varname) const;
    void deallocuseError(const Token *tok, const std::string &varname) const;
    void mismatchSizeError(const Token *tok, const std::string &sz) const;
    void mismatchAllocDealloc(const std::vector<const Token *> &callstack, const std::string &varname) const;
    void memleakUponReallocFailureError(const Token *tok, const std::string &reallocfunction, const std::string &varname) const;

    /** What type of allocated memory does the given function return? */
    AllocType functionReturnType(const Function* func, std::vector<const Function*> *callstack = nullptr) const;

    /** Function allocates pointed-to argument (a la asprintf)? */
    const char *functionArgAlloc(const Function *func, unsigned int targetpar, AllocType &allocType) const;
};

/// @}



/// @addtogroup Checks
/// @{


/**
 * @brief %CheckMemoryLeakInFunction detects when a function variable is allocated but not deallocated properly.
 *
 * The checking is done by looking at each function variable separately. By repeating these 4 steps over and over:
 * -# locate a function variable
 * -# create a simple token list that describes the usage of the function variable.
 * -# simplify the token list.
 * -# finally, check if the simplified token list contain any leaks.
 */

class CPPCHECKLIB CheckMemoryLeakInFunction : private Check, public CheckMemoryLeak {
public:
    /** @brief This constructor is used when registering this class */
    CheckMemoryLeakInFunction() : Check(myName()), CheckMemoryLeak() {
    }

    /** @brief This constructor is used when running checks */
    explicit CheckMemoryLeakInFunction(const Context& ctx)
        : Check(myName(), ctx), CheckMemoryLeak(ctx) {
    }

    void runChecks(const Context& ctx) override {
        CheckMemoryLeakInFunction checkMemoryLeak(ctx);
        checkMemoryLeak.checkReallocUsage();
    }

    /**
     * Checking for a memory leak caused by improper realloc usage.
     */
    void checkReallocUsage();

private:
    /** Report all possible errors (for the --errorlist) */
    void getErrorMessages(const Context& ctx) const override {
        CheckMemoryLeakInFunction c(ctx);

        c.memleakError(nullptr, "varname");
        c.resourceLeakError(nullptr, "varname");

        c.deallocDeallocError(nullptr, "varname");
        c.deallocuseError(nullptr, "varname");
        c.mismatchSizeError(nullptr, "sz");
        const std::vector<const Token *> callstack;
        c.mismatchAllocDealloc(callstack, "varname");
        c.memleakUponReallocFailureError(nullptr, "realloc", "varname");
    }

    /**
     * Get name of class (--doc)
     * @return name of class
     */
    static const char* myName() {
        return "MemoryLeakInFunction";
    }

    /**
     * Get class information (--doc)
     * @return Wiki formatted information about this class
     */
    std::string classInfo() const override {
        return "Is there any allocated memory when a function goes out of scope\n";
    }
};



/**
 * @brief %Check class variables, variables that are allocated in the constructor should be deallocated in the destructor
 */

class CPPCHECKLIB CheckMemoryLeakInClass : private Check, private CheckMemoryLeak {
public:
    CheckMemoryLeakInClass() : Check(myName()), CheckMemoryLeak() {
    }

    explicit CheckMemoryLeakInClass(const Context& ctx)
        : Check(myName(), ctx), CheckMemoryLeak(ctx) {
    }

    void runChecks(const Context& ctx) override {
        if (!ctx.tokenizer->isCPP())
            return;

        CheckMemoryLeakInClass checkMemoryLeak(ctx);
        checkMemoryLeak.check();
    }

    void check();

private:
    void variable(const Scope *scope, const Token *tokVarname);

    /** Public functions: possible double-allocation */
    void checkPublicFunctions(const Scope *scope, const Token *classtok);
    void publicAllocationError(const Token *tok, const std::string &varname);

    void unsafeClassError(const Token *tok, const std::string &classname, const std::string &varname);

    void getErrorMessages(const Context& ctx) const override {
        CheckMemoryLeakInClass c(ctx);
        c.publicAllocationError(nullptr, "varname");
        c.unsafeClassError(nullptr, "class", "class::varname");
    }

    static const char* myName() {
        return "MemoryLeakInClass";
    }

    std::string classInfo() const override {
        return "If the constructor allocate memory then the destructor must deallocate it.\n";
    }
};



/** @brief detect simple memory leaks for struct members */

class CPPCHECKLIB CheckMemoryLeakStructMember : private Check, private CheckMemoryLeak {
public:
    CheckMemoryLeakStructMember() : Check(myName()), CheckMemoryLeak() {
    }

    explicit CheckMemoryLeakStructMember(const Context& ctx)
        : Check(myName(), ctx), CheckMemoryLeak(ctx) {
    }

    void runChecks(const Context& ctx) override {
        CheckMemoryLeakStructMember checkMemoryLeak(ctx);
        checkMemoryLeak.check();
    }

    void check();

private:

    /** Is local variable allocated with malloc? */
    static bool isMalloc(const Variable *variable);

    void checkStructVariable(const Variable * const variable);

    void getErrorMessages(const Context&) const override {
    }

    static const char* myName() {
        return "MemoryLeakInStruct";
    }

    std::string classInfo() const override {
        return "Don't forget to deallocate struct members\n";
    }
};



/** @brief detect simple memory leaks (address not taken) */

class CPPCHECKLIB CheckMemoryLeakNoVar : private Check, private CheckMemoryLeak {
public:
    CheckMemoryLeakNoVar() : Check(myName()), CheckMemoryLeak() {
    }

    explicit CheckMemoryLeakNoVar(const Context& ctx)
        : Check(myName(), ctx), CheckMemoryLeak(ctx) {
    }

    void runChecks(const Context& ctx) override {
        CheckMemoryLeakNoVar checkMemoryLeak(ctx);
        checkMemoryLeak.check();
    }

    void check();

private:
    /**
     * @brief %Check if an input argument to a function is the return value of an allocation function
     * like malloc(), and the function does not release it.
     * @param scope     The scope of the function to check.
     */
    void checkForUnreleasedInputArgument(const Scope *scope);

    /**
     * @brief %Check if a call to an allocation function like malloc() is made and its return value is not assigned.
     * @param scope     The scope of the function to check.
     */
    void checkForUnusedReturnValue(const Scope *scope);

    /**
     * @brief %Check if an exception could cause a leak in an argument constructed with shared_ptr/unique_ptr.
     * @param scope     The scope of the function to check.
     */
    void checkForUnsafeArgAlloc(const Scope *scope);

    void functionCallLeak(const Token *loc, const std::string &alloc, const std::string &functionCall);
    void returnValueNotUsedError(const Token* tok, const std::string &alloc);
    void unsafeArgAllocError(const Token *tok, const std::string &funcName, const std::string &ptrType, const std::string &objType);

    void getErrorMessages(const Context& ctx) const override {
        CheckMemoryLeakNoVar c(ctx);

        c.functionCallLeak(nullptr, "funcName", "funcName");
        c.returnValueNotUsedError(nullptr, "funcName");
        c.unsafeArgAllocError(nullptr, "funcName", "shared_ptr", "int");
    }

    static const char* myName() {
        return "MemoryLeakReturnValue";
    }

    std::string classInfo() const override {
        return "Not taking the address to allocated memory\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkmemoryleakH
