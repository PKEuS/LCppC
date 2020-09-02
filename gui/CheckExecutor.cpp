/*
 * LCppC - A tool for static C/C++ code analysis
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

#include "CheckExecutor.h"
#include "../lib/settings.h"
#include "../lib/cppcheck.h"
#include "../lib/analyzerinfo.h"
#include "../lib/threadexecutor.h"
#include "../lib/path.h"
#include "../lib/version.h"


ErrorLogger* CheckExecutor::errorlogger = nullptr;
Settings CheckExecutor::settings;
static size_t libcount = 0;

bool tryLoadLibrary(Library& destination, const char* basepath, const char* filename)
{
    libcount++;
    const Library::Error err = destination.load(basepath, filename);

    if (err.errorcode != Library::OK) {
        // TODO: Error Handling
        return false;
    }
    return true;
}

void CheckExecutor::init(Project& project)
{
    libcount = 0;
    wxString exepath = wxStandardPaths::Get().GetExecutablePath();

    // Load libraries
    project.libraries.emplace("std");
    if (project.isWindowsPlatform())
        project.libraries.emplace("windows");

    for (const std::string& lib : project.libraries) {
        if (!tryLoadLibrary(project.library, exepath.c_str(), lib.c_str())) {
            std::string msg, details;
            if (lib == "std" || lib == "windows") {
                msg = "Failed to load '" + lib + ".cfg'. Your Cppcheck installation is broken, please re-install. ";
#ifdef FILESDIR
                details = "The " PROGRAMNAME " binary was compiled with FILESDIR set to \""
                          FILESDIR "\" and will therefore search for "
                          "std.cfg in " FILESDIR "/cfg.";
#else
                const std::string cfgfolder(Path::fromNativeSeparators(Path::getPathFromFilename(exepath.ToStdString())) + "cfg");
                details = "The " PROGRAMNAME " binary was compiled without FILESDIR set. Either the "
                          "std.cfg should be available in " + cfgfolder + " or the FILESDIR "
                          "should be configured.";
#endif
            } else {
                msg = "Failed to load '" + lib + ".cfg'.";
            }
            const std::list<ErrorMessage::FileLocation> callstack;
            ErrorMessage errmsg(callstack, emptyString, Severity::information, msg + " " + details, "failedToLoadCfg", Certainty::safe);
            errorlogger->reportErr(errmsg);
            return;
        }
    }
}

void CheckExecutor::check(Project& project, const wxString& directory)
{
    AnalyzerInformation analyzerInformation;

    CppCheck cppcheck(*errorlogger, settings, project, false);

    ThreadExecutor executor(analyzerInformation.getCTUs(), settings, project, *errorlogger);
    executor.checkSync();

    cppcheck.analyseWholeProgram(analyzerInformation);
}
void CheckExecutor::check(Project& project, const wxString& filename, const wxString& code)
{
    std::size_t status_init = libcount * 1024; // Assume initialization is equivalent to a 1024 bytes file per library loaded
    std::size_t status_wpa = (std::size_t)(code.size() * 0.02); // Assume whole program analysis is 2% of runtime

    AnalyzerInformation analyzerInformation;
    CTU::CTUInfo& ctu = analyzerInformation.addCTU(filename.ToStdString(), code.size(), emptyString);

    CppCheck cppcheck(*errorlogger, settings, project, false);

    errorlogger->reportStatus(1, 1, status_init, status_init + status_wpa + code.size());

    cppcheck.check(&ctu, code.ToStdString());

    errorlogger->reportStatus(1, 1, status_init + code.size(), status_init + status_wpa + code.size());

    cppcheck.analyseWholeProgram(analyzerInformation);

    errorlogger->reportStatus(1, 1, status_init + status_wpa + code.size(), status_init + status_wpa + code.size());
}
