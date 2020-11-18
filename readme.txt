=====================
LCppC - Lean Cppcheck
=====================


About

    This is a fork of the Cppcheck project, that aims at providing a lean, fast and
    mature static code analysis tool for C and C++.

Manual

    A manual (for Cppcheck) is available online. It should be mostly compatible with LCppC:
    http://cppcheck.sourceforge.net/manual.pdf

Compiling

    Any modern C++ compiler should work. If your compiler has the C++11 features that are
    available in Visual Studio 2019 / Clang 8 / GCC 8 then it will work.

    To build the GUI, you need Qt.

    While building the command line tool, PCRE is optional. It is used if you build with rules.

    There are multiple compilation choices:
      * qmake - cross platform build tool
      * Windows: Visual Studio
      * Windows: Qt Creator + mingw
      * Linux/BSD: GNU Make with g++ or clang++
      * Linux/BSD: gcc++ or clang++ directly

    qmake
    =====
        You can use the gui/gui.pro file to build the GUI.
            cd gui
            qmake
            make

    Visual Studio
    =============
        Use the cppcheck.sln file. The file is configured for Visual Studio 2019, but the platform
        toolset can be changed easily to older or newer versions. The solution contains platform
        targets for both x86 and x64.

        To compile with rules, select "Release-PCRE" or "Debug-PCRE" configuration.
        pcre.lib (pcre64.lib for x64 builds) and pcre.h are expected to be in /externals then.
        A current version of PCRE for Visual Studio can be obtained using vcpkg:
        https://github.com/microsoft/vcpkg

    Qt Creator + mingw
    ==================
        The PCRE dll is needed to build the CLI. It can be downloaded here:
            http://software-download.name/pcre-library-windows/

    GNU Make
    ========
        Simple build (no dependencies):
            make

        Debug build:
            make DEBUG=yes

        The recommended release build is:
            make MATCHCOMPILER=yes FILESDIR=/usr/share/cppcheck HAVE_RULES=yes

        Flags:
        MATCHCOMPILER=yes               : Python is used to optimise cppcheck at compile time
        DEBUG=yes                       : Use debug configuration
        FILESDIR=/usr/share/cppcheck    : Specify folder where cppcheck files are installed
        HAVE_RULES=yes                  : Enable rules (pcre is required if this is used)

    g++ (for experts)
    =================
        If you just want to build Cppcheck without dependencies then you can use this command:
            g++ -o cppcheck -std=c++11 -Iexternals -Iexternals/simplecpp -Iexternals/tinyxml2 -Ilib cli/*.cpp lib/*.cpp externals/simplecpp/simplecpp.cpp externals/tinyxml2/*.cpp

        If you want to use --rule and --rule-file then dependencies are needed:
            g++ -o cppcheck -std=c++11 -lpcre -DHAVE_RULES -Ilib -Iexternals -Iexternals/simplecpp -Iexternals/tinyxml2 cli/*.cpp lib/*.cpp externals/simplecpp/simplecpp.cpp externals/tinyxml2/*.cpp

    mingw
    =====
        The "LDFLAGS=-lshlwapi" is needed when building with mingw
            mingw32-make LDFLAGS=-lshlwapi

    other compilers/ide
    ===================

        1. Create a empty project file / makefile.
        2. Add all cpp files in the cppcheck cli and lib folders to the project file / makefile.
        3. Add all cpp files in the externals folders to the project file / makefile.
        4. Compile.

    Cross compiling Win32 (CLI) version of Cppcheck in Linux

        sudo apt-get install mingw32
        make CXX=i586-mingw32msvc-g++ LDFLAGS="-lshlwapi"
        mv cppcheck cppcheck.exe

Webpage

    http://cppcheck.sourceforge.net/ (webpage of the mainline cppcheck project)
