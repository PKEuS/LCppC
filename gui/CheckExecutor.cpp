#include "CheckExecutor.h"
#include "../lib/settings.h"
#include "../lib/cppcheck.h"
#include "../lib/analyzerinfo.h"
#include "../lib/threadexecutor.h"
#include "../lib/path.h"


ErrorLogger* CheckExecutor::errorlogger = nullptr;


bool tryLoadLibrary(Library& destination, const char* basepath, const char* filename)
{
    const Library::Error err = destination.load(basepath, filename);

    if (err.errorcode != Library::OK) {
        // TODO: Error Handling
        return false;
    }
    return true;
}

void CheckExecutor::init(Settings& settings)
{
    wxString exepath = wxStandardPaths::Get().GetExecutablePath();

    // Load libraries
    const bool std = tryLoadLibrary(settings.library, exepath.c_str(), "std.cfg");

    for (const std::string& lib : settings.libraries) {
        if (!tryLoadLibrary(settings.library, exepath.c_str(), lib.c_str())) {
            const std::string msg("Failed to load the library " + lib);
            const std::list<ErrorMessage::FileLocation> callstack;
            ErrorMessage errmsg(callstack, emptyString, Severity::information, msg, "failedToLoadCfg", Certainty::safe);
            errorlogger->reportErr(errmsg);
            return;
        }
    }

    bool posix = true;
    if (settings.posix())
        posix = tryLoadLibrary(settings.library, exepath.c_str(), "posix.cfg");
    bool windows = true;
    if (settings.isWindowsPlatform())
        windows = tryLoadLibrary(settings.library, exepath.c_str(), "windows.cfg");

    if (!std || !posix || !windows) {
        const std::list<ErrorMessage::FileLocation> callstack;
        const std::string msg("Failed to load " + std::string(!std ? "std.cfg" : !posix ? "posix.cfg" : "windows.cfg") + ". Your Cppcheck installation is broken, please re-install.");
#ifdef FILESDIR
        const std::string details("The Cppcheck binary was compiled with FILESDIR set to \""
                                  FILESDIR "\" and will therefore search for "
                                  "std.cfg in " FILESDIR "/cfg.");
#else
        const std::string cfgfolder(Path::fromNativeSeparators(Path::getPathFromFilename(exepath.ToStdString())) + "cfg"); // TODO
        const std::string details("The Cppcheck binary was compiled without FILESDIR set. Either the "
                                  "std.cfg should be available in " + cfgfolder + " or the FILESDIR "
                                  "should be configured.");
#endif
        ErrorMessage errmsg(callstack, emptyString, Severity::information, msg + " " + details, "failedToLoadCfg", Certainty::safe);
        errorlogger->reportErr(errmsg);
        return;
    }
}
void CheckExecutor::check(Settings& settings)
{
    AnalyzerInformation analyzerInformation;

    CppCheck cppcheck(*errorlogger, settings, false);

    ThreadExecutor executor(analyzerInformation.getCTUs(), settings, *errorlogger);
    executor.check();

    cppcheck.analyseWholeProgram(analyzerInformation);
}
void CheckExecutor::check(Settings& settings, const wxString& filename, const wxString& code)
{
    AnalyzerInformation analyzerInformation;
    CTU::CTUInfo& ctu = analyzerInformation.addCTU(filename.ToStdString(), code.size(), emptyString);

    CppCheck cppcheck(*errorlogger, settings, false);

    cppcheck.check(&ctu, code.ToStdString());

    cppcheck.analyseWholeProgram(analyzerInformation);
}
