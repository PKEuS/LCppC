===========
Cppcheck v1
===========


About

    This is a fork of the original Cppcheck project, that aims at providing a lean, fast and
    mature static code analysis tool for C and C++.

Manual

    A manual (for mainline cppcheck) is available online. It should be mostly compatible with
    this version: https://cppcheck.sourceforge.net/manual.pdf

Compiling

    Any C++11 compiler should work. For compilers with partial C++11 support it may work. If
    your compiler has the C++11 features that are available in Visual Studio 2013 / GCC 4.6
    then it will work.

    To build the GUI, you need Qt.

    When building the command line tool, PCRE is optional. It is used if you build with rules.

    There are multiple compilation choices:
      * qmake - cross platform build tool
      * Windows: Visual Studio
      * Windows: Qt Creator + mingw
      * gnu make
      * g++ 4.6 (or later)
      * clang++

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

    gnu make
    ========
        Simple build (no dependencies):
            make

        The recommended release build is:
            make MATCHCOMPILER=yes FILESDIR=/usr/share/cppcheck HAVE_RULES=yes

        Flags:
        MATCHCOMPILER=yes               : Python is used to optimise cppcheck at compile time
        FILESDIR=/usr/share/cppcheck    : Specify folder where cppcheck files are installed
        HAVE_RULES=yes                  : Enable rules (pcre is required if this is used)

    g++ (for experts)
    =================
        If you just want to build Cppcheck without dependencies then you can use this command:
            g++ -o cppcheck -std=c++11 -Iexternals -Iexternals/simplecpp -Iexternals/tinyxml -Ilib cli/*.cpp lib/*.cpp externals/simplecpp/simplecpp.cpp externals/tinyxml/*.cpp

        If you want to use --rule and --rule-file then dependencies are needed:
            g++ -o cppcheck -std=c++11 -lpcre -DHAVE_RULES -Ilib -Iexternals -Iexternals/simplecpp -Iexternals/tinyxml cli/*.cpp lib/*.cpp externals/simplecpp/simplecpp.cpp externals/tinyxml/*.cpp

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

    https://cppcheck.sourceforge.net/ (webpage of the mainline cppcheck project)
