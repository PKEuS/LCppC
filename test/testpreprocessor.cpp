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


// The preprocessor that Cppcheck uses is a bit special. Instead of generating
// the code for a known configuration, it generates the code for each configuration.


#include "testsuite.h"
#include "preprocessor.h"
#include "tokenize.h"
#include "token.h"
#include "settings.h"

#include <map>
#include <set>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdio>

extern std::ostringstream errout;
extern std::ostringstream output;

#ifdef _MSC_VER
// Visual Studio complains about truncated values for '(char)0xff' and '(char)0xfe'
// TODO: Is there any nice way to fix these warnings?
#pragma warning( disable : 4310 )
#endif

class TestPreprocessor : public TestFixture {
    std::set<std::string> includeFiles;

public:
    TestPreprocessor() : TestFixture("TestPreprocessor") {
        Preprocessor2::macroChar = '$';
    }
    ~TestPreprocessor() {
        for (std::set<std::string>::const_iterator i = includeFiles.begin(); i != includeFiles.end(); ++i)
            std::remove(i->c_str());
    }

    /* class OurPreprocessor : public Preprocessor {
     public:
         using Preprocessor::replaceIfDefined;
         using Preprocessor::getHeaderFileName;

         static std::string expandMacros(const std::string& code, ErrorLogger *errorLogger = 0) {
             return Preprocessor::expandMacros(code, "file.cpp", "", errorLogger);
         }
     };*/

private:

    void createIncludeFile(const char filename[], const char data[]) {
        std::ofstream ofs(filename, std::ios_base::trunc|std::ios_base::out);
        ofs << data;
        includeFiles.insert(filename);
    }

    void preprocess(Preprocessor2& preprocessor, const char filedata[]) {
        std::istringstream istr(filedata);
        std::string data = preprocessor.readCode(istr);
        preprocessor.simplifyString(data, "file.c");
        preprocessor.getConfigurations(data, "file.c");
    }

    std::string preprocEmptyCfg(const char filedata[], bool cpp = false, Settings* settings = 0) {
        Preprocessor2 preprocessor(settings, this);
        std::istringstream istr(filedata);
        std::string data = preprocessor.readCode(istr);
        preprocessor.simplifyString(data, cpp?"file.cpp":"file.c");
        preprocessor.getConfigurations(data, cpp?"file.cpp":"file.c");
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""])
            return (preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        else
            return ("");
    }

    void run() {
        // Just read the code into a string. Perform simple cleanup of the code
        TEST_CASE(readCode1);
        TEST_CASE(readCode2); // #4308 - convert C++11 raw string to plain old C string
        TEST_CASE(readCode3); // #4351 - escaped whitespace in gcc

        // reading utf-16 file
        TEST_CASE(utf16);

        TEST_CASE(newlines);

        TEST_CASE(simplifyCode1);
        TEST_CASE(simplifyCode2);
        TEST_CASE(simplifyCode3);
        TEST_CASE(simplifyCode4);

        TEST_CASE(diTrigraphs);

        TEST_CASE(linkage);

        TEST_CASE(include1);
        TEST_CASE(include2);
        ///TEST_CASE(includeRec); // TODO
        TEST_CASE(missingInclude); // Show 'missing include' warnings
        TEST_CASE(inline_suppression_for_missing_include); // inline suppression, missingInclude

        // The bug that started the whole work with the new preprocessor
        TEST_CASE(Bug2190219);

        TEST_CASE(split1);
        TEST_CASE(split2);
        TEST_CASE(split3);
        TEST_CASE(split4);
        TEST_CASE(split5a);
        TEST_CASE(split5b);
        TEST_CASE(split5c);
        TEST_CASE(split5d);
        TEST_CASE(split6);
        TEST_CASE(split7);
        TEST_CASE(test8);  // #if A==1  => cfg: A=1
        TEST_CASE(test9);  // Don't crash for invalid code
        TEST_CASE(test10); // Ticket #5139

        // #error => don't extract any code
        TEST_CASE(error1);
        TEST_CASE(error2); // #error with extended chars
        TEST_CASE(error3);
        TEST_CASE(error4);

        TEST_CASE(if0_exclude);
        TEST_CASE(if0_else);
        TEST_CASE(if0_elif);

        // Handling include guards (don't create extra configuration for it)
        TEST_CASE(includeguard1);
        TEST_CASE(includeguard2);
        TEST_CASE(includeguard3);

        TEST_CASE(comments1);
        TEST_CASE(removeComments);

        TEST_CASE(elif);

        TEST_CASE(if_cond1);
        TEST_CASE(if_cond2);
        TEST_CASE(if_cond3);
        TEST_CASE(if_cond4);
        TEST_CASE(if_cond5);
        TEST_CASE(if_cond6);
        TEST_CASE(if_cond8);
        TEST_CASE(if_cond9);
        TEST_CASE(if_cond10);
        //TEST_CASE(if_cond11);
        TEST_CASE(if_cond12);
        TEST_CASE(if_cond13);
        TEST_CASE(if_cond14);
        TEST_CASE(if_cond15); // #4456 - segfault

        TEST_CASE(if_or_1);

        TEST_CASE(if_macro_eq_macro); // #3536
        TEST_CASE(ticket_3675);
        TEST_CASE(ticket_3699);
        TEST_CASE(ticket_4922); // #4922

        TEST_CASE(multiline1);
        TEST_CASE(multiline2);
        TEST_CASE(multiline3);
        TEST_CASE(multiline4);
        TEST_CASE(multiline5);

        // Macros..
        TEST_CASE(macro_simple1);
        TEST_CASE(macro_simple2);
        TEST_CASE(macro_simple3);
        TEST_CASE(macro_simple4);
        TEST_CASE(macro_simple5);
        TEST_CASE(macro_simple6);
        TEST_CASE(macro_simple7);
        TEST_CASE(macro_simple8);
        TEST_CASE(macro_simple9);
        TEST_CASE(macro_simple10);
        TEST_CASE(macro_simple11);
        TEST_CASE(macro_simple12);
        TEST_CASE(macro_simple13);
        TEST_CASE(macro_simple14);
        TEST_CASE(macro_simple15);
        TEST_CASE(macro_simple16);  // #4703: Macro parameters not trimmed
        TEST_CASE(macro_simple17);  // #5074: isExpandedMacro not set
        TEST_CASE(macro_simple18);  // (1e-7)
        TEST_CASE(macroInMacro1);
        TEST_CASE(macroInMacro2);
        TEST_CASE(macro_mismatch);
        TEST_CASE(macro_nopar);
        TEST_CASE(macro_switchCase);
        TEST_CASE(macro_nullptr); // skip #define nullptr .. it is replaced in the tokenizer
        TEST_CASE(string1);
        TEST_CASE(string2);
        TEST_CASE(string3);
        TEST_CASE(preprocessor_undef);
        TEST_CASE(defdef);  // Defined multiple times
        TEST_CASE(preprocessor_doublesharp);
        TEST_CASE(va_args_1);
        TEST_CASE(va_args_2);
        TEST_CASE(va_args_3);
        TEST_CASE(va_args_4);
        TEST_CASE(multi_character_character);

        TEST_CASE(stringifyList);
        TEST_CASE(stringifyList2);
        TEST_CASE(stringifyList3);
        TEST_CASE(stringifyList4);
        TEST_CASE(stringifyList5);
        TEST_CASE(stringifyList6);
        TEST_CASE(ifdefwithfile);
        TEST_CASE(pragma_pack);
        TEST_CASE(pragma_once);
        TEST_CASE(pragma_asm_1);
        TEST_CASE(pragma_asm_2);
        TEST_CASE(asm_1);
        TEST_CASE(endifsemicolon);
        TEST_CASE(missing_doublequote);
        TEST_CASE(dup_defines);

        TEST_CASE(unicodeInCode);
        TEST_CASE(unicodeInComment);
        TEST_CASE(unicodeInString);
        TEST_CASE(define_part_of_func);
        TEST_CASE(conditionalDefine);
        TEST_CASE(multiline_comment);
        TEST_CASE(macro_parameters);
        TEST_CASE(newline_in_macro);
        TEST_CASE(ifdef_ifdefined);

        // define and then ifdef
        TEST_CASE(define_if1);
        TEST_CASE(define_if2);
        TEST_CASE(define_if3);
        TEST_CASE(define_if4); // #4079 - #define X +123
        TEST_CASE(define_if5); // #4516 - #define B (A & 0x00f0)
        TEST_CASE(define_ifdef);
        TEST_CASE(define_ifndef1);
        TEST_CASE(define_ifndef2);
        TEST_CASE(ifndef_define);
        TEST_CASE(undef_ifdef);

        TEST_CASE(redundant_config);

        TEST_CASE(invalid_define_1); // #2605 - hang for: '#define ='
        TEST_CASE(invalid_define_2); // #4036 - hang for: '#define () {(int f(x) }'

        // Test Preprocessor::simplifyCondition
        TEST_CASE(simplifyCondition);
        TEST_CASE(invalidElIf); // #2942 segfault

        TEST_CASE(def_valueWithParenthesis); // #3531

        // Using -D to predefine symbols
        TEST_CASE(predefine1);
        TEST_CASE(predefine2);
        TEST_CASE(predefine3);
        TEST_CASE(predefine4);
        TEST_CASE(predefine5);  // automatically define __cplusplus

        // Defines are given: test Preprocessor::handleIncludes
        TEST_CASE(def_handleIncludes);
        TEST_CASE(def_missingInclude);
        TEST_CASE(def_handleIncludes_ifelse1);   // problems in handleIncludes for #else

        TEST_CASE(def_valueWithParentheses); // #3531

        // Using -U to undefine symbols
        TEST_CASE(undef1);
        TEST_CASE(undef2);
        TEST_CASE(undef3);
        TEST_CASE(undef4);
        TEST_CASE(undef5);
        TEST_CASE(undef6);
        TEST_CASE(undef7);
        TEST_CASE(undef9);
        TEST_CASE(undef10);

        TEST_CASE(def_valueWithParenthesis); // #3531

        TEST_CASE(handleUndef);

        TEST_CASE(macroChar);

        TEST_CASE(validateCfg);

        TEST_CASE(if_sizeof);

        TEST_CASE(double_include); // #5717
        TEST_CASE(invalid_ifs)// #5909
    }


    void readCode1() {
        static const char code[] = " \t a //\n"
                                   "  #aa\t /* remove this */\tb  \r\n";
        Preprocessor2 preprocessor(0, this);
        std::istringstream istr(code);
        std::string codestr(preprocessor.readCode(istr));
        preprocessor.simplifyString(codestr, "test.c");
        ASSERT_EQUALS("a\n#aa   b\n", codestr);
    }

    void readCode2() {
        static const char code[] = "R\"( \" /* abc */ \n)\";";
        Preprocessor2 preprocessor(0, this);
        std::istringstream istr(code);
        std::string codestr(preprocessor.readCode(istr));
        preprocessor.simplifyString(codestr, "test.c");
        ASSERT_EQUALS("\" \\\" /* abc */ \\n\"\n;", codestr);
    }

    void readCode3() {
        const char code[] = "char c = '\\ ';";
        Settings settings;
        errout.str("");
        Preprocessor2 preprocessor(&settings, this);
        std::istringstream istr(code);
        ASSERT_EQUALS("char c = '\\ ';", preprocessor.readCode(istr));
        ASSERT_EQUALS("", errout.str());
    }


    void removeComments() {
        /*   Settings settings;
           Preprocessor preprocessor(&settings, this);

           // #3837 - asm comments
           const char code[] = "void test(void) {\n"
                               "   __asm\n"
                               "   {\n"
                               "      ;---- тест\n"
                               "   }\n"
                               "}\n";
           ASSERT_EQUALS(true, std::string::npos == preprocessor.removeComments(code, "3837.c").find("----"));

           ASSERT_EQUALS(" __asm123", preprocessor.removeComments(" __asm123", "3837.cpp"));
           ASSERT_EQUALS("\" __asm { ; } \"", preprocessor.removeComments("\" __asm { ; } \"", "3837.cpp"));
           ASSERT_EQUALS("__asm__ volatile { \"\" }", preprocessor.removeComments("__asm__ volatile { \"\" }", "3837.cpp"));*/
    }

    void utf16() {
        Settings settings;
        Preprocessor2 preprocessor(&settings, this);

        // a => a
        {
            const char code[] = { (char)0xff, (char)0xfe, 'a', '\0' };
            std::string s(code, sizeof(code));
            std::istringstream istr(s);
            ASSERT_EQUALS("a", preprocessor.readCode(istr));
        }

        {
            const char code[] = { (char)0xfe, (char)0xff, '\0', 'a' };
            std::string s(code, sizeof(code));
            std::istringstream istr(s);
            ASSERT_EQUALS("a", preprocessor.readCode(istr));
        }

        // extended char => 0xff
        {
            const char code[] = { (char)0xff, (char)0xfe, 'a', 'a' };
            std::string s(code, sizeof(code));
            std::istringstream istr(s);
            const char expected[] = { (char)0xff, 0 };
            ASSERT_EQUALS(expected, preprocessor.readCode(istr));
        }

        {
            const char code[] = { (char)0xfe, (char)0xff, 'a', 'a' };
            std::string s(code, sizeof(code));
            std::istringstream istr(s);
            const char expected[] = { (char)0xff, 0 };
            ASSERT_EQUALS(expected, preprocessor.readCode(istr));
        }

        // \r\n => \n
        {
            const char code[] = { (char)0xff, (char)0xfe, '\r', '\0', '\n', '\0' };
            std::string s(code, sizeof(code));
            std::istringstream istr(s);
            ASSERT_EQUALS("\n", preprocessor.readCode(istr));
        }

        {
            const char code[] = { (char)0xfe, (char)0xff, '\0', '\r', '\0', '\n' };
            std::string s(code, sizeof(code));
            std::istringstream istr(s);
            ASSERT_EQUALS("\n", preprocessor.readCode(istr));
        }
    }


    void newlines() {
        const char filedata[] = "\r\r\n\n";
        std::istringstream istr(filedata);
        ASSERT_EQUALS("\n\n\n", Preprocessor2::readCode(istr));
    }

    void simplifyCode1() {
        std::string filedata("/*\n*/ # /*\n*/ defi\\\nne FO\\\nO 10\\\n20");
        Preprocessor2 preprocessor(0, this);
        preprocessor.simplifyString(filedata, "test.cpp");
        ASSERT_EQUALS("# define FOO 1020", filedata);
    }

    void simplifyCode2() {
        std::string filedata("\"foo\\\\\nbar\"");
        Preprocessor2 preprocessor(0, this);
        preprocessor.simplifyString(filedata, "test.cpp");
        ASSERT_EQUALS("\"foo\\bar\"", filedata);
    }

    void simplifyCode3() {
        std::string filedata("#define A \" a  \"\n\" b\"");
        Preprocessor2 preprocessor(0, this);
        preprocessor.simplifyString(filedata, "test.cpp");
        ASSERT_EQUALS(filedata, filedata);
    }

    void simplifyCode4() {
        Preprocessor2 preprocessor(0, this);
        {
            // test < \\> < > (unescaped)
            std::string filedata("#define A \" \\\\\"/*space*/  \" \"");
            preprocessor.simplifyString(filedata, "test.cpp");
            ASSERT_EQUALS("#define A \" \\\\\" \" \"", filedata);
        }

        {
            // test <" \\\"  "> (unescaped)
            std::string filedata("#define A \" \\\\\\\"  \"");
            preprocessor.simplifyString(filedata, "test.cpp");
            ASSERT_EQUALS("#define A \" \\\\\\\"  \"", filedata);
        }

        {
            // test <" \\\\">  <" "> (unescaped)
            std::string filedata("#define A \" \\\\\\\\\"/*space*/  \" \"");
            preprocessor.simplifyString(filedata, "test.cpp");
            ASSERT_EQUALS("#define A \" \\\\\\\\\" \" \"", filedata);
        }
    }

    void diTrigraphs() {
        std::string digraphs = "<::><%%>%:";
        std::string trigraphs = "??=??/??'??(??)??!??<??>??-";
        Preprocessor2::replaceDiTrigraphs(digraphs);
        ASSERT_EQUALS("[]{}#", digraphs);
        Preprocessor2::replaceDiTrigraphs(trigraphs);
        ASSERT_EQUALS("#\\^[]|{}~", trigraphs);
    }

    void linkage() {
        std::string sourceFile = "a\n"
                                 "#if FOO\n"
                                 "#  if ABC\n"
                                 "      b1\n"
                                 "#  else\n"
                                 "      b2\n"
                                 "#  endif\n"
                                 "#elif BAR\n"
                                 "   c\n"
                                 "#else\n"
                                 "   d\n"
                                 "#endif\n"
                                 "e";

        Preprocessor2 preprocessor(0, this);
        preprocessor.simplifyString(sourceFile, "test.c");
        Tokenizer tokenizer;
        preprocessor.tokenize(sourceFile, tokenizer, "test.c", 0);

        const Token* t1 = tokenizer.tokens(); // a
        const Token* t2 = t1->next();         // # if FOO
        const Token* t3 = t2->tokAt(3);       // # if ABC
        const Token* t4 = t3->tokAt(4);       // # else
        const Token* t5 = t4->tokAt(3);       // # endif
        const Token* t6 = t5->tokAt(2);       // # elif BAR
        const Token* t7 = t6->tokAt(4);       // # else
        const Token* t8 = t7->tokAt(3);       // # endif

        // Outer if's
        ASSERT(t2->link() == t6);
        ASSERT(t6->link() == t7);
        ASSERT(t7->link() == t8);
        ASSERT(t8->link() == t7);

        // Inner if's
        ASSERT(t3->link() == t4);
        ASSERT(t4->link() == t5);
        ASSERT(t5->link() == t4);
    }

    void include1() {
        static const char includeFile[] = "a\n#if 0\nb\n#endif\nc";
        createIncludeFile("test.h", includeFile);
        std::string sourceFile = "1\n#include \"test.h\"\n2";

        Preprocessor2 preprocessor(0, this);
        preprocessor.simplifyString(sourceFile, "test.c");
        preprocessor.getConfigurations(sourceFile, "test.c");
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""]) {
            const Tokenizer& tokenizer = preprocessor.cfg[""]->tokenizer;
            ASSERT_EQUALS("1 a c 2", tokenizer.tokens()->stringifyList(0, false));
            ASSERT_EQUALS("[test.c:1]", tokenizer.list.fileLine(tokenizer.tokens()));
            ASSERT_EQUALS("[test.h:1]", tokenizer.list.fileLine(tokenizer.tokens()->next()));
            ASSERT_EQUALS("[test.h:5]", tokenizer.list.fileLine(tokenizer.tokens()->tokAt(2)));
            ASSERT_EQUALS("[test.c:3]", tokenizer.list.fileLine(tokenizer.tokens()->tokAt(3)));
        }
    }

    void include2() {
        static const char includeFile1[] = "b\n#include \"test2.h\"\nd";
        static const char includeFile2[] = "c";
        createIncludeFile("test1.h", includeFile1);
        createIncludeFile("test2.h", includeFile2);
        std::string sourceFile = "a\n#include \"test1.h\"\ne";

        Preprocessor2 preprocessor(0, this);
        preprocessor.simplifyString(sourceFile, "test.c");
        preprocessor.getConfigurations(sourceFile, "test.c");

        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""]) {
            const Tokenizer& tokenizer = preprocessor.cfg[""]->tokenizer;
            ASSERT_EQUALS("a b c d e", tokenizer.tokens()->stringifyList(0, false));
            ASSERT_EQUALS("[test.c:1]", tokenizer.list.fileLine(tokenizer.tokens()));
            ASSERT_EQUALS("[test1.h:1]", tokenizer.list.fileLine(tokenizer.tokens()->next()));
            ASSERT_EQUALS("[test2.h:1]", tokenizer.list.fileLine(tokenizer.tokens()->tokAt(2)));
            ASSERT_EQUALS("[test1.h:3]", tokenizer.list.fileLine(tokenizer.tokens()->tokAt(3)));
            ASSERT_EQUALS("[test.c:3]", tokenizer.list.fileLine(tokenizer.tokens()->tokAt(4)));
        }
    }

    void includeRec() {
        static const char includeFile[] = "#include \"test.h\"";
        createIncludeFile("test.h", includeFile);
        std::string sourceFile = "#include \"test.h\"";

        Preprocessor2 preprocessor(0, this);
        preprocessor.simplifyString(sourceFile, "test.c");
        preprocessor.getConfigurations(sourceFile, "test.c"); // Don't freeze
    }

    void missingInclude() {
        Settings settings;
        settings.addEnabled("information");
        settings.checkConfiguration = true;

        {
            errout.str("");
            Preprocessor2 preprocessor(&settings, this);
            Preprocessor2::missingIncludeFlag = false;

            static const char code[] = "#include \"missing.h\"\n";
            preprocess(preprocessor, code);

            ASSERT_EQUALS(true, Preprocessor2::missingIncludeFlag);
            ASSERT_EQUALS("[file.c:1]: (information) Include file: \"missing.h\" not found.\n", errout.str());
        } {
            settings.debugwarnings = true;
            errout.str("");
            Preprocessor2 preprocessor(&settings, this);
            Preprocessor2::missingIncludeFlag = false;

            static const char code[] = "#include <missing.h>\n";
            preprocess(preprocessor, code);

            ASSERT_EQUALS(true, Preprocessor2::missingIncludeFlag);
            ASSERT_EQUALS("[file.c:1]: (information) Include file: <missing.h> not found. Please note: Cppcheck does not need standard library headers to get proper results.\n", errout.str());
        }
    }

    void inline_suppression_for_missing_include() {
        errout.str("");
        Settings settings;
        settings._inlineSuppressions = true;
        settings.addEnabled("all");
        Preprocessor2 preprocessor(&settings, this);
        Preprocessor2::missingIncludeFlag = false;

        static const char code[] = "// cppcheck-suppress missingInclude\n"
                                   "#include \"missing.h\"\n"
                                   "int x;";
        preprocess(preprocessor, code);

        ASSERT_EQUALS(true, Preprocessor2::missingIncludeFlag); /// false?!
        ASSERT_EQUALS("", errout.str());
    }


    void Bug2190219() {
        const char filedata[] = "int main()\n"
                                "{\n"
                                "#ifdef __cplusplus\n"
                                "    int* flags = new int[10];\n"
                                "#else\n"
                                "    int* flags = (int*)malloc((10)*sizeof(int));\n"
                                "#endif\n"
                                "\n"
                                "#ifdef __cplusplus\n"
                                "    delete [] flags;\n"
                                "#else\n"
                                "    free(flags);\n"
                                "#endif\n"
                                "}\n";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // Compare results..
        ASSERT_EQUALS(2, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] && preprocessor.cfg["__cplusplus"]);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("int main ( ) { int * flags = ( int * ) malloc ( ( 10 ) * sizeof ( int ) ) ; free ( flags ) ; }",
                          preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["__cplusplus"])
            ASSERT_EQUALS("int main ( ) { int * flags = new int [ 10 ] ; delete [ ] flags ; }",
                          preprocessor.cfg["__cplusplus"]->tokenizer.tokens()->stringifyList(0, false));
    }


    void split1() {
        const char filedata[] = "#ifdef  WIN32 \n"
                                "    abcdef\n"
                                "#else  \n"
                                "    qwerty\n"
                                "#endif  \n";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // Compare results..
        ASSERT_EQUALS(2, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] && preprocessor.cfg["WIN32"]);
        if (preprocessor.cfg[""]) {
            ASSERT_EQUALS("qwerty", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
            ASSERT_EQUALS(4, preprocessor.cfg[""]->tokenizer.tokens()->linenr());
        }
        if (preprocessor.cfg["WIN32"]) {
            ASSERT_EQUALS("abcdef", preprocessor.cfg["WIN32"]->tokenizer.tokens()->stringifyList(0, false));
            ASSERT_EQUALS(2, preprocessor.cfg["WIN32"]->tokenizer.tokens()->linenr());
        }
    }

    void split2() {
        const char filedata[] = "#ifdef ABC\n"
                                "a\n"
                                "#ifdef DEF\n"
                                "b\n"
                                "#endif\n"
                                "c\n"
                                "#endif\n";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // Compare results..
        ASSERT_EQUALS(3, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] && preprocessor.cfg["ABC"] && preprocessor.cfg["ABC;DEF"]);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["ABC"])
            ASSERT_EQUALS("a c", preprocessor.cfg["ABC"]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["ABC;DEF"])
            ASSERT_EQUALS("a b c", preprocessor.cfg["ABC;DEF"]->tokenizer.tokens()->stringifyList(0, false));
    }

    void split3() {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#endif\t\n"
                                "#ifdef ABC\n"
                                "A\n"
                                "#endif\n";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // Compare results..
        ASSERT_EQUALS(2, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] && preprocessor.cfg["ABC"]);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["ABC"])
            ASSERT_EQUALS("A A", preprocessor.cfg["ABC"]->tokenizer.tokens()->stringifyList(0, false));
    }

    void split4() {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#else\n"
                                "B\n"
                                "#ifdef DEF\n"
                                "C\n"
                                "#endif\n"
                                "#endif\n";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // Compare results..
        ASSERT_EQUALS(3, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] && preprocessor.cfg["ABC"] && preprocessor.cfg["DEF"]);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("B", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["ABC"])
            ASSERT_EQUALS("A", preprocessor.cfg["ABC"]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["DEF"])
            ASSERT_EQUALS("B C", preprocessor.cfg["DEF"]->tokenizer.tokens()->stringifyList(0, false));
    }

    void split5a() {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#ifdef ABC\n"
                                "B\n"
                                "#endif\n"
                                "#endif\n";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // Make sure an error message is written..
        TODO_ASSERT_EQUALS("[file.c:3]: (error) ABC is already guaranteed to be defined\n",
                           "",
                           errout.str());

        // Compare results..
        ASSERT_EQUALS(2, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] && preprocessor.cfg["ABC"]);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["ABC"])
            ASSERT_EQUALS("A B", preprocessor.cfg["ABC"]->tokenizer.tokens()->stringifyList(0, false));
    }

    void split5b() {
        const char filedata[] = "#ifndef ABC\n"
                                "A\n"
                                "#ifndef ABC\n"
                                "B\n"
                                "#endif\n"
                                "#endif\n";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // Make sure an error message is written..
        TODO_ASSERT_EQUALS("[file.c:3]: (error) ABC is already guaranteed NOT to be defined\n",
                           "",
                           errout.str());

        // Compare results..
        ASSERT_EQUALS(2, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] && preprocessor.cfg["ABC"]);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("A B", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["ABC"])
            ASSERT_EQUALS("", preprocessor.cfg["ABC"]->tokenizer.tokens()->stringifyList(0, false));
    }

    void split5c() {
        const char filedata[] = "#ifndef ABC\n"
                                "A\n"
                                "#ifdef ABC\n"
                                "B\n"
                                "#endif\n"
                                "#endif\n";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // Make sure an error message is written..
        TODO_ASSERT_EQUALS("[file.c:3]: (error) ABC is already guaranteed NOT to be defined\n",
                           "",
                           errout.str());

        // Compare results..
        ASSERT_EQUALS(2, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] && preprocessor.cfg["ABC"]);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("A", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["ABC"])
            ASSERT_EQUALS("", preprocessor.cfg["ABC"]->tokenizer.tokens()->stringifyList(0, false));
    }

    void split5d() {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#ifndef ABC\n"
                                "B\n"
                                "#endif\n"
                                "#endif\n";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // Make sure an error message is written..
        TODO_ASSERT_EQUALS("[file.c:3]: (error) ABC is already guaranteed to be defined\n",
                           "",
                           errout.str());

        // Compare results..
        ASSERT_EQUALS(2, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] && preprocessor.cfg["ABC"]);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["ABC"])
            ASSERT_EQUALS("A", preprocessor.cfg["ABC"]->tokenizer.tokens()->stringifyList(0, false));
    }

    void split6() {
        const char include[] = "#ifndef test_h\n"
                               "#define test_h\n"
                               "#ifdef ABC\n"
                               "#endif\n"
                               "#endif";
        createIncludeFile("test.h", include);

        const char filedata[] = "#ifdef ABC\n"
                                "#include \"test.h\"\n"
                                "#endif";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // Make sure no error message is written..
        ASSERT_EQUALS("", errout.str());

        // Compare results..
        ASSERT_EQUALS(2, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] && preprocessor.cfg["ABC"]);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["ABC"])
            ASSERT_EQUALS("", preprocessor.cfg["ABC"]->tokenizer.tokens()->stringifyList(0, false));
    }

    void split7() {
        /// REPLACE
        /*const char filedata[] = "#if(A)\n"
                                "#if ( A ) \n"
                                "#if A\n"
                                "#if defined((A))\n"
                                "#elif defined (A)\n";

        std::istringstream istr(filedata);
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        const std::string actual(preprocessor.read(istr, "test.c"));

        // Compare results..
        ASSERT_EQUALS("#if A\n#if A\n#if A\n#if defined(A)\n#elif defined(A)\n", actual);*/
    }

    void test8() {
        const char filedata[] = "#if A == 1\n"
                                "1 A\n"
                                "#endif\n";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // No error..
        ASSERT_EQUALS("", errout.str());

        // Compare results..
        ASSERT_EQUALS(2, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] && preprocessor.cfg["A=1"]);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["A=1"])
            ASSERT_EQUALS("1 1", preprocessor.cfg["A=1"]->tokenizer.tokens()->stringifyList(0, false));
    }

    void test9() {
        const char filedata[] = "#if\n"
                                "#else\n"
                                "#endif\n";

        // Preprocess => actual result..
        Settings settings;
        settings._maxConfigs = 1;
        settings.userDefines.insert("X");
        Preprocessor2 preprocessor(&settings, this);
        preprocess(preprocessor, filedata); // <- don't crash
    }

    void test10() { // Ticket #5139
        const char filedata[] = "#define foo a.foo\n"
                                "#define bar foo\n"
                                "#define baz bar+0\n"
                                "#if 0\n"
                                "#endif";

        // Preprocess => actual result..
        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata); // <- don't crash
    }

    void error1() {
        const char filedata[] = "#ifdef A\n"
                                ";\n"
                                "#else\n"
                                "#error abcd\n"
                                "#endif\n";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // Compare results..
        ASSERT_EQUALS(1, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg["A"] != 0);
        if (preprocessor.cfg["A"])
            ASSERT_EQUALS(";", preprocessor.cfg["A"]->tokenizer.tokens()->stringifyList(0, false));
    }


    void error2() {
        /*errout.str("");

        const char filedata[] = "#error \xAB\n"
                                "#warning \xAB\n"
                                "123";

        // Read string..
        std::istringstream istr(filedata);
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        ASSERT_EQUALS("#error\n\n123", preprocessor.read(istr,"test.c"));*/
    }

    void error3() {
        /*errout.str("");
        Settings settings;
        settings.userDefines.insert("__cplusplus");
        Preprocessor preprocessor(&settings, this);
        const std::string code("#error hello world!\n");
        preprocessor.getcode(code, "X", "test.c");
        ASSERT_EQUALS("[test.c:1]: (error) #error hello world!\n", errout.str());*/
    }

    void error4() {
        errout.str("");
        Settings settings;
        settings.userDefines.insert("FOO");
        settings._force = true; // No message if --force is given
        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, "#error hello world!\n");
        ASSERT_EQUALS("", errout.str());
    }

    void if0_exclude() {
        Preprocessor2 preprocessor1(0, this);

        static const char code1[] = "#if 0\n"
                                    "A\n"
                                    "#endif\n"
                                    "B\n";

        preprocessor1.getConfigurations(code1, "test.c");

        ASSERT_EQUALS("B", preprocessor1.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        ASSERT_EQUALS(1, preprocessor1.cfg.size());


        Preprocessor2 preprocessor2(0, this);

        static const char code2[] = "#if (0)\n"
                                    "A\n"
                                    "#endif\n"
                                    "B\n";

        preprocessor2.getConfigurations(code2, "test.c");

        ASSERT_EQUALS("B", preprocessor1.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        ASSERT_EQUALS(1, preprocessor1.cfg.size());
    }

    void if0_else() {
        Preprocessor2 preprocessor1(0, this);

        static const char code1[] = "#if 0\n"
                                    "A\n"
                                    "#else\n"
                                    "B\n"
                                    "#endif\n"
                                    "C\n";

        preprocessor1.getConfigurations(code1, "test.c");

        ASSERT_EQUALS("B C", preprocessor1.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        ASSERT_EQUALS(1, preprocessor1.cfg.size());


        Preprocessor2 preprocessor2(0, this);

        static const char code2[] = "#if 1\n"
                                    "A\n"
                                    "#else\n"
                                    "B\n"
                                    "#endif\n"
                                    "C\n";

        preprocessor2.getConfigurations(code2, "test.c");

        ASSERT_EQUALS("A C", preprocessor2.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        ASSERT_EQUALS(1, preprocessor2.cfg.size());
    }

    void if0_elif() {
        Preprocessor2 preprocessor(0, this);

        static const char code[] = "#if 0\n"
                                   "A\n"
                                   "#elif 1\n"
                                   "B\n"
                                   "#endif\n"
                                   "C\n";

        preprocessor.getConfigurations(code, "test.c");

        ASSERT_EQUALS("B C", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        ASSERT_EQUALS(1, preprocessor.cfg.size());
    }

    void includeguard1() {
        // Handling include guards..
        const char include[] = "#ifndef abcH\n"
                               "#define abcH\n"
                               "bar\n"
                               "#endif";
        createIncludeFile("test.h", include);

        const char filedata[] = "foo\n"
                                "#include \"test.h\"\n"
                                "foo\n"
                                "#include \"test.h\"";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // Compare results..
        ASSERT_EQUALS(1, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("foo bar foo", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
    }

    void includeguard2() {
        // Handling include guards..
        const char include[] = "#pragma once\n"
                               "bar";
        createIncludeFile("test.h", include);

        const char filedata[] = "foo\n"
                                "#include \"test.h\"\n"
                                "foo\n"
                                "#include \"test.h\"";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // Compare results..
        ASSERT_EQUALS(1, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("foo bar foo", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
    }

    void includeguard3() {
        static const char code[] = "#ifndef X\n#define X\n1\n#endif\n"
                                   "#ifndef X\n#define X\n2\n#endif\n";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, code);

        ASSERT_EQUALS(1, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("1", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
    }

    void includeguard4() {
        static const char code[] = "\n#ifndef PAL_UTIL_UTILS_H_\n"
                                   "#define PAL_UTIL_UTILS_H_\n"
                                   "1\n"
                                   "#ifndef USE_BOOST\n"
                                   "2\n"
                                   "#else\n"
                                   "3\n"
                                   "#endif\n"
                                   "4\n"
                                   "#endif\n"
                                   "\n"
                                   "#ifndef PAL_UTIL_UTILS_H_\n"
                                   "#define PAL_UTIL_UTILS_H_\n"
                                   "5\n"
                                   "#ifndef USE_BOOST\n"
                                   "6\n"
                                   "#else\n"
                                   "7\n"
                                   "#endif\n"
                                   "8\n"
                                   "#endif";

        Preprocessor2 preprocessor(nullptr,this);
        preprocess(preprocessor, code);

        ASSERT_EQUALS(2, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] && preprocessor.cfg["USE_BOOST"]);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("1 2 4", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["USE_BOOST"])
            ASSERT_EQUALS("1 3 4", preprocessor.cfg["USE_BOOST"]->tokenizer.tokens()->stringifyList(0, false));
    }

    void includeguard5() { // #3651
        const char code[] = "#if defined(A)\n"
                            "a\n"
                            "#if defined(B)\n"
                            "a;b\n"
                            "#endif\n"
                            "#elif defined(C)\n"
                            "c\n"
                            "#else\n"
                            "\n"
                            "123\n"
                            "\n"
                            "#endif";

        Preprocessor2 preprocessor(nullptr, this);
        preprocess(preprocessor, code);

        ASSERT_EQUALS(4, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] && preprocessor.cfg["A"] && preprocessor.cfg["A;B"] && preprocessor.cfg["C"]);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("123", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["A"])
            ASSERT_EQUALS("a", preprocessor.cfg["A"]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["A;B"])
            ASSERT_EQUALS("a;b", preprocessor.cfg["A;B"]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["C"])
            ASSERT_EQUALS("c", preprocessor.cfg["C"]->tokenizer.tokens()->stringifyList(0, false));
    }

    void ifdefwithfile() {
        //// Handling include guards..
        //const char filedata[] = "#ifdef ABC\n"
        //                        "#file \"abc.h\"\n"
        //                        "class A{};/*\n\n\n\n\n\n\n*/\n"
        //                        "#endfile\n"
        //                        "#endif\n"
        //                        "int main() {}\n";

        //// Preprocess => actual result..
        //std::istringstream istr(filedata);
        //std::map<std::string, std::string> actual;
        //Settings settings;
        //Preprocessor preprocessor(&settings, this);
        //preprocessor.preprocess(istr, actual, "file.c");

        //// Expected configurations: "" and "ABC"
        //ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
        //ASSERT_EQUALS("\n#file \"abc.h\"\n\n\n\n\n\n\n\n\n#endfile\n\nint main() {}\n", actual[""]);
        //ASSERT_EQUALS("\n#file \"abc.h\"\nclass A{};\n\n\n\n\n\n\n\n#endfile\n\nint main() {}\n", actual["ABC"]);
    }



    void comments1() {
        {
            const char filedata[] = "/*\n"
                                    "#ifdef WIN32\n"
                                    "#endif\n"
                                    "*/\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            Preprocessor2 preprocessor(0, this);
            std::string code = preprocessor.readCode(istr);
            preprocessor.simplifyString(code, "test.c");

            // Compare results..
            ASSERT_EQUALS("\n\n\n\n", code);
        }

        {
            const char filedata[] = "/*\n"
                                    "\x080 #ifdef WIN32\n"
                                    "#endif\n"
                                    "*/\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            Preprocessor2 preprocessor(0, this);
            std::string code = preprocessor.readCode(istr);
            ///preprocessor.simplifyString(code, "test.c");

            // Compare results..
            ///ASSERT_EQUALS("\n\n\n\n", code);
        }

        {
            const char filedata[] = "void f()\n"
                                    "{\n"
                                    "  *p = a / *b / *c;\n"
                                    "}\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            Preprocessor2 preprocessor(0, this);
            std::string code = preprocessor.readCode(istr);
            preprocessor.simplifyString(code, "test.c");

            // Compare results..
            ASSERT_EQUALS("void f()\n{\n*p = a / *b / *c;\n}\n", code);
        }
    }



    void elif() {
        /*{
            const char filedata[] = "#if DEF1\n"
                                    "ABC\n"
                                    "#elif DEF2\n"
                                    "DEF\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS("\n\n\n\n\n", actual[""]);
            ASSERT_EQUALS("\nABC\n\n\n\n", actual["DEF1"]);
            ASSERT_EQUALS("\n\n\nDEF\n\n", actual["DEF2"]);
            ASSERT_EQUALS(3, static_cast<unsigned int>(actual.size()));
        }*/

        {
            const char filedata[] = "#if(defined DEF1)\n"
                                    "ABC\n"
                                    "#elif(defined DEF2)\n"
                                    "DEF\n"
                                    "#else\n"
                                    "GHI\n"
                                    "#endif\n";

            // Preprocess => actual result..
            Preprocessor2 preprocessor(0, this);
            preprocess(preprocessor, filedata);

            ASSERT_EQUALS(3, preprocessor.cfg.size());
            ASSERT(preprocessor.cfg[""] != 0 && preprocessor.cfg["DEF1"] != 0 && preprocessor.cfg["DEF2"] != 0);

            if (preprocessor.cfg[""] != 0)
                ASSERT_EQUALS("GHI", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
            if (preprocessor.cfg["DEF1"] != 0)
                ASSERT_EQUALS("ABC", preprocessor.cfg["DEF1"]->tokenizer.tokens()->stringifyList(0, false));
            if (preprocessor.cfg["DEF2"] != 0)
                ASSERT_EQUALS("DEF", preprocessor.cfg["DEF2"]->tokenizer.tokens()->stringifyList(0, false));
        }
    }


    void if_cond1() {
        /*const char filedata[] = "#if LIBVER>100\n"
                                "    A\n"
                                "#else\n"
                                "    B\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n\n\nB\n\n", actual[""]);
        TODO_ASSERT_EQUALS("\nA\n\n\n\n",
                           "", actual["LIBVER=101"]);*/
    }

    void if_cond2() {
        const char filedata[] = "#ifdef A\n"
                                "a\n"
                                "#endif\n"
                                "#if defined(A) && defined(B)\n"
                                "ab\n"
                                "#endif\n";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        ASSERT_EQUALS(3, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] != 0 && preprocessor.cfg["A"] != 0 && preprocessor.cfg["A;B"] != 0);

        if (preprocessor.cfg[""] != 0)
            ASSERT_EQUALS("", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["A"] != 0)
            ASSERT_EQUALS("a", preprocessor.cfg["A"]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["A;B"] != 0)
            ASSERT_EQUALS("a ab", preprocessor.cfg["A;B"]->tokenizer.tokens()->stringifyList(0, false));

        if_cond2b();
        if_cond2c();
        if_cond2d();
        if_cond2e();
    }

    void if_cond2b() {
        const char filedata[] = "#ifndef A\n"
                                "!a\n"
                                "#ifdef B\n"
                                "b\n"
                                "#endif\n"
                                "#else\n"
                                "a\n"
                                "#endif\n";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        ASSERT_EQUALS(3, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] != 0 && preprocessor.cfg["A"] != 0 && preprocessor.cfg["B"] != 0);

        if (preprocessor.cfg[""] != 0)
            ASSERT_EQUALS("! a", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["A"] != 0)
            ASSERT_EQUALS("a", preprocessor.cfg["A"]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["B"] != 0)
            ASSERT_EQUALS("! a b", preprocessor.cfg["B"]->tokenizer.tokens()->stringifyList(0, false));
    }

    void if_cond2c() {
        const char filedata[] = "#ifndef A\n"
                                "!a\n"
                                "#ifdef B\n"
                                "b\n"
                                "#else\n"
                                "!b\n"
                                "#endif\n"
                                "#else\n"
                                "a\n"
                                "#endif\n";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        ASSERT_EQUALS(3, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] != 0 && preprocessor.cfg["A"] != 0 && preprocessor.cfg["B"] != 0);

        if (preprocessor.cfg[""] != 0)
            ASSERT_EQUALS("! a ! b", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["A"] != 0)
            ASSERT_EQUALS("a", preprocessor.cfg["A"]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["B"] != 0)
            ASSERT_EQUALS("! a b", preprocessor.cfg["B"]->tokenizer.tokens()->stringifyList(0, false));
    }

    void if_cond2d() {
        const char filedata[] = "#ifndef A\n"
                                "!a\n"
                                "#ifdef B\n"
                                "b\n"
                                "#else\n"
                                "!b\n"
                                "#endif\n"
                                "#else\n"
                                "a\n"
                                "#ifdef B\n"
                                "b\n"
                                "#else\n"
                                "!b\n"
                                "#endif\n"
                                "#endif\n";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        ASSERT_EQUALS(4, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] != 0 && preprocessor.cfg["A"] != 0 && preprocessor.cfg["B"] != 0 && preprocessor.cfg["A;B"] != 0);

        if (preprocessor.cfg[""] != 0)
            ASSERT_EQUALS("! a ! b", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["A"] != 0)
            ASSERT_EQUALS("a ! b", preprocessor.cfg["A"]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["B"] != 0)
            ASSERT_EQUALS("! a b", preprocessor.cfg["B"]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["A;B"] != 0)
            ASSERT_EQUALS("a b", preprocessor.cfg["A;B"]->tokenizer.tokens()->stringifyList(0, false));
    }

    void if_cond2e() {
        const char filedata[] = "#if !defined(A)\n"
                                "!a\n"
                                "#elif !defined(B)\n"
                                "!b\n"
                                "#endif\n";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        ASSERT_EQUALS(3, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] != 0 && preprocessor.cfg["A"] != 0 && preprocessor.cfg["A;B"] != 0);

        if (preprocessor.cfg[""] != 0)
            ASSERT_EQUALS("! a", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["A"] != 0)
            ASSERT_EQUALS("! b", preprocessor.cfg["A"]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["A;B"] != 0)
            ASSERT_EQUALS("", preprocessor.cfg["A;B"]->tokenizer.tokens()->stringifyList(0, false));
    }

    void if_cond3() {
        const char filedata[] = "#ifdef A\n"
                                "a\n"
                                "#if defined(B) && defined(C)\n"
                                "abc\n"
                                "#endif\n"
                                "#endif\n";

        // Preprocess => actual result..
        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        TODO_ASSERT_EQUALS(3, 4, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] != 0 && preprocessor.cfg["A"] != 0 && preprocessor.cfg["A;B;C"] != 0);

        if (preprocessor.cfg[""] != 0)
            ASSERT_EQUALS("", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["A"] != 0)
            ASSERT_EQUALS("a", preprocessor.cfg["A"]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["A;B;C"] != 0)
            ASSERT_EQUALS("a abc", preprocessor.cfg["A;B;C"]->tokenizer.tokens()->stringifyList(0, false));
    }

    void if_cond4() {
        {
            const char filedata[] = "#define A\n"
                                    "#define B\n"
                                    "#if defined A || defined B\n"
                                    "ab\n"
                                    "#endif\n";

            // Preprocess => actual result..
            Preprocessor2 preprocessor(0, this);
            preprocess(preprocessor, filedata);

            ASSERT_EQUALS(1, preprocessor.cfg.size());
            ASSERT(preprocessor.cfg[""] != 0);

            if (preprocessor.cfg[""] != 0)
                ASSERT_EQUALS("ab", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        }

        /*{
            const char filedata[] = "#if A\n"
                                    "{\n"
                                    "#if (defined(B))\n"
                                    "foo();\n"
                                    "#endif\n"
                                    "}\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS(3, static_cast<unsigned int>(actual.size()));
            ASSERT_EQUALS("\n\n\n\n\n\n\n", actual[""]);
            ASSERT_EQUALS("\n{\n\n\n\n}\n\n", actual["A"]);
            ASSERT_EQUALS("\n{\n\nfoo();\n\n}\n\n", actual["A;B"]);
        }*/

        /*{
            const char filedata[] = "#if (A)\n"
                                    "foo();\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
            ASSERT_EQUALS("\n\n\n", actual[""]);
            ASSERT_EQUALS("\nfoo();\n\n", actual["A"]);
        }*/

        /*{
            const char filedata[] = "#if! A\n"
                                    "foo();\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            TODO_ASSERT_EQUALS(2, 1, static_cast<unsigned int>(actual.size()));
            ASSERT_EQUALS("\nfoo();\n\n", actual[""]);
        }*/
    }

    void if_cond5() {
        const char filedata[] = "#if defined(A) && defined(B)\n"
                                "ab\n"
                                "#endif\n"
                                "cd\n"
                                "#if defined(B) && defined(A)\n"
                                "ef\n"
                                "#endif\n";

        // Preprocess => actual result..
        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        TODO_ASSERT_EQUALS(2, 3, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] != 0 && preprocessor.cfg["A;B"] != 0);

        if (preprocessor.cfg[""] != 0)
            ASSERT_EQUALS("cd", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["A;B"] != 0)
            ASSERT_EQUALS("ab cd ef", preprocessor.cfg["A;B"]->tokenizer.tokens()->stringifyList(0, false));
    }

    void if_cond6() {
        /*const char filedata[] = "\n"
                                "#if defined(A) && defined(B))\n"
                                "#endif\n";

        errout.str("");

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("[file.c:2]: (error) mismatching number of '(' and ')' in this line: defined(A)&&defined(B))\n", errout.str());*/
    }

    void if_cond8() {
        /*const char filedata[] = "#if defined(A) + defined(B) + defined(C) != 1\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, (int)actual.size());
        ASSERT_EQUALS("\n\n", actual[""]);*/
    }


    void if_cond9() {
        const char filedata[] = "#if !defined _A\n"
                                "abc\n"
                                "#endif\n";

        // Preprocess => actual result..
        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        ASSERT_EQUALS(2, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] != 0 && preprocessor.cfg["_A"] != 0);

        if (preprocessor.cfg[""] != 0)
            ASSERT_EQUALS("abc", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["_A"] != 0)
            ASSERT_EQUALS("", preprocessor.cfg["_A"]->tokenizer.tokens()->stringifyList(0, false));
    }

    void if_cond10() {
        const char filedata[] = "#if !defined(a) && !defined(b)\n"
                                "#if defined(and)\n"
                                "#endif\n"
                                "#endif\n";

        // Preprocess => don't crash..
        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);
    }

    void if_cond11() {
        errout.str("");
        const char filedata[] = "#if defined(L_fixunssfdi) && LIBGCC2_HAS_SF_MODE\n"
                                "1\n"
                                "#if LIBGCC2_HAS_DF_MODE\n"
                                "2\n"
                                "#elif FLT_MANT_DIG < W_TYPE_SIZE\n"
                                "3\n"
                                "#endif\n"
                                "4\n"
                                "#endif\n";

        Preprocessor2 preprocessor;
        preprocess(preprocessor, filedata);

        ASSERT_EQUALS(2, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] && preprocessor.cfg["L_fixunssfdi;LIBGCC2_HAS_SF_MODE"]);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["L_fixunssfdi;LIBGCC2_HAS_SF_MODE"])
            ASSERT_EQUALS("124", preprocessor.cfg["L_fixunssfdi;LIBGCC2_HAS_SF_MODE"]->tokenizer.tokens()->stringifyList(0, false));
    }

    void if_cond12() {
        /*const char filedata[] = "#define A (1)\n"
                                "#if A == 1\n"
                                ";\n"
                                "#endif\n";
        Preprocessor preprocessor(nullptr, this);
        ASSERT_EQUALS("\n\n;\n\n", preprocessor.getcode(filedata,"",""));*/
    }

    void if_cond13() {
        /*const char filedata[] = "#if ('A' == 0x41)\n"
                                "123\n"
                                "#endif\n";
        Preprocessor preprocessor(nullptr, this);
        ASSERT_EQUALS("\n123\n\n", preprocessor.getcode(filedata,"",""));*/
    }

    void if_cond14() {
        /*const char filedata[] = "#if !(A)\n"
                                "123\n"
                                "#endif\n";
        Preprocessor preprocessor(nullptr, this);
        ASSERT_EQUALS("\n123\n\n", preprocessor.getcode(filedata,"",""));*/
    }

    void if_cond15() { // #4456 - segmentation fault
        /*const char filedata[] = "#if ((A >= B) && (C != D))\n"
                                "#if (E < F(1))\n"
                                "#endif\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "4456.c");  // <- don't crash in Preprocessor::getcfgs -> Tokenize -> number of template parameters*/
    }



    void if_or_1() {
        const char filedata[] = "#if defined(DEF_10) || defined(DEF_11)\n"
                                "a1;\n"
                                "#endif\n";

        Settings settings;
        {
            // Preprocess => actual result..
            Preprocessor2 preprocessor(&settings, this);
            preprocess(preprocessor, filedata);

            TODO_ASSERT_EQUALS(2, 3, preprocessor.cfg.size());
            ASSERT(preprocessor.cfg[""] != 0 && preprocessor.cfg["DEF_10"] != 0);

            if (preprocessor.cfg[""] != 0)
                ASSERT_EQUALS("", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
            if (preprocessor.cfg["DEF_10"] != 0)
                ASSERT_EQUALS("a1 ;", preprocessor.cfg["DEF_10"]->tokenizer.tokens()->stringifyList(0, false));
        }

        settings.userDefines.insert("DEF_11");
        {
            // Preprocess => actual result..
            Preprocessor2 preprocessor(&settings, this);
            preprocess(preprocessor, filedata);

            ASSERT_EQUALS(1, preprocessor.cfg.size());
            ASSERT(preprocessor.cfg[""] != 0);

            if (preprocessor.cfg[""] != 0)
                ASSERT_EQUALS("a1 ;", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        }
    }

    void if_or_2() {
        /*const std::string code("#if X || Y\n"
                               "a1;\n"
                               "#endif\n");
        Preprocessor preprocessor(nullptr, this);
        ASSERT_EQUALS("\na1;\n\n", preprocessor.getcode(code, "X", "test.c"));
        ASSERT_EQUALS("\na1;\n\n", preprocessor.getcode(code, "Y", "test.c"));*/
    }

    void if_macro_eq_macro() {
        /*static const char code[] = "#define A B\n"
                                   "#define B 1\n"
                                   "#define C 1\n"
                                   "#if A == C\n"
                                   "Wilma\n"
                                   "#else\n"
                                   "Betty\n"
                                   "#endif\n";
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        std::istringstream istr(code);
        std::map<std::string, std::string> actual;
        preprocessor.preprocess(istr, actual, "file.c");

        ASSERT_EQUALS("\n\n\n\nWilma\n\n\n\n", actual[""]);*/
    }

    void ticket_3675() {
        static const char code[] = "#ifdef YYSTACKSIZE\n"
                                   "#define YYMAXDEPTH YYSTACKSIZE\n"
                                   "#else\n"
                                   "#define YYSTACKSIZE YYMAXDEPTH\n"
                                   "#endif\n"
                                   "#if YYDEBUG\n"
                                   "#endif";

        Preprocessor2 preprocessor(0, this);
        ///preprocess(preprocessor, code); // TODO

        // There's nothing to assert. It just needs to not hang.
    }

    void ticket_3699() {
        static const char code[] = "#define INLINE __forceinline\n"
                                   "#define inline __forceinline\n"
                                   "#define __forceinline inline\n"
                                   "#if !defined(_WIN32)\n"
                                   "#endif\n"
                                   "INLINE inline __forceinline\n";

        // Preprocess => actual result..
        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, code);

        ASSERT_EQUALS(2, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] != 0 && preprocessor.cfg["_WIN32"] != 0);

        if (preprocessor.cfg[""] != 0)
            ASSERT_EQUALS("$__forceinline $inline $__forceinline", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, true));
    }

    void ticket_4922() {// #4922
        static const char code[] = "__asm__ \n"
                                   "{ int extern __value) 0; (double return (\"\" } extern\n"
                                   "__typeof __finite (__finite) __finite __inline \"__GI___finite\");";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, code); // <-- don't crash
    }

    void multiline1() {
        std::string filedata = "#define str \"abc\"     \\\n"
                               "            \"def\"       \n"
                               "abcdef = str;\n";

        Preprocessor2 preprocessor(nullptr, this);
        preprocessor.simplifyString(filedata, "test.c");
        ASSERT_EQUALS("#define str \"abc\" \"def\"\n\nabcdef = str;\n", filedata);
    }

    void multiline2() {
        std::string filedata = "#define sqr(aa) aa * \\\n"
                               "                aa\n"
                               "sqr(5);\n";

        Preprocessor2 preprocessor(nullptr, this);
        preprocessor.simplifyString(filedata, "test.c");
        ASSERT_EQUALS("#define sqr(aa) aa * aa\n\nsqr(5);\n", filedata);
    }

    void multiline3() {
        std::string filedata = "const char *str = \"abc\\\n"
                               "def\\\n"
                               "ghi\"\n";

        Preprocessor2 preprocessor(nullptr, this);
        preprocessor.simplifyString(filedata, "test.c");
        ASSERT_EQUALS("const char *str = \"abcdefghi\"\n", filedata);
    }

    void multiline4() {
        errout.str("");
        static const char code[] = "#define A int a = 4;\\ \n"
                                   " int b = 5;\n"
                                   "A\n";

        // Preprocess => actual result..
        Preprocessor2 preprocessor;
        preprocess(preprocessor, code);

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(preprocessor.cfg.size()));
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""]) {
#ifdef __GNUC__
            ASSERT_EQUALS(3, preprocessor.cfg[""]->tokenizer.tokens()->linenr());
            ASSERT_EQUALS(3, preprocessor.cfg[""]->tokenizer.tokens()->tokAt(5)->linenr());
            ASSERT_EQUALS("int a = 4 ; int b = 5 ;", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
#else
            ASSERT_EQUALS(2, preprocessor.cfg[""]->tokenizer.tokens()->linenr());
            ASSERT_EQUALS(3, preprocessor.cfg[""]->tokenizer.tokens()->tokAt(5)->linenr());
            ASSERT_EQUALS("int b = 5 ; int a = 4 ; \\", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
#endif
        }
        ASSERT_EQUALS("", errout.str());
    }

    void multiline5() {
        errout.str("");
        const char filedata[] = "#define ABC int a /*\n"
                                "*/= 4;\n"
                                "int main(){\n"
                                "ABC\n"
                                "}\n";

        // Preprocess => actual result..
        Preprocessor2 preprocessor;
        preprocess(preprocessor, filedata);

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(preprocessor.cfg.size()));
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""]) {
            ASSERT_EQUALS("int main ( ) { int a = 4 ; }", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
            ASSERT_EQUALS(5, preprocessor.cfg[""]->tokenizer.tokens()->tokAt(10)->linenr());
        }
        ASSERT_EQUALS("", errout.str());
    }

    void macro_simple1() {
        {
            const char filedata[] = "#define AAA(aa) f(aa)\n"
                                    "AAA(5);\n";
            ASSERT_EQUALS("f ( 5 ) ;", preprocEmptyCfg(filedata));
        }

        {
            const char filedata[] = "#define AAA(aa) f(aa)\n"
                                    "AAA (5);\n";
            ASSERT_EQUALS("f ( 5 ) ;", preprocEmptyCfg(filedata));
        }
    }

    void macro_simple2() {
        const char filedata[] = "#define min(x,y) x<y?x:y\n"
                                "min(a(),b());\n";
        ASSERT_EQUALS("a ( ) < b ( ) ? a ( ) : b ( ) ;", preprocEmptyCfg(filedata));
    }

    void macro_simple3() {
        const char filedata[] = "#define A 4\n"
                                "A AA\n";
        ASSERT_EQUALS("4 AA", preprocEmptyCfg(filedata));
    }

    void macro_simple4() {
        const char filedata[] = "#define A A\n"
                                "A";
        ASSERT_EQUALS("A", preprocEmptyCfg(filedata));
    }

    void macro_simple5() {
        const char filedata[] = "#define ABC if( temp > 0 ) return 1;\n"
                                "\n"
                                "void foo()\n"
                                "{\n"
                                "    int temp = 0;\n"
                                "    ABC\n"
                                "}\n";
        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""]) {
            ASSERT_EQUALS("\n\n##file 0\n"
                          "1:\n"
                          "2:\n"
                          "3: void foo ( )\n"
                          "4: {\n"
                          "5: int temp = 0 ;\n"
                          "6: $if $( $temp $> $0 $) $return $1 $;\n"
                          "7: }\n", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(false, true, true, true, true));
        }
    }

    void macro_simple6() {
        const char filedata[] = "#define ABC (a+b+c)\n"
                                "ABC\n";
        ASSERT_EQUALS("( a + b + c )", preprocEmptyCfg(filedata));
    }

    void macro_simple7() {
        const char filedata[] = "#define ABC(str) str\n"
                                "ABC(\"(\")\n";
        ASSERT_EQUALS("\"(\" ", preprocEmptyCfg(filedata));
    }

    void macro_simple8() {
        const char filedata[] = "#define ABC 123\n"
                                "#define ABCD 1234\n"
                                "ABC ABCD\n";
        ASSERT_EQUALS("123 1234", preprocEmptyCfg(filedata));
    }

    void macro_simple9() {
        const char filedata[] = "#define ABC(a) f(a)\n"
                                "ABC( \"\\\"\" );\n"
                                "ABC( \"g\" );\n";
        ASSERT_EQUALS("f ( \"\\\"\" ) ; f ( \"g\" ) ;", preprocEmptyCfg(filedata));
    }

    void macro_simple10() {
        const char filedata[] = "#define ABC(t) t x\n"
                                "ABC(unsigned long);\n";
        ASSERT_EQUALS("unsigned long x ;", preprocEmptyCfg(filedata));
    }

    void macro_simple11() {
        const char filedata[] = "#define ABC(x) delete x\n"
                                "ABC(a);\n";
        ASSERT_EQUALS("delete a ;", preprocEmptyCfg(filedata));
    }

    void macro_simple12() {
        const char filedata[] = "#define AB ab.AB\n"
                                "AB.CD\n";
        ASSERT_EQUALS("ab . AB . CD", preprocEmptyCfg(filedata));
    }

    void macro_simple13() {
        const char filedata[] = "#define TRACE(x)\n"
                                "TRACE(;if(a))\n";
        ASSERT_EQUALS("", preprocEmptyCfg(filedata));
    }

    void macro_simple14() {
        const char filedata[] = "#define A \" a \"\n"
                                "printf(A);\n";
        ASSERT_EQUALS("printf ( \" a \" ) ;", preprocEmptyCfg(filedata));
    }

    void macro_simple15() {
        const char filedata[] = "#define FOO\"foo\"\n"
                                "FOO\n";
        ASSERT_EQUALS("\"foo\"", preprocEmptyCfg(filedata));
    }

    void macro_simple16() {  // #4703
        const char filedata[] = "#define MACRO( A, B, C ) class A##B##C##Creator {};\n"
                                "MACRO( B\t, U , G )";
        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("$class $BUGCreator ${ $} $;", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(false, true, false, false, false));
    }

    void macro_simple17() {  // # 5074 - the Token::isExpandedMacro() doesn't always indicate properly if token comes from macro
        // It would probably be OK if the generated code was
        // "\n123+$123" since the first 123 comes from the source code
        const char filedata[] = "#define MACRO(A) A+123\n"
                                "MACRO(123)";
        ASSERT_EQUALS("123 + 123", preprocEmptyCfg(filedata));
    }

    void macro_simple18() {  // (1e-7)
        const char filedata1[] = "#define A (1e-7)\n"
                                 "a=A;";
        ASSERT_EQUALS("a = ( 1e-7 ) ;", preprocEmptyCfg(filedata1));

        const char filedata2[] = "#define A (1E-7)\n"
                                 "a=A;";
        ASSERT_EQUALS("a = ( 1E-7 ) ;", preprocEmptyCfg(filedata2));

        const char filedata3[] = "#define A (1e+7)\n"
                                 "a=A;";
        ASSERT_EQUALS("a = ( 1e+7 ) ;", preprocEmptyCfg(filedata3));

        const char filedata4[] = "#define A (1.e+7)\n"
                                 "a=A;";
        ASSERT_EQUALS("a = ( 1.e+7 ) ;", preprocEmptyCfg(filedata4));

        const char filedata5[] = "#define A (1.7f)\n"
                                 "a=A;";
        ASSERT_EQUALS("a = ( 1.7f ) ;", preprocEmptyCfg(filedata5));
    }

    void macroInMacro1() {
        {
            const char filedata[] = "#define A(m) long n = m; n++;\n"
                                    "#define B(n) A(n)\n"
                                    "B(0)\n";
            Preprocessor2 preprocessor(0, this);
            preprocess(preprocessor, filedata);
            ASSERT_EQUALS("$long $n $= $0 $; $n $++ $;", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(false, true, false, false, false));
        }

        {
            const char filedata[] = "#define A B\n"
                                    "#define B A\n"
                                    "A\n";
            ASSERT_EQUALS("A", preprocEmptyCfg(filedata));
        }

        {
            const char filedata[] = "#define A B\n"
                                    "#define B 3\n"
                                    "A\n";
            ASSERT_EQUALS("3", preprocEmptyCfg(filedata));
        }

        {
            const char filedata[] = "#define B 3\n"
                                    "#define A B\n"
                                    "A\n";
            ASSERT_EQUALS("3", preprocEmptyCfg(filedata));
        }

        /*{
            const char filedata[] = "#define DBG(fmt, args...) printf(fmt, ## args)\n"
                                    "#define D(fmt, args...) DBG(fmt, ## args)\n"
                                    "DBG(\"hello\");\n";
            ASSERT_EQUALS("\n\n$printf(\"hello\");\n", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define DBG(fmt, args...) printf(fmt, ## args)\n"
                                    "#define D(fmt, args...) DBG(fmt, ## args)\n"
                                    "DBG(\"hello: %d\",3);\n";
            ASSERT_EQUALS("\n\n$printf(\"hello: %d\",3);\n", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define BC(b, c...) 0##b * 0##c\n"
                                    "#define ABC(a, b...) a + BC(b)\n"
                                    "\n"
                                    "ABC(1);\n"
                                    "ABC(2,3);\n"
                                    "ABC(4,5,6);\n";

            ASSERT_EQUALS("\n\n\n$1+$0*0;\n$2+$03*0;\n$4+$05*06;\n", OurPreprocessor::expandMacros(filedata));
        }*/

        {
            const char filedata[] = "#define A 4\n"
                                    "#define B(a) a,A\n"
                                    "B(2);\n";
            ASSERT_EQUALS("2 , 4 ;", preprocEmptyCfg(filedata));
        }

        {
            const char filedata[] = "#define A(x) (x)\n"
                                    "#define B )A(\n"
                                    "#define C )A(\n";
            ASSERT_EQUALS("", preprocEmptyCfg(filedata));
        }

        {
            const char filedata[] = "#define A(x) (x*2)\n"
                                    "#define B A(\n"
                                    "foo B(i));\n";
            ASSERT_EQUALS("foo ( ( i ) * 2 ) ;", preprocEmptyCfg(filedata));
        }

        {
            const char filedata[] = "#define foo foo\n"
                                    "foo\n";
            ASSERT_EQUALS("foo", preprocEmptyCfg(filedata));
        }

        /*{
            const char filedata[] =
                "#define B(A1, A2) } while (0)\n"
                "#define A(name) void foo##name() { do { B(1, 2); }\n"
                "A(0)\n"
                "A(1)\n";
            ASSERT_EQUALS("\n\n$void foo0(){do{$}while(0);}\n$void foo1(){do{$}while(0);}\n", OurPreprocessor::expandMacros(filedata));
        }*/

        {
            const char filedata[] =
                "#define B(x) (\n"
                "#define A() B(xx)\n"
                "B(1) A() ) )\n";
            ASSERT_EQUALS("( ( ) )", preprocEmptyCfg(filedata));
        }

        {
            const char filedata[] =
                "#define PTR1 (\n"
                "#define PTR2 PTR1 PTR1\n"
                "int PTR2 PTR2 foo )))) = 0;\n";
            ASSERT_EQUALS("int ( ( ( ( foo ) ) ) ) = 0 ;", preprocEmptyCfg(filedata));
        }

        {
            const char filedata[] =
                "#define PTR1 (\n"
                "PTR1 PTR1\n";
            ASSERT_EQUALS("( (", preprocEmptyCfg(filedata));
        }
    }

    void macroInMacro2() {
        /*const char filedata[] = "#define A(x) a##x\n"
                                "#define B 0\n"
                                "A(B)\n";
        ASSERT_EQUALS("\n\n$aB\n", OurPreprocessor::expandMacros(filedata));*/
    }

    void macro_mismatch() {
        /*const char filedata[] = "#define AAA(aa,bb) f(aa)\n"
                                "AAA(5);\n";
        ASSERT_EQUALS("\nAAA(5);\n", OurPreprocessor::expandMacros(filedata));*/
    }

    void macro_nopar() {
        const char filedata[] = "#define AAA( ) { nullptr }\n"
                                "AAA()\n";
        ASSERT_EQUALS("{ nullptr }", preprocEmptyCfg(filedata));
    }

    void macro_switchCase() {
        {
            // Make sure "case 2" doesn't become "case2"
            const char filedata[] = "#define A( b ) "
                                    "switch( a ){ "
                                    "case 2: "
                                    " break; "
                                    "}\n"
                                    "A( 5 );\n";
            ASSERT_EQUALS("switch ( a ) { case 2 : break ; } ;", preprocEmptyCfg(filedata));
        }

        {
            // Make sure "2 BB" doesn't become "2BB"
            const char filedata[] = "#define A() AA : 2 BB\n"
                                    "A();\n";
            ASSERT_EQUALS("AA : 2 BB ;", preprocEmptyCfg(filedata));
        }

        {
            const char filedata[] = "#define A }\n"
                                    "#define B() A\n"
                                    "#define C( a ) B() break;\n"
                                    "{C( 2 );\n";
            ASSERT_EQUALS("{ } break ; ;", preprocEmptyCfg(filedata));
        }


        {
            const char filedata[] = "#define A }\n"
                                    "#define B() A\n"
                                    "#define C( a ) B() _break;\n"
                                    "{C( 2 );\n";
            ASSERT_EQUALS("{ } _break ; ;", preprocEmptyCfg(filedata));
        }


        {
            const char filedata[] = "#define A }\n"
                                    "#define B() A\n"
                                    "#define C( a ) B() 5;\n"
                                    "{C( 2 );\n";
            ASSERT_EQUALS("{ } 5 ; ;", preprocEmptyCfg(filedata));
        }
    }

    void macro_nullptr() {
        // Let the tokenizer handle nullptr.
        // See ticket #4482 - UB when passing nullptr to variadic function
        /*ASSERT_EQUALS("\n$0", OurPreprocessor::expandMacros("#define null 0\nnull"));
        ASSERT_EQUALS("\nnullptr", OurPreprocessor::expandMacros("#define nullptr 0\nnullptr"));*/
    }

    void string1() {
        const char filedata[] = "int main()"
                                "{"
                                "    const char *a = \"#define A\n\";"
                                "}\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // Compare results..
        ASSERT_EQUALS(1, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("int main ( ) { const char * a = \"#define A\n\" ; }", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
    }

    void string2() {
        const char filedata[] = "#define AAA 123\n"
                                "str = \"AAA\"\n";

        // Compare results..
        ASSERT_EQUALS("str = \"AAA\"", preprocEmptyCfg(filedata));
    }

    void string3() {
        const char filedata[] = "str(\";\");\n";

        // Compare results..
        ASSERT_EQUALS("str ( \";\" ) ;", preprocEmptyCfg(filedata));
    }


    void preprocessor_undef() {
        {
            const char filedata[] = "#define AAA int a;\n"
                                    "#undef AAA\n"
                                    "#define AAA char b=0;\n"
                                    "AAA\n";

            // Compare results..
            ASSERT_EQUALS("char b = 0 ;", preprocEmptyCfg(filedata));
        }

        {
            // ticket #403
            const char filedata[] = "#define z p[2]\n"
                                    "#undef z\n"
                                    "int z;\n"
                                    "z = 0;\n";

            ASSERT_EQUALS("int z ; z = 0 ;", preprocEmptyCfg(filedata));
        }
    }

    void defdef() {
        const char filedata[] = "#define AAA 123\n"
                                "#define AAA 456\n"
                                "#define AAA 789\n"
                                "AAA\n";

        // Compare results..
        ASSERT_EQUALS("789", preprocEmptyCfg(filedata));
    }

    void preprocessor_doublesharp() {
        // simple testcase without ##
        const char filedata1[] = "#define TEST(var,val) var = val\n"
                                 "TEST(foo,20);\n";
        ASSERT_EQUALS("foo = 20 ;", preprocEmptyCfg(filedata1));

        /*// simple testcase with ##
        const char filedata2[] = "#define TEST(var,val) var##_##val = val\n"
                                 "TEST(foo,20);\n";
        ASSERT_EQUALS("\n$foo_20=20;\n", OurPreprocessor::expandMacros(filedata2));

        // concat macroname
        const char filedata3[] = "#define ABCD 123\n"
                                 "#define A(B) A##B\n"
                                 "A(BCD)\n";
        ASSERT_EQUALS("\n\n$$123\n", OurPreprocessor::expandMacros(filedata3));

        // Ticket #1802 - inner ## must be expanded before outer macro
        const char filedata4[] = "#define A(B) A##B\n"
                                 "#define a(B) A(B)\n"
                                 "a(A(B))\n";
        ASSERT_EQUALS("\n\n$$AAB\n", OurPreprocessor::expandMacros(filedata4));

        // Ticket #1802 - inner ## must be expanded before outer macro
        const char filedata5[] = "#define AB(A,B) A##B\n"
                                 "#define ab(A,B) AB(A,B)\n"
                                 "ab(a,AB(b,c))\n";
        ASSERT_EQUALS("\n\n$$abc\n", OurPreprocessor::expandMacros(filedata5));

        // Ticket #1802
        const char filedata6[] = "#define AB_(A,B) A ## B\n"
                                 "#define AB(A,B) AB_(A,B)\n"
                                 "#define ab(suf) AB(X, AB_(_, suf))\n"
                                 "#define X x\n"
                                 "ab(y)\n";
        ASSERT_EQUALS("\n\n\n\n$$$x_y\n", OurPreprocessor::expandMacros(filedata6));*/
    }



    void va_args_1() {
        /*const char filedata[] = "#define DBG(fmt...) printf(fmt)\n"
                                "DBG(\"[0x%lx-0x%lx)\", pstart, pend);\n";

        // Preprocess..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\n$printf(\"[0x%lx-0x%lx)\",pstart,pend);\n", actual);*/
    }

    void va_args_2() {
        /*const char filedata[] = "#define DBG(fmt, args...) printf(fmt, ## args)\n"
                                "DBG(\"hello\");\n";

        // Preprocess..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\n$printf(\"hello\");\n", actual);*/
    }

    void va_args_3() {
        /*const char filedata[] = "#define FRED(...) { fred(__VA_ARGS__); }\n"
                                "FRED(123)\n";
        ASSERT_EQUALS("\n${ fred(123); }\n", OurPreprocessor::expandMacros(filedata));*/
    }

    void va_args_4() {
        /*const char filedata[] = "#define FRED(name, ...) name (__VA_ARGS__)\n"
                                "FRED(abc, 123)\n";
        ASSERT_EQUALS("\n$abc(123)\n", OurPreprocessor::expandMacros(filedata));*/
    }



    void multi_character_character() {
        const char filedata[] = "#define FOO 'ABCD'\n"
                                "int main()\n"
                                "{\n"
                                "  if( FOO == 0 );\n"
                                "  return 0;\n"
                                "}\n";

        // Preprocess => actual result..
        Preprocessor2 preprocessor;
        preprocess(preprocessor, filedata);

        // Compare results..
        ASSERT_EQUALS(1, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""]) {
            ASSERT_EQUALS("int main ( ) { if ( 'ABCD' == 0 ) ; return 0 ; }", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
            ASSERT(preprocessor.cfg[""]->tokenizer.tokens()->tokAt(7)->isExpandedMacro());
        }
    }


    void stringifyList() {
        /*const char filedata[] = "#define STRINGIFY(x) #x\n"
                                "STRINGIFY(abc)\n";

        // expand macros..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\n$\"abc\"\n", actual);*/
    }

    void stringifyList2() {
        /*const char filedata[] = "#define A(x) g(#x)\n"
                                "A(abc);\n";

        // expand macros..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\n$g(\"abc\");\n", actual);*/
    }

    void stringifyList3() {
        /*const char filedata[] = "#define A(x) g(#x)\n"
                                "A( abc);\n";

        // expand macros..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\n$g(\"abc\");\n", actual);*/
    }

    void stringifyList4() {
        /*const char filedata[] = "#define A(x) #x\n"
                                "1 A(\n"
                                "abc\n"
                                ") 2\n";

        // expand macros..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\n1 $\n\n\"abc\" 2\n", actual);*/
    }

    void stringifyList5() {
        /*const char filedata[] = "#define A(x) a(#x,x)\n"
                                "A(foo(\"\\\"\"))\n";
        ASSERT_EQUALS("\n$a(\"foo(\\\"\\\\\\\"\\\")\",foo(\"\\\"\"))\n", OurPreprocessor::expandMacros(filedata));*/
    }

    void stringifyList6() {
        /*const char filedata[] = "#define STRINGIFY(x) #x\n"
                                "#define FOO 123\n"
                                "STRINGIFY(FOO)\n";

        // expand macros..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\n\n$\"FOO\"\n", actual);*/
    }

    void pragma_pack() {
        const char filedata[] = "#pragma pack\n"
                                "void f()\n"
                                "{\n"
                                "}\n";


        Preprocessor2 preprocessor(0, this);
        preprocessor.getConfigurations(filedata, "test.c");

        // Compare results..
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("void f ( ) { }", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
    }

    void pragma_once() {
        const char code[] = "#pragma once\n"
                            "int x";

        Preprocessor2 preprocessor(nullptr, this);
        std::istringstream istr(code);
        std::string data = preprocessor.readCode(istr);
        preprocessor.simplifyString(data, "header.h");
        preprocessor.getConfigurations(data, "header.h");

        ASSERT_EQUALS(1U, preprocessor.cfg[""]->includedOnce.size());
        ASSERT_EQUALS("header.h", *(preprocessor.cfg[""]->includedOnce.begin()));
    }

    void pragma_asm_1() {
        static const char code[] = "#pragma asm\n"
                                   "    mov r1, 11\n"
                                   "#pragma endasm\n"
                                   "aaa\n"
                                   "#pragma asm foo\n"
                                   "    mov r1, 11\n"
                                   "#pragma endasm bar\n"
                                   "bbb";


        Preprocessor2 preprocessor(0, this);
        preprocessor.getConfigurations(code, "test.c");

        // Compare results..
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("asm ( mov r1 , 11 ) ; aaa asm ( foo mov r1 , 11 ) ; bar bbb", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
    }

    void pragma_asm_2() {
        /*const char filedata[] = "#pragma asm\n"
                                "    mov @w1, 11\n"
                                "#pragma endasm ( temp=@w1 )\n"
                                "bbb";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n\nasm(temp);\nbbb\n", actual[""]);*/
    }

    void asm_1() {
        static const char code[] = "#asm\n"
                                   "    mov ax,bx\n"
                                   "#endasm";

        Preprocessor2 preprocessor(0, this);
        preprocessor.getConfigurations(code, "test.c");

        // Compare results..
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("asm ( mov ax , bx ) ;", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
    }

    void endifsemicolon() {
        const char filedata[] = "void f() {\n"
                                "#ifdef A\n"
                                "#endif;\n"
                                "}\n";
        const char expected[] = "void f ( ) { }";

        // Preprocess => actual result..
        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // Compare results..
        ASSERT_EQUALS(2, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] && preprocessor.cfg["A"]);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS(expected, preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["A"])
            ASSERT_EQUALS(expected, preprocessor.cfg["A"]->tokenizer.tokens()->stringifyList(0, false));
    }

    void missing_doublequote() {
        /*{
            const char filedata[] = "#define a\n"
                                    "#ifdef 1\n"
                                    "\"\n"
                                    "#endif\n";

            // expand macros..
            errout.str("");
            const std::string actual(OurPreprocessor::expandMacros(filedata, this));

            ASSERT_EQUALS("", actual);
            ASSERT_EQUALS("[file.cpp:3]: (error) No pair for character (\"). Can't process file. File is either invalid or unicode, which is currently not supported.\n", errout.str());
        }

        {
            const char filedata[] = "#file \"abc.h\"\n"
                                    "#define a\n"
                                    "\"\n"
                                    "#endfile\n";

            // expand macros..
            errout.str("");
            const std::string actual(OurPreprocessor::expandMacros(filedata, this));

            ASSERT_EQUALS("", actual);
            ASSERT_EQUALS("[abc.h:2]: (error) No pair for character (\"). Can't process file. File is either invalid or unicode, which is currently not supported.\n", errout.str());
        }

        {
            const char filedata[] = "#file \"abc.h\"\n"
                                    "#define a\n"
                                    "#endfile\n"
                                    "\"\n";

            // expand macros..
            errout.str("");
            const std::string actual(OurPreprocessor::expandMacros(filedata, this));

            ASSERT_EQUALS("", actual);
            ASSERT_EQUALS("[file.cpp:2]: (error) No pair for character (\"). Can't process file. File is either invalid or unicode, which is currently not supported.\n", errout.str());
        }

        {
            const char filedata[] = "#define A 1\n"
                                    "#define B \"\n"
                                    "int a = A;\n";

            // expand macros..
            errout.str("");
            const std::string actual(OurPreprocessor::expandMacros(filedata, this));

            ASSERT_EQUALS("\n\nint a = $1;\n", actual);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char filedata[] = "void foo()\n"
                                    "{\n"
                                    "\n"
                                    "\n"
                                    "\n"
                                    "int a = 0;\n"
                                    "printf(Text\");\n"
                                    "}\n";

            // expand macros..
            errout.str("");
            const std::string actual(OurPreprocessor::expandMacros(filedata, this));

            ASSERT_EQUALS("[file.cpp:7]: (error) No pair for character (\"). Can't process file. File is either invalid or unicode, which is currently not supported.\n", errout.str());
        }*/
    }

    void unicodeInCode() {
        std::string filedata("a\xC8");
        std::istringstream istr(filedata);
        errout.str("");
        Preprocessor2 preprocessor(0, this);
        filedata = preprocessor.readCode(istr);
        preprocessor.simplifyString(filedata, "test.cpp");
        ASSERT_EQUALS("[test.cpp:1]: (error) The code contains characters that are unhandled. Neither unicode nor extended ASCII are supported. (line=1, character code=c8)\n", errout.str());
    }

    void unicodeInComment() {
        std::string filedata("//\xC8");
        std::istringstream istr(filedata);
        Preprocessor2 preprocessor(0, this);
        filedata = preprocessor.readCode(istr);
        preprocessor.simplifyString(filedata, "test.cpp");
        ASSERT_EQUALS("", filedata);
    }

    void unicodeInString() {
        std::string filedata("\"\xC8\"");
        std::istringstream istr(filedata);
        Preprocessor2 preprocessor(0, this);
        filedata = preprocessor.readCode(istr);
        preprocessor.simplifyString(filedata, "test.cpp");
        ASSERT_EQUALS("\"\xC8\"", filedata);
    }


    void define_part_of_func() {
        errout.str("");
        const char filedata[] = "#define A g(\n"
                                "void f() {\n"
                                "  A );\n"
                                "  }\n";

        // Preprocess => actual result..
        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(preprocessor.cfg.size()));
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""]) {
            ASSERT_EQUALS("void f ( ) { g ( ) ; }", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
            ASSERT(!preprocessor.cfg[""]->tokenizer.tokens()->tokAt(4)->isExpandedMacro());
            ASSERT(preprocessor.cfg[""]->tokenizer.tokens()->tokAt(5)->isExpandedMacro());
            ASSERT(preprocessor.cfg[""]->tokenizer.tokens()->tokAt(6)->isExpandedMacro());
            ASSERT(!preprocessor.cfg[""]->tokenizer.tokens()->tokAt(7)->isExpandedMacro());
        }
        ASSERT_EQUALS("", errout.str());
    }

    void conditionalDefine() {
        const char filedata[] = "#ifdef A\n"
                                "#define N 10\n"
                                "#else\n"
                                "#define N 20\n"
                                "#endif\n"
                                "N";

        // Preprocess => actual result..
        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // Compare results..
        ASSERT_EQUALS(2, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] && preprocessor.cfg["A"]);
        if (preprocessor.cfg[""]) {
            ASSERT_EQUALS("20", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
            ASSERT(preprocessor.cfg[""]->tokenizer.tokens()->isExpandedMacro());
        }
        if (preprocessor.cfg["A"]) {
            ASSERT_EQUALS("10", preprocessor.cfg["A"]->tokenizer.tokens()->stringifyList(0, false));
            ASSERT(preprocessor.cfg[""]->tokenizer.tokens()->isExpandedMacro());
        }
        ASSERT_EQUALS("", errout.str());
    }


    void multiline_comment() {
        errout.str("");
        const char filedata[] = "#define ABC {// \\\n"
                                "}\n"
                                "void f() ABC }\n";

        // Preprocess => actual result..
        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(preprocessor.cfg.size()));
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""]) {
            ASSERT_EQUALS("void f ( ) { }", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
            ASSERT(preprocessor.cfg[""]->tokenizer.tokens()->tokAt(4)->isExpandedMacro());
        }
        ASSERT_EQUALS("", errout.str());
    }

    void macro_parameters() {
        /*errout.str("");
        const char filedata[] = "#define BC(a, b, c, arg...) \\\n"
                                "AB(a, b, c, ## arg)\n"
                                "\n"
                                "void f()\n"
                                "{\n"
                                "  BC(3);\n"
                                "}\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c", std::list<std::string>());

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("", actual[""]);
        ASSERT_EQUALS("[file.c:6]: (error) Syntax error. Not enough parameters for macro 'BC'.\n", errout.str());*/
    }

    void newline_in_macro() {
        errout.str("");
        const char filedata[] = "#define ABC(str) printf( str )\n"
                                "void f()\n"
                                "{\n"
                                "  ABC(\"\\n\");\n"
                                "}\n";

        // Preprocess => actual result..
        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(preprocessor.cfg.size()));
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""]) {
            ASSERT_EQUALS("void f ( ) { printf ( \"\\n\" ) ; }", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        }
        ASSERT_EQUALS("", errout.str());
    }

    void ifdef_ifdefined() {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#endif\t\n"
                                "#if defined ABC\n"
                                "A\n"
                                "#endif\n";

        // Preprocess => actual result..
        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // Compare results..
        ASSERT_EQUALS(2, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] && preprocessor.cfg["ABC"]);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["ABC"])
            ASSERT_EQUALS("A A", preprocessor.cfg["ABC"]->tokenizer.tokens()->stringifyList(0, false));
    }

    void define_if1() {
        {
            const char filedata[] = "#define A 0\n"
                                    "#if A\n"
                                    "FOO\n"
                                    "#endif";

            Preprocessor2 preprocessor(0, this);
            preprocess(preprocessor, filedata);

            ASSERT_EQUALS(1, preprocessor.cfg.size());
            ASSERT(preprocessor.cfg[""] != 0);
            if (preprocessor.cfg[""])
                ASSERT_EQUALS("", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        }
        /*{
            Preprocessor preprocessor(nullptr, this);
            const char filedata[] = "#define A 1\n"
                                    "#if A==1\n"
                                    "FOO\n"
                                    "#endif";
            ASSERT_EQUALS("\n\nFOO\n\n", preprocessor.getcode(filedata,"",""));
        }*/
    }

    void define_if2() {
        /*const char filedata[] = "#define A 22\n"
                                "#define B A\n"
                                "#if (B==A) || (B==C)\n"
                                "FOO\n"
                                "#endif";
        Preprocessor preprocessor(nullptr, this);
        ASSERT_EQUALS("\n\n\nFOO\n\n", preprocessor.getcode(filedata,"",""));*/
    }

    void define_if3() {
        /*const char filedata[] = "#define A 0\n"
                                "#if (A==0)\n"
                                "FOO\n"
                                "#endif";
        Preprocessor preprocessor(nullptr, this);
        ASSERT_EQUALS("\n\nFOO\n\n", preprocessor.getcode(filedata,"",""));*/
    }

    void define_if4() {
        /*const char filedata[] = "#define X +123\n"
                                "#if X==123\n"
                                "FOO\n"
                                "#endif";
        Preprocessor preprocessor(nullptr, this);
        ASSERT_EQUALS("\n\nFOO\n\n", preprocessor.getcode(filedata,"",""));*/
    }

    void define_if5() { // #4516 - #define B (A & 0x00f0)
        /*{
            const char filedata[] = "#define A 0x0010\n"
                                    "#define B (A & 0x00f0)\n"
                                    "#if B==0x0010\n"
                                    "FOO\n"
                                    "#endif";
            Preprocessor preprocessor(nullptr, this);
            ASSERT_EQUALS("\n\n\nFOO\n\n", preprocessor.getcode(filedata,"",""));
        }
        {
            const char filedata[] = "#define A 0x00f0\n"
                                    "#define B (16)\n"
                                    "#define C (B & A)\n"
                                    "#if C==0x0010\n"
                                    "FOO\n"
                                    "#endif";
            Preprocessor preprocessor(nullptr, this);
            ASSERT_EQUALS("\n\n\n\nFOO\n\n", preprocessor.getcode(filedata,"",""));
        }
        {
            const char filedata[] = "#define A (1+A)\n" // don't hang for recursive macros
                                    "#if A==1\n"
                                    "FOO\n"
                                    "#endif";
            Preprocessor preprocessor(nullptr, this);
            ASSERT_EQUALS("\n\n\n\n", preprocessor.getcode(filedata,"",""));
        }*/
    }

    void define_ifdef() {
        {
            const char filedata[] = "#define ABC\n"
                                    "#ifndef ABC\n"
                                    "A\n"
                                    "#else\n"
                                    "B\n"
                                    "#endif\n";

            Preprocessor2 preprocessor(0, this);
            preprocess(preprocessor, filedata);

            // Compare results..
            ASSERT_EQUALS(1, preprocessor.cfg.size());
            ASSERT(preprocessor.cfg[""] != 0);
            if (preprocessor.cfg[""])
                ASSERT_EQUALS("B", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        }

        {
            const char filedata[] = "#define A 1\n"
                                    "#ifdef A\n"
                                    "A\n"
                                    "#endif\n";

            Preprocessor2 preprocessor(0, this);
            preprocess(preprocessor, filedata);

            // Compare results..
            ASSERT_EQUALS(1, preprocessor.cfg.size());
            ASSERT(preprocessor.cfg[""] != 0);
            if (preprocessor.cfg[""])
                ASSERT_EQUALS("1", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        }

        /*{
            const char filedata[] = "#define A 1\n"
                                    "#if A==1\n"
                                    "A\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS("\n\n$1\n\n", actual[""]);
            ASSERT_EQUALS(1, (int)actual.size());
        }*/

        /*{
            /// TODO: invalide
            const char filedata[] = "#define A 1\n"
                                    "#ifdef A>0\n"
                                    "A\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS("\n\n$1\n\n", actual[""]);
            ASSERT_EQUALS(1, (int)actual.size());
        }*/
        {
            const char filedata[] = "#define A 1\n"
                                    "#if 0\n"
                                    "#undef A\n"
                                    "#endif\n"
                                    "A\n";

            Preprocessor2 preprocessor(0, this);
            preprocess(preprocessor, filedata);

            // Compare results..
            ASSERT_EQUALS(1, preprocessor.cfg.size());
            ASSERT(preprocessor.cfg[""] != 0);
            if (preprocessor.cfg[""])
                ASSERT_EQUALS("1", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        }

        {
            const char filedata[] = "#define A 1\n"
                                    "#undef A\n"
                                    "#ifdef A\n"
                                    "A\n"
                                    "#else\n"
                                    "B\n"
                                    "#endif\n";

            Preprocessor2 preprocessor(0, this);
            preprocess(preprocessor, filedata);

            // Compare results..
            ASSERT_EQUALS(1, preprocessor.cfg.size());
            ASSERT(preprocessor.cfg[""] != 0);
            if (preprocessor.cfg[""])
                ASSERT_EQUALS("B", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        }
    }

    void define_ifndef1() {
        const char filedata[] = "#define A(x) (x)\n"
                                "#ifndef A\n"
                                ";\n"
                                "#endif\n";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // Compare results..
        ASSERT_EQUALS(1, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
    }

    void define_ifndef2() {
        const char filedata[] = "#ifdef A\n"
                                "#define B char\n"
                                "#endif\n"
                                "#ifndef B\n"
                                "#define B int\n"
                                "#endif\n"
                                "B me;\n";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        ASSERT_EQUALS(3, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] && preprocessor.cfg["A"] && preprocessor.cfg["B"]);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("int me ;", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["A"])
            ASSERT_EQUALS("char me ;", preprocessor.cfg["A"]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["B"])
            ASSERT_EQUALS("B me ;", preprocessor.cfg["B"]->tokenizer.tokens()->stringifyList(0, false)); // TODO: Should this cfg exist?
    }

    void ifndef_define() {
        /*const char filedata[] = "#ifndef A\n"
                                "#define A(x) x\n"
                                "#endif\n"
                                "A(123);";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        ASSERT_EQUALS(1U, actual.size());
        ASSERT_EQUALS("\n\n\n$123;\n", actual[""]);*/
    }

    void undef_ifdef() {
        const char filedata[] = "#undef A\n"
                                "#ifdef A\n"
                                "123\n"
                                "#endif\n";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        ASSERT_EQUALS(1, static_cast<unsigned int>(preprocessor.cfg.size()));
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
    }

    void redundant_config() {
        const char filedata[] = "int main() {\n"
                                "#ifdef FOO\n"
                                "#ifdef BAR\n"
                                "    std::cout << 1;\n"
                                "#endif\n"
                                "#endif\n"
                                "\n"
                                "#ifdef BAR\n"
                                "#ifdef FOO\n"
                                "    std::cout << 2;\n"
                                "#endif\n"
                                "#endif\n"
                                "}";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        // Compare results..
        ASSERT_EQUALS(4, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] != 0);
        ASSERT(preprocessor.cfg["BAR"] != 0);
        ASSERT(preprocessor.cfg["FOO"] != 0);
        ASSERT(preprocessor.cfg["BAR;FOO"] != 0);
    }

    void dup_defines() {
        const char filedata[] = "#ifdef A\n"
                                "#define B\n"
                                "#if defined(B) && defined(A)\n"
                                "a\n"
                                "#else\n"
                                "b\n"
                                "#endif\n"
                                "#endif\n";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        ASSERT_EQUALS(2, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] && preprocessor.cfg["A"]);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        if (preprocessor.cfg["A"])
            ASSERT_EQUALS("a", preprocessor.cfg["A"]->tokenizer.tokens()->stringifyList(0, false));
    }

    void invalid_define_1() {
        static const char code[] = "#define =\n";
        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, code); // don't hang
    }

    void invalid_define_2() {  // #4036 - hang
        static const char code[] = "#define () {(int f(x) }\n";
        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, code); // don't hang
    }

    void simplifyCondition() {
        // Ticket #2794
        static const char code[] = "#define C\n"
                                   "#if defined(A) || defined(B) || defined(C)\n"
                                   "a\n"
                                   "#else\n"
                                   "b\n"
                                   "#endif";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, code);

        ASSERT_EQUALS(1, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("a", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
    }

    void invalidElIf() {
        // #2942 - segfault
        static const char code[] = "#elif (){\n";
        Preprocessor2 preprocessor(nullptr,this);
        preprocess(preprocessor, code);
    }

    void def_valueWithParenthesis() {
        // #define should introduce a new symbol regardless of parenthesis in the value
        static const char code[] = "#define A (Fred)";
        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, code);

        ASSERT(preprocessor.cfg[""]->defs.find("A") != preprocessor.cfg[""]->defs.end());
        ASSERT_EQUALS(" ( Fred )", preprocessor.cfg[""]->defs["A"]); // The space in front of ( indicates, that this is _not_ a function macro
    }

    void predefine1() {
        static const char code[] = "#if defined(X) || defined(Y)\n"
                                   "Fred & Wilma\n"
                                   "#endif\n";
        Settings settings;
        settings.userDefines.insert("X=1");

        Preprocessor2 preprocessor(&settings, this);
        preprocess(preprocessor, code);

        ASSERT_EQUALS(1, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("Fred & Wilma", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
    }

    void predefine2() {
        static const char* src = "#if defined(X) && defined(Y)\n"
                                 "Fred & Wilma\n"
                                 "#endif";
        Settings settings;
        settings.userDefines.insert("X=1");
        {
            Preprocessor2 preprocessor(&settings, this);
            preprocess(preprocessor, src);
            ASSERT_EQUALS(2, preprocessor.cfg.size());
            ASSERT(preprocessor.cfg[""] != 0 && preprocessor.cfg["Y"] != 0);
            if (preprocessor.cfg[""])
                ASSERT_EQUALS("", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
            if (preprocessor.cfg["Y"])
                ASSERT_EQUALS("Fred & Wilma", preprocessor.cfg["Y"]->tokenizer.tokens()->stringifyList(0, false));
        }

        settings.userDefines.insert("Y=2");
        {
            Preprocessor2 preprocessor(&settings, this);
            preprocess(preprocessor, src);
            ASSERT_EQUALS(1, preprocessor.cfg.size());
            ASSERT(preprocessor.cfg[""] != 0);
            if (preprocessor.cfg[""])
                ASSERT_EQUALS("Fred & Wilma", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
        }
    }

    void predefine3() {
        /*// #2871 - define in source is not used if -D is used
        const char code[] = "#define X 1\n"
                            "#define Y X\n"
                            "#if (X == Y)\n"
                            "Fred & Wilma\n"
                            "#endif\n";
        Preprocessor preprocessor(nullptr,this);
        const std::string actual = preprocessor.getcode(code, "TEST", "test.c");
        ASSERT_EQUALS("\n\n\nFred & Wilma\n\n", actual);*/
    }

    void predefine4() {
        // #3577
        static const char code[] = "char buf[X];\n";
        Settings settings;
        settings.userDefines.insert("X=123");
        Preprocessor2 preprocessor(&settings, this);
        preprocess(preprocessor, code);
        ASSERT_EQUALS(1, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("char buf [ $123 ] ;", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, true));
    }

    void predefine5() {  // #3737 - automatically define __cplusplus
        const char code[] = "#ifdef __cplusplus\n"
                            "__cplusplus\n"
                            "#endif\n"
                            "#ifdef __STDC__\n"
                            "__STDC__ __STDC_VERSION__\n"
                            "#endif";
        Settings settings;
        // C89
        settings.standards.c = Standards::C89;
        ASSERT_EQUALS("1 199409", preprocEmptyCfg(code, false, &settings));
        // C99
        settings.standards.c = Standards::C99;
        ASSERT_EQUALS("1 199901", preprocEmptyCfg(code, false, &settings));
        // C11
        settings.standards.c = Standards::C11;
        ASSERT_EQUALS("1 201112", preprocEmptyCfg(code, false, &settings));
        // C++89
        settings.standards.cpp = Standards::CPP03;
        ASSERT_EQUALS("199711", preprocEmptyCfg(code, true, &settings));
        // C++11
        settings.standards.cpp = Standards::CPP11;
        ASSERT_EQUALS("201103", preprocEmptyCfg(code, true, &settings));
    }


    void def_handleIncludes() {
        /*const std::string filePath("test.c");
        const std::list<std::string> includePaths;
        std::map<std::string,std::string> defs;
        Preprocessor preprocessor(nullptr, this);

        // ifdef
        {
            defs.clear();
            defs["A"] = "";
            {
                const std::string code("#ifdef A\n123\n#endif\n");
                const std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs));
                ASSERT_EQUALS("\n123\n\n", actual);
            }{
                const std::string code("#ifdef B\n123\n#endif\n");
                const std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs));
                ASSERT_EQUALS("\n\n\n", actual);
            }
        }

        // ifndef
        {
            defs.clear();
            defs["A"] = "";
            {
                const std::string code("#ifndef A\n123\n#endif\n");
                const std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs));
                ASSERT_EQUALS("\n\n\n", actual);
            }{
                const std::string code("#ifndef B\n123\n#endif\n");
                const std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs));
                ASSERT_EQUALS("\n123\n\n", actual);
            }
        }

        // define - ifndef
        {
            defs.clear();
            const std::string code("#ifndef X\n#define X\n123\n#endif\n"
                                   "#ifndef X\n#define X\n123\n#endif\n");
            const std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs));
            ASSERT_EQUALS("\n#define X\n123\n\n" "\n\n\n\n", actual);
        }

        // #define => #if
        {
            defs.clear();
            const std::string code("#define X 123\n"
                                   "#if X==123\n"
                                   "456\n"
                                   "#endif\n");
            const std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs));
            ASSERT_EQUALS("#define X 123\n\n456\n\n", actual);
        }

        // #elif
        {
            const std::string code("#if defined(A)\n"
                                   "1\n"
                                   "#elif defined(B)\n"
                                   "2\n"
                                   "#elif defined(C)\n"
                                   "3\n"
                                   "#else\n"
                                   "4\n"
                                   "#endif");
            {
                defs.clear();
                defs["A"] = "";
                defs["C"] = "";
                const std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs));
                ASSERT_EQUALS("\n1\n\n\n\n\n\n\n\n", actual);
            }

            {
                defs.clear();
                defs["B"] = "";
                const std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs));
                ASSERT_EQUALS("\n\n\n2\n\n\n\n\n\n", actual);
            }

            {
                defs.clear();
                const std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs));
                ASSERT_EQUALS("\n\n\n\n\n\n\n4\n\n", actual);
            }
        }

        // #endif
        {
            // see also endifsemicolon
            const std::string code("{\n#ifdef X\n#endif;\n}");
            defs.clear();
            defs["Z"] = "";
            const std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs));
            ASSERT_EQUALS("{\n\n\n}\n", actual);
        }

        // #undef
        {
            const std::string code("#ifndef X\n"
                                   "#define X\n"
                                   "123\n"
                                   "#endif\n");

            defs.clear();
            const std::string actual1(preprocessor.handleIncludes(code,filePath,includePaths,defs));

            defs.clear();
            const std::string actual(preprocessor.handleIncludes(code + "#undef X\n" + code, filePath, includePaths, defs));

            ASSERT_EQUALS(actual1 + "#undef X\n" + actual1, actual);
        }

        // #error
        {
            errout.str("");
            defs.clear();
            const std::string code("#ifndef X\n#error abc\n#endif");
            const std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs));
            ASSERT_EQUALS("\n#error abc\n\n", actual);
            ASSERT_EQUALS("[test.c:2]: (error) abc\n", errout.str());
        }*/
    }

    void def_missingInclude() {
        /*const std::list<std::string> includePaths;
        std::map<std::string,std::string> defs;
        defs["AA"] = "";
        Settings settings;
        Preprocessor preprocessor(&settings,this);

        // missing local include
        {
            const std::string code("#include \"missing-include!!.h\"\n");

            errout.str("");
            settings = Settings();
            preprocessor.handleIncludes(code,"test.c",includePaths,defs);
            ASSERT_EQUALS("", errout.str());

            errout.str("");
            settings.checkConfiguration = true;
            preprocessor.handleIncludes(code,"test.c",includePaths,defs);
            ASSERT_EQUALS("[test.c:1]: (information) Include file: \"missing-include!!.h\" not found.\n", errout.str());

            errout.str("");
            settings.nomsg.addSuppression("missingInclude");
            preprocessor.handleIncludes(code,"test.c",includePaths,defs);
            ASSERT_EQUALS("", errout.str());
        }

        // missing system header
        {
            const std::string code("#include <missing-include!!.h>\n");

            errout.str("");
            settings = Settings();
            preprocessor.handleIncludes(code,"test.c",includePaths,defs);
            ASSERT_EQUALS("", errout.str());

            errout.str("");
            settings.checkConfiguration = true;
            preprocessor.handleIncludes(code,"test.c",includePaths,defs);
            ASSERT_EQUALS("[test.c:1]: (information) Include file: <missing-include!!.h> not found.\n", errout.str());

            errout.str("");
            settings.nomsg.addSuppression("missingIncludeSystem");
            preprocessor.handleIncludes(code,"test.c",includePaths,defs);
            ASSERT_EQUALS("", errout.str());

            pragmaOnce.clear();
            errout.str("");
            settings = Settings();
            settings.nomsg.addSuppression("missingInclude");
            preprocessor.handleIncludes(code,"test.c",includePaths,defs,pragmaOnce,std::list<std::string>());
            ASSERT_EQUALS("", errout.str());
        }

        // #3285 - #elif
        {
            const std::string code("#ifdef GNU\n"
                                   "#elif defined(WIN32)\n"
                                   "#include \"missing-include!!.h\"\n"
                                   "#endif");
            defs.clear();
            defs["GNU"] = "";

            errout.str("");
            settings = Settings();
            preprocessor.handleIncludes(code,"test.c",includePaths,defs);
            ASSERT_EQUALS("", errout.str());
        }*/
    }

    void def_handleIncludes_ifelse1() {
        /*const std::string filePath("test.c");
        const std::list<std::string> includePaths;
        std::map<std::string,std::string> defs;
        Preprocessor preprocessor(nullptr, this);

        // #3405
        {
            defs.clear();
            defs["A"] = "";
            const std::string code("\n#ifndef PAL_UTIL_UTILS_H_\n"
                                   "#define PAL_UTIL_UTILS_H_\n"
                                   "1\n"
                                   "#ifndef USE_BOOST\n"
                                   "2\n"
                                   "#else\n"
                                   "3\n"
                                   "#endif\n"
                                   "4\n"
                                   "#endif\n"
                                   "\n"
                                   "#ifndef PAL_UTIL_UTILS_H_\n"
                                   "#define PAL_UTIL_UTILS_H_\n"
                                   "5\n"
                                   "#ifndef USE_BOOST\n"
                                   "6\n"
                                   "#else\n"
                                   "7\n"
                                   "#endif\n"
                                   "8\n"
                                   "#endif\n"
                                   "\n");
            std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs));

            // the 1,2,4 should be in the result
            actual.erase(0, actual.find("1"));
            while (actual.find("\n") != std::string::npos)
                actual.erase(actual.find("\n"),1);
            ASSERT_EQUALS("124", actual);
        }

        // #3418
        {
            defs.clear();
            const char code[] = "#define A 1\n"
                                "#define B A\n"
                                "#if A == B\n"
                                "123\n"
                                "#endif\n";

            std::string actual(preprocessor.handleIncludes(code, filePath, includePaths, defs));
            ASSERT_EQUALS("#define A 1\n#define B A\n\n123\n\n", actual);
        }*/
    }

    void def_valueWithParentheses() {
        /*// #define should introduce a new symbol regardless of parentheses in the value
        // and regardless of white space in weird places (people do this for some reason).
        const char code[] = "#define A (Fred)\n"
                            "      #       define B (Flintstone)\n"
                            "     #define C (Barney)\n"
                            "\t#\tdefine\tD\t(Rubble)\t\t\t\n";

        const std::string filePath("test.c");
        const std::list<std::string> includePaths;
        std::map<std::string,std::string> defs;
        Preprocessor preprocessor(nullptr, this);

        std::istringstream istr(code);
        const std::string s(preprocessor.read(istr, ""));
        preprocessor.handleIncludes(s, filePath, includePaths, defs);

        ASSERT(defs.find("A") != defs.end());
        ASSERT_EQUALS("(Fred)", defs["A"]);

        ASSERT(defs.find("B") != defs.end());
        ASSERT_EQUALS("(Flintstone)", defs["B"]);

        ASSERT(defs.find("C") != defs.end());
        ASSERT_EQUALS("(Barney)", defs["C"]);

        ASSERT(defs.find("D") != defs.end());
        ASSERT_EQUALS("(Rubble)", defs["D"]);*/
    }

    void undef1() {
        const char filedata[] = "#ifdef X\n"
                                "Fred & Wilma\n"
                                "#endif\n";
        Settings settings;
        settings.userUndefs.insert("X");

        Preprocessor2 preprocessor(&settings, this);
        preprocess(preprocessor, filedata);

        ASSERT_EQUALS(1, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
    }

    void undef2() {
        const char filedata[] = "#ifndef X\n"
                                "Fred & Wilma\n"
                                "#endif\n";
        Settings settings;
        settings.userUndefs.insert("X");

        Preprocessor2 preprocessor(&settings, this);
        preprocess(preprocessor, filedata);

        ASSERT_EQUALS(1, preprocessor.cfg.size());
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("Fred & Wilma", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
    }

    void undef3() {
        /*Settings settings;

        const char filedata[] = "#define X\n"
                                "#ifdef X\n"
                                "Fred & Wilma\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        settings.userUndefs.insert("X"); // User undefs should override internal defines

        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1U, actual.size());
        ASSERT_EQUALS("\n\n\n\n", actual[""]);*/
    }

    void undef4() {
        /*Settings settings;

        const char filedata[] = "#define X() Y\n"
                                "#ifdef X\n"
                                "Fred & Wilma\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        settings.userUndefs.insert("X"); // User undefs should override internal defines

        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1U, actual.size());
        ASSERT_EQUALS("\n\n\n\n", actual[""]);*/
    }

    void undef5() {
        /*Settings settings;

        const char filedata[] = "#define X Y\n"
                                "#ifdef X\n"
                                "Fred & Wilma\n"
                                "#else\n"
                                "Barney & Betty\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        settings.userUndefs.insert("X"); // User undefs should override internal defines

        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1U, actual.size());
        ASSERT_EQUALS("\n\n\n\nBarney & Betty\n\n", actual[""]);*/
    }

    void undef6() {
        /*Settings settings;

        const char filedata[] = "#define X XDefined\n"
                                "X;\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        settings.userUndefs.insert("X"); // User undefs should override internal defines

        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1U, actual.size());
        TODO_ASSERT_EQUALS("\n;\n","\n$XDefined;\n", actual[""]);*/
    }

    void undef7() {
        const char filedata[] = "#ifdef HAVE_CONFIG_H\n"
                                "#include \"config.h\"\n"
                                "#endif\n"
                                "\n"
                                "void foo();\n";

        Settings settings;
        settings.addEnabled("information");
        settings.userUndefs.insert("X"); // User undefs should override internal defines
        settings.checkConfiguration = true;
        errout.str("");

        Preprocessor2 preprocessor(&settings, this);
        preprocess(preprocessor, filedata);

        ASSERT_EQUALS("[file.c:2]: (information) Include file: \"config.h\" not found.\n", errout.str());
        ASSERT(preprocessor.cfg[""] != 0);
        if (preprocessor.cfg[""])
            ASSERT_EQUALS("void foo ( ) ;", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(0, false));
    }

    void undef9() {
        /*Settings settings;

        const char filedata[] = "#define X Y\n"
                                "#ifndef X\n"
                                "Fred & Wilma\n"
                                "#else\n"
                                "Barney & Betty\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        settings.userUndefs.insert("X"); // User undefs should override internal defines

        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1U, actual.size());
        ASSERT_EQUALS("\n\nFred & Wilma\n\n\n\n", actual[""]);*/
    }

    void undef10() {
        /*Settings settings;

        const char filedata[] = "#ifndef X\n"
                                "#endif\n"
                                "#ifndef Y\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        settings.userUndefs.insert("X"); // User undefs should override internal defines

        Preprocessor preprocessor(&settings, this);

        std::string processedFile;
        std::list<std::string> resultConfigurations;
        const std::list<std::string> includePaths;
        preprocessor.preprocess(istr, processedFile, resultConfigurations, "file.c", includePaths);


        // Compare results. Two configurations "" and "Y". No "X".
        ASSERT_EQUALS(2U, resultConfigurations.size());
        ASSERT_EQUALS("", resultConfigurations.front());
        ASSERT_EQUALS("Y", resultConfigurations.back());*/
    }

    void macroChar() {
        const char filedata[] = "#define X 1\nX\n";

        Preprocessor2 preprocessor(0, this);
        preprocess(preprocessor, filedata);

        ASSERT_EQUALS("$1", preprocessor.cfg[""]->tokenizer.tokens()->stringifyList(true, true, false, false, false));
    }

    void handleUndef() {
        /*Settings settings;
        settings.userUndefs.insert("X");
        const Preprocessor preprocessor(&settings, this);
        std::list<std::string> configurations;

        // configurations to keep
        configurations.clear();
        configurations.push_back("XY;");
        configurations.push_back("AX;");
        configurations.push_back("A;XY");
        preprocessor.handleUndef(configurations);
        ASSERT_EQUALS(3U, configurations.size());

        // configurations to remove
        configurations.clear();
        configurations.push_back("X;Y");
        configurations.push_back("X=1;Y");
        configurations.push_back("A;X;B");
        configurations.push_back("A;X=1;B");
        configurations.push_back("A;X");
        configurations.push_back("A;X=1");
        preprocessor.handleUndef(configurations);
        ASSERT_EQUALS(0U, configurations.size());*/
    }

    void validateCfg() {
        /*Settings settings;
        Preprocessor preprocessor(&settings, this);

        ASSERT_EQUALS(true, preprocessor.validateCfg("", "X=42"));  // don't hang when parsing cfg
        ASSERT_EQUALS(false, preprocessor.validateCfg("int y=Y;", "X=42;Y"));
        ASSERT_EQUALS(false, preprocessor.validateCfg("int x=X;", "X"));
        ASSERT_EQUALS(false, preprocessor.validateCfg("X=1;", "X"));
        ASSERT_EQUALS(true, preprocessor.validateCfg("int x=X;", "Y"));
        ASSERT_EQUALS(true, preprocessor.validateCfg("FOO_DEBUG()", "DEBUG"));
        ASSERT_EQUALS(true, preprocessor.validateCfg("\"DEBUG()\"", "DEBUG"));
        ASSERT_EQUALS(true, preprocessor.validateCfg("\"\\\"DEBUG()\"", "DEBUG"));
        ASSERT_EQUALS(false, preprocessor.validateCfg("\"DEBUG()\" DEBUG", "DEBUG"));
        ASSERT_EQUALS(true, preprocessor.validateCfg("#undef DEBUG", "DEBUG"));*/
    }

    void if_sizeof() { // #4071
        /*static const char* code = "#if sizeof(unsigned short) == 2\n"
                                  "Fred & Wilma\n"
                                  "#elif sizeof(unsigned short) == 4\n"
                                  "Fred & Wilma\n"
                                  "#else\n"
                                  "#endif";

        Settings settings;
        Preprocessor preprocessor(&settings, this);
        std::istringstream istr(code);
        std::map<std::string, std::string> actual;
        preprocessor.preprocess(istr, actual, "file.c");
        ASSERT_EQUALS("\nFred & Wilma\n\n\n\n\n", actual[""]);*/
    }

    void double_include() {
        /*const char code[] = "int x";

        Preprocessor preprocessor(nullptr, this);
        std::list<std::string> includePaths;
        includePaths.push_back(".");
        includePaths.push_back(".");
        std::map<std::string,std::string> defs;
        std::set<std::string> pragmaOnce;
        preprocessor.handleIncludes(code, "123.h", includePaths, defs, pragmaOnce, std::list<std::string>());*/
    }

    void invalid_ifs()  {
        /*const char filedata[] = "#ifdef\n"
                                "#endif\n"
                                "#ifdef !\n"
                                "#endif\n"
                                "#if defined\n"
                                "#endif\n"
                                "#define f(x) x\n"
                                "#if f(2\n"
                                "#endif\n"
                                "int x;\n";

        // Preprocess => don't crash..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");
        */
    }
};

REGISTER_TEST(TestPreprocessor)
