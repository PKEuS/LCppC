/*
* Cppcheck - A tool for static C/C++ code analysis
* Copyright (C) 2007-2018 Cppcheck team.
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
#ifndef analyzerinfoH
#define analyzerinfoH
//---------------------------------------------------------------------------

#include "config.h"
#include "ctu.h"

#include <fstream>
#include <list>
#include <string>

class ErrorMessage;

/// @addtogroup Core
/// @{

/**
* @brief Analyzer information
*
* Store various analysis information:
* - checksum
* - error messages
* - whole program analysis data
*
* The information can be used for various purposes. It allows:
* - 'make' - only analyze TUs that are changed and generate full report
* - should be possible to add distributed analysis later
* - multi-threaded whole program analysis
*/
class CPPCHECKLIB AnalyzerInformation {
public:
    ~AnalyzerInformation();

    void createCTUs(const std::string &buildDir, const std::map<std::string, std::size_t>& sourcefiles);
    CTU::CTUInfo& addCTU(const std::string& sourcefile, std::size_t filesize, const std::string& analyzerfile) {
        return mFileInfo.emplace_back(sourcefile, filesize, analyzerfile);
    }

    /** Close current TU.analyzerinfo file */
    void close();
    bool analyzeFile(const std::string &buildDir, const std::string &sourcefile, const std::string &cfg, unsigned long long checksum, std::list<ErrorMessage> *errors);
    void reportErr(const ErrorMessage &msg, bool verbose);
    static std::string getAnalyzerInfoFile(const std::string &buildDir, const std::string &sourcefile, const std::string &cfg);
    std::list<CTU::CTUInfo>& getCTUs() {
        return mFileInfo;
    }

private:
    /** File info used for whole program analysis */
    std::list<CTU::CTUInfo> mFileInfo;

    std::ofstream mOutputStream;
    std::string mAnalyzerInfoFile;
};

/// @}
//---------------------------------------------------------------------------
#endif // analyzerinfoH
