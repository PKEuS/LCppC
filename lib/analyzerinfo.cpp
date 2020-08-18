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

#include "analyzerinfo.h"

#include "errorlogger.h"
#include "path.h"

#include <cstdio>
#include <cstring>
#include <map>
#include <fstream>


static std::string getFilename(const std::string &fullpath)
{
    std::string::size_type pos1 = fullpath.find_last_of("/\\");
    pos1 = (pos1 == std::string::npos) ? 0U : (pos1 + 1U);
    std::string::size_type pos2 = fullpath.rfind('.');
    if (pos2 < pos1)
        pos2 = std::string::npos;
    if (pos2 != std::string::npos)
        pos2 = pos2 - pos1;
    return fullpath.substr(pos1,pos2);
}

void AnalyzerInformation::createCTUs(const std::string &buildDir, const std::map<std::string, std::size_t>& sourcefiles)
{
    if (buildDir.empty()) {
        for (auto it = sourcefiles.cbegin(); it != sourcefiles.cend(); ++it)
            mFileInfo.emplace_back(it->first, it->second, emptyString);
        return;
    }

    const std::string filesTxt(buildDir + "/files.txt");
    std::map<std::string, unsigned int> fileCount;

    // Read existing files.txt
    std::map<std::string, std::string> existingFiles;
    std::ifstream fin(filesTxt);
    std::string filesTxtLine;
    while (std::getline(fin, filesTxtLine)) {
        const std::string::size_type firstColon = filesTxtLine.find(':');
        if (firstColon == std::string::npos)
            continue;

        const std::string::size_type lastColon = filesTxtLine.find(':', firstColon+1);
        if (lastColon == std::string::npos)
            continue;

        std::string filename = filesTxtLine.substr(0, firstColon);
        existingFiles[filesTxtLine.substr(lastColon+1)] = filename;
        ++fileCount[filename.substr(0, filename.rfind('.'))];
    }
    fin.close();

    // Create new files.txt
    std::ofstream fout(filesTxt);
    for (auto it = sourcefiles.cbegin(); it != sourcefiles.cend(); ++it) {
        const std::string path = Path::simplifyPath(Path::fromNativeSeparators(it->first));
        auto file = existingFiles.find(path);
        bool existing = (file != existingFiles.cend());
        std::string afile;
        if (existing) {
            afile = file->second;
            existingFiles.erase(path);
        } else {
            const std::string filename = getFilename(it->first);
            afile = filename + ".a" + std::to_string(++fileCount[filename]);
        }

        fout << afile << "::" << path << '\n';
        mFileInfo.emplace_back(it->first, it->second, buildDir + '/' + afile);
        mFileInfo.back().analyzerfileExists = existing;
    }

    // Remove stale analyzer files
    for (auto it = existingFiles.cbegin(); it != existingFiles.cend(); ++it) {
        std::remove(it->second.c_str());
    }
}
