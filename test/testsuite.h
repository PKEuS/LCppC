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


#ifndef testsuiteH
#define testsuiteH

#include "config.h"
#include "errorlogger.h"

#include <cstddef>
#include <set>
#include <sstream>

class options;

class TestFixture : public ErrorLogger {
private:
    static std::ostringstream errmsg;
    static std::size_t countTests;
    static std::size_t fails_counter;
    static std::size_t todos_counter;
    static std::size_t succeeded_todos_counter;
    static std::set<std::string> missingLibs;
    std::string mTemplateFormat;
    std::string mTemplateLocation;
    std::string mTestname;
    std::string mFilename;
    std::string testToRun;
    bool quiet_tests;

protected:

    virtual void run() = 0;

    bool prepareTest(const char filename[], const char testname[]);
    void outputLocationStr(const unsigned int linenr) const;

    bool assert_(const unsigned int linenr, const bool condition) const;

    bool assertEquals(const unsigned int linenr, const std::string &expected, const std::string &actual, const std::string &msg = emptyString) const;
    void assertEqualsWithoutLineNumbers(const unsigned int linenr, const std::string &expected, const std::string &actual, const std::string &msg = emptyString) const;
    bool assertEquals(const unsigned int linenr, const char expected[], const std::string& actual, const std::string &msg = emptyString) const;
    bool assertEquals(const unsigned int linenr, const char expected[], const char actual[], const std::string &msg = emptyString) const;
    bool assertEquals(const unsigned int linenr, const std::string& expected, const char actual[], const std::string &msg = emptyString) const;
    bool assertEquals(const unsigned int linenr, const long long expected, const long long actual, const std::string &msg = emptyString) const;
    void assertEqualsDouble(const unsigned int linenr, const double expected, const double actual, const double tolerance, const std::string &msg = emptyString) const;

    void todoAssertEquals(const unsigned int linenr, const std::string &wanted,
                          const std::string &current, const std::string &actual) const;
    void todoAssertEquals(const unsigned int linenr, const char wanted[],
                          const char current[], const std::string &actual) const;
    void todoAssertEquals(const unsigned int linenr, const long long wanted,
                          const long long current, const long long actual) const;
    void assertThrow(const unsigned int linenr) const;
    void assertThrowFail(const unsigned int linenr) const;
    void assertNoThrowFail(const unsigned int linenr) const;
    void complainMissingLib(const char * const libname) const;
    std::string deleteLineNumber(const std::string &message) const;

    void setMultiline();

    void processOptions(const options& args);
public:
    void reportOut(const std::string &outmsg) override;
    void reportErr(const ErrorMessage &msg) override;
    void run(const std::string &str);
    static void printHelp();
    const std::string classname;

    explicit TestFixture(const char * const _name);
    ~TestFixture() override { }

    static std::size_t runTests(const options& args);
};

extern std::ostringstream errout;
extern std::ostringstream output;

#define TEST_CASE( NAME )  do { if ( prepareTest(__FILE__, #NAME) ) { NAME(); } } while(false)
#define ASSERT( CONDITION )  if (!assert_(__LINE__, (CONDITION))) return
#define ASSERT_EQUALS( EXPECTED , ACTUAL )  assertEquals(__LINE__, (EXPECTED), (ACTUAL))
#define ASSERT_EQUALS_WITHOUT_LINENUMBERS( EXPECTED , ACTUAL )  assertEqualsWithoutLineNumbers(__LINE__, EXPECTED, ACTUAL)
#define ASSERT_EQUALS_DOUBLE( EXPECTED , ACTUAL, TOLERANCE )  assertEqualsDouble(__LINE__, EXPECTED, ACTUAL, TOLERANCE)
#define ASSERT_EQUALS_MSG( EXPECTED , ACTUAL, MSG )  assertEquals(__LINE__, EXPECTED, ACTUAL, MSG)
#define ASSERT_THROW( CMD, EXCEPTION ) do { try { CMD ; assertThrowFail(__LINE__); } catch (const EXCEPTION&) { } catch (...) { assertThrowFail(__LINE__); } } while(false)
#define ASSERT_THROW_EQUALS( CMD, EXCEPTION, EXPECTED ) do { try { CMD ; assertThrowFail(__LINE__); } catch (const EXCEPTION& e) { assertEquals(__LINE__, EXPECTED, e.errorMessage); } catch (...) { assertThrowFail(__LINE__); } } while(false)
#define ASSERT_NO_THROW( CMD ) do { try { CMD ; } catch (...) { assertNoThrowFail(__LINE__); } } while(false)
#define TODO_ASSERT_THROW( CMD, EXCEPTION ) do { try { CMD ; } catch (const EXCEPTION&) { } catch (...) { assertThrow(__LINE__); } } while(false)
#define TODO_ASSERT( CONDITION ) do { const bool condition=(CONDITION); todoAssertEquals(__LINE__, true, false, condition); } while(false)
#define TODO_ASSERT_EQUALS( WANTED , CURRENT , ACTUAL ) todoAssertEquals(__LINE__, WANTED, CURRENT, ACTUAL)
#define REGISTER_TEST( CLASSNAME ) namespace { CLASSNAME instance_##CLASSNAME; }

#ifdef _WIN32
#define LOAD_LIB_2( LIB, NAME ) do { { if (((LIB).load("./testrunner", "../cfg/" NAME).errorcode != Library::OK) && ((LIB).load("./testrunner", "cfg/" NAME).errorcode != Library::OK)) { complainMissingLib(NAME); return; } } } while(false)
#else
#define LOAD_LIB_2( LIB, NAME ) do { { if (((LIB).load("./testrunner", "cfg/" NAME).errorcode != Library::OK) && ((LIB).load("./bin/testrunner", "bin/cfg/" NAME).errorcode != Library::OK)) { complainMissingLib(NAME); return; } } } while(false)
#endif

#endif
