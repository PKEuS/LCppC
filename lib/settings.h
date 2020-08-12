/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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
#ifndef settingsH
#define settingsH
//---------------------------------------------------------------------------

#include "config.h"
#include "library.h"
#include "platform.h"
#include "standards.h"
#include "suppressions.h"

#include <algorithm>
#include <atomic>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>

namespace ValueFlow {
    class Value;
}

/// @addtogroup Core
/// @{

template<typename T>
class SimpleEnableGroup {
    uint32_t flags = 0;
public:
    uint32_t intValue() const {
        return flags;
    }
    void clear() {
        flags = 0;
    }
    void fill() {
        flags = 0xFFFFFFFF;
    }
    void setEnabledAll(bool enabled) {
        if (enabled)
            fill();
        else
            clear();
    }
    bool isEnabled(T flag) const {
        return (flags & (1U << (uint32_t)flag)) != 0;
    }
    void enable(T flag) {
        flags |= (1U << (uint32_t)flag);
    }
    void disable(T flag) {
        flags &= ~(1U << (uint32_t)flag);
    }
    void setEnabled(T flag, bool enabled) {
        if (enabled)
            enable(flag);
        else
            disable(flag);
    }
};

class ComplexEnableGroup {
    std::unordered_map<std::string, bool> flags;
    bool unknownDefault = true;
public:
    void clear() {
        unknownDefault = false;
        flags.clear();
    }
    void fill() {
        unknownDefault = true;
        flags.clear();
    }
    void setEnabledAll(bool enabled) {
        if (enabled)
            fill();
        else
            clear();
    }
    bool isEnabled(const std::string& flag) const {
        auto it = flags.find(flag);
        if (it == flags.cend())
            return unknownDefault;
        return it->second;
    }
    void enable(const std::string& flag) {
        flags[flag] = true;
    }
    void disable(const std::string& flag) {
        flags[flag] = false;
    }
    void setEnabled(const std::string& flag, bool enabled) {
        if (enabled)
            enable(flag);
        else
            disable(flag);
    }
};


/**

 * @brief This is just a container for general settings so that we don't need
 * to pass individual values to functions or constructors now or in the
 * future when we might have even more detailed settings.
 */
class CPPCHECKLIB Settings : public cppcheck::Platform {
private:

    /** @brief terminate checking */
    static std::atomic<bool> mTerminated;

public:
    Settings();

    /** @brief addons, either filename of python/json file or json data */
    std::vector<std::string> addons;

    /** @brief Path to the python interpreter to be used to run addons. */
    std::string addonPython;

    /** @brief Paths used as base for conversion to relative paths. */
    std::vector<std::string> basePaths;

    /** @brief --cppcheck-build-dir */
    std::string buildDir;

    /** @brief check all configurations (false if -D or --max-configs is used */
    bool checkAllConfigurations;

    /** Is the 'configuration checking' wanted? */
    bool checkConfiguration;

    /**
     * Check code in the headers, this is on by default but can
     * be turned off to save CPU */
    bool checkHeaders;

    /** Check for incomplete info in library files? */
    bool checkLibrary;

    /** @brief check unknown function return values */
    std::set<std::string> checkUnknownFunctionReturn;

    /** @brief include paths excluded from checking the configuration */
    std::set<std::string> configExcludePaths;

    /** Check unused/uninstantiated templates */
    bool checkUnusedTemplates;

    /** @brief Is --debug-normal given? */
    bool debugnormal;

    /** @brief Is --debug-template given? */
    bool debugtemplate;

    /** @brief Is --debug-warnings given? */
    bool debugwarnings;

    /** @brief Is --dump given? */
    bool dump;
    std::string dumpFile;

    enum Language : uint8_t {
        None, C, CPP
    };

    /** @brief Name of the language that is enforced. Empty per default. */
    Language enforcedLang;

    /** @brief Is --exception-handling given */
    bool exceptionHandling;

    // argv[0]
    std::string exename;

    /** @brief --file-filter for analyzing special files */
    std::string fileFilter;

    /** @brief List of include paths, e.g. "my/includes/" which should be used
        for finding include files inside source files. (-I) */
    std::vector<std::string> includePaths;

    /** @brief If errors are found, this value is returned from main().
        Default value is 0. */
    int exitCode;

    /** @brief Force checking the files with "too many" configurations (--force). */
    bool force;

    /** @brief Is --inline-suppr given? */
    bool inlineSuppressions;

    /** @brief How many processes/threads should do checking at the same
        time. Default is 1. (-j N) */
    unsigned int jobs;

    /** @brief --library= */
    std::set<std::string> libraries;

    /** Library */
    Library library;

    /** @brief Maximum number of configurations to check before bailing.
        Default is 12. (--max-configs=N) */
    unsigned int maxConfigs;

    /** @brief --max-ctu-depth */
    int maxCtuDepth;

    /** @brief max template recursion */
    unsigned int maxTemplateRecursion;

    /** @brief suppress exitcode */
    Suppressions nofail;

    /** @brief suppress message (--suppressions) */
    Suppressions nomsg;

    /** @brief write results (--output-file=&lt;file&gt;) */
    std::string outputFile;

    /** @brief Using -E for debugging purposes */
    bool preprocessOnly;

    /** @brief Use relative paths in output. */
    bool relativePaths;

    /** Rule */
    class CPPCHECKLIB Rule {
    public:
        Rule()
            : tokenlist("simple")         // use simple tokenlist
            , id("rule")                  // default id
            , severity(Severity::style) { // default severity
        }

        std::string tokenlist;
        std::string pattern;
        std::string id;
        std::string summary;
        Severity::SeverityType severity;
    };

    /**
     * @brief Extra rules
     */
    std::vector<Rule> rules;

    /** Do not only check how interface is used. Also check that interface is safe. */
    class CPPCHECKLIB SafeChecks {
    public:
        SafeChecks() : classes(false), externalFunctions(false), internalFunctions(false), externalVariables(false) {}

        static const char XmlRootName[];
        static const char XmlClasses[];
        static const char XmlExternalFunctions[];
        static const char XmlInternalFunctions[];
        static const char XmlExternalVariables[];

        void clear() {
            classes = externalFunctions = internalFunctions = externalVariables = false;
        }

        /**
         * Public interface of classes
         * - public function parameters can have any value
         * - public functions can be called in any order
         * - public variables can have any value
         */
        bool classes;

        /**
         * External functions
         * - external functions can be called in any order
         * - function parameters can have any values
         */
        bool externalFunctions;

        /**
         * Experimental: assume that internal functions can be used in any way
         * This is only available in the GUI.
         */
        bool internalFunctions;

        /**
         * Global variables that can be modified outside the TU.
         * - Such variable can have "any" value
         */
        bool externalVariables;
    };

    SafeChecks safeChecks;

    SimpleEnableGroup<int> output;
    SimpleEnableGroup<Severity::SeverityType> severity;
    SimpleEnableGroup<Certainty::CertaintyLevel> certainty;
    ComplexEnableGroup checks;

    enum SHOWTIME_MODES : uint8_t {
        SHOWTIME_NONE = 0,
        SHOWTIME_FILE,
        SHOWTIME_SUMMARY,
        SHOWTIME_TOP5
    };
    /** @brief show timing information (--showtime=file|summary|top5) */
    SHOWTIME_MODES showtime;

    /** Struct contains standards settings */
    Standards standards;

    /** @brief The output format in which the errors are printed in text mode,
        e.g. "{severity} {file}:{line} {message} {id}" */
    std::string templateFormat;

    /** @brief The output format in which the error locations are printed in
     *  text mode, e.g. "{file}:{line} {info}" */
    std::string templateLocation;

    /** @brief defines given by the user */
    std::string userDefines;

    /** @brief undefines given by the user */
    std::set<std::string> userUndefs;

    /** @brief forced includes given by the user */
    std::vector<std::string> userIncludes;

    /** @brief Is --verbose given? */
    bool verbose;

    /** @brief write XML results (--xml) */
    bool xml;

    /** @brief XML version (--xml-version=..) */
    int xml_version;

    /**
     * @brief return true if a included file is to be excluded in Preprocessor::getConfigs
     * @return true for the file to be excluded.
     */
    bool configurationExcluded(const std::string &file) const {
        for (const std::string & configExcludePath : configExcludePaths) {
            if (file.length()>=configExcludePath.length() && file.compare(0,configExcludePath.length(),configExcludePath)==0) {
                return true;
            }
        }
        return false;
    }

    /**
    * @brief Returns true if given value can be shown
    * @return true if the value can be shown
    */
    bool isEnabled(const ValueFlow::Value *value, bool inconclusiveCheck=false) const;

    /** Is particular library specified? */
    bool hasLibrary(const char* lib) const {
        return libraries.find(lib) != libraries.end();
    }

    void addLibrary(const std::string& libname);

    /** @brief Request termination of checking */
    static void terminate(bool t = true) {
        Settings::mTerminated = t;
    }

    /** @brief termination requested? */
    static bool terminated() {
        return Settings::mTerminated;
    }
};

/// @}
//---------------------------------------------------------------------------
#endif // settingsH
