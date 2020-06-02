# **Lean Cppcheck** 

|License|
|:-:|
|[![License](https://img.shields.io/badge/license-GPL3.0-blue.svg)](https://opensource.org/licenses/GPL-3.0) 

## About the name

This is a fork of the original Cppcheck project, that aims at providing a lean, fast and
mature static code analysis tool for C and C++.

## Manual

A manual (for mainline cppcheck) is available online. It should be mostly compatible with this Lean Cppcheck: https://cppcheck.sourceforge.net/manual.pdf

## Compiling

Any modern C++ compiler should work. If your compiler has the C++11 features that are available in Visual Studio 2019 / GCC 10 then it will work.

To build the GUI, you need Qt.

When building the command line tool, [PCRE](http://www.pcre.org/) is optional. It is used if you build with rules.

There are multiple compilation choices:
* qmake - cross platform build tool
* Windows: Visual Studio
* Windows: Qt Creator + mingw
* Linux/BSD: GNU Make with g++ or clang++
* Linux/BSD: gcc++ or clang++ directly

### qmake

You can use the gui/gui.pro file to build the GUI.

```shell
cd gui
qmake
make
```

### Visual Studio

Use the cppcheck.sln file. The file is configured for Visual Studio 2019, but the platform toolset can be changed easily to older or newer versions. The solution contains platform targets for both x86 and x64.

To compile with rules, select "Release-PCRE" or "Debug-PCRE" configuration. pcre.lib (pcre64.lib for x64 builds) and pcre.h are expected to be in /externals then. A current version of PCRE for Visual Studio can be obtained using [vcpkg](https://github.com/microsoft/vcpkg).

### Qt Creator + MinGW

The PCRE dll is needed to build the CLI. It can be downloaded here:
http://software-download.name/pcre-library-windows/

### GNU make

Simple, unoptimized build (no dependencies):

```shell
make
```

Debug build:

```shell
make DEBUG=yes

The recommended release build is:

```shell
make MATCHCOMPILER=yes FILESDIR=/usr/share/cppcheck HAVE_RULES=yes CXXFLAGS="-O2 -DNDEBUG -Wall -Wno-sign-compare -Wno-unused-function"
```

Flags:

1.  `MATCHCOMPILER=yes`
    Python is used to optimise cppcheck. The Token::Match patterns are converted into C++ code at compile time.

2.  `FILESDIR=/usr/share/cppcheck`
    Specify folder where cppcheck files are installed (addons, cfg, platform)

3.  `HAVE_RULES=yes`
    Enable rules (PCRE is required if this is used)

4.  `DEBUG=yes`
    Use debug configuration.

### g++ (for experts)

If you just want to build Cppcheck without dependencies then you can use this command:

```shell
g++ -o cppcheck -std=c++11 -Iexternals -Iexternals/simplecpp -Iexternals/tinyxml -Ilib cli/*.cpp lib/*.cpp externals/simplecpp/simplecpp.cpp externals/tinyxml/*.cpp
```

If you want to use `--rule` and `--rule-file` then dependencies are needed:

```shell
g++ -o cppcheck -std=c++11 -lpcre -DHAVE_RULES -Ilib -Iexternals -Iexternals/simplecpp -Iexternals/tinyxml cli/*.cpp lib/*.cpp externals/simplecpp/simplecpp.cpp externals/tinyxml/*.cpp
```

### MinGW

```shell
mingw32-make LDFLAGS=-lshlwapi
```

### Other Compiler/IDE

1. Create an empty project file / makefile.
2. Add all cpp files in the cppcheck cli and lib folders to the project file / makefile.
3. Add all cpp files in the externals folders to the project file / makefile.
4. Compile.

### Cross compiling Win32 (CLI) version of Cppcheck in Linux

```shell
sudo apt-get install mingw32
make CXX=i586-mingw32msvc-g++ LDFLAGS="-lshlwapi" RDYNAMIC=""
mv cppcheck cppcheck.exe
```

## Webpage

https://cppcheck.sourceforge.net/ (webpage of the mainline cppcheck project)
