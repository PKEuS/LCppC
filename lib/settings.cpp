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

#include "settings.h"

#include "valueflow.h"

std::atomic<bool> Settings::mTerminated;

const char Project::SafeChecks::XmlRootName[] = "safe-checks";
const char Project::SafeChecks::XmlClasses[] = "class-public";
const char Project::SafeChecks::XmlExternalFunctions[] = "external-functions";
const char Project::SafeChecks::XmlInternalFunctions[] = "internal-functions";
const char Project::SafeChecks::XmlExternalVariables[] = "external-variables";


Settings::Settings() :
    checkConfiguration(false),
    checkLibrary(false),
    debugnormal(false),
    debugtemplate(false),
    debugwarnings(false),
    dump(false),
    exceptionHandling(false),
    exitCode(0),
    jobs(1),
    relativePaths(false),
    showtime(SHOWTIME_MODES::SHOWTIME_NONE),
    verbose(false),
    xml(false),
    xml_version(2)
{
    output.setEnabled(Output::findings, true);
}

Project::Project() :
    checkAllConfigurations(true),
    checkHeaders(true),
    checkUnusedTemplates(false),
    enforcedLang(None),
    force(false),
    inlineSuppressions(false),
    maxConfigs(12),
    maxCtuDepth(2),
    maxTemplateRecursion(100),
    preprocessOnly(false)
{
    severity.setEnabled(Severity::error, true);
    certainty.setEnabled(Certainty::safe, true);
    checks.setEnabled("MissingInclude", false);
}

bool Project::isEnabled(const ValueFlow::Value *value, bool inconclusiveCheck) const
{
    if (!severity.isEnabled(Severity::warning) && (value->condition || value->defaultArg))
        return false;
    if (!certainty.isEnabled(Certainty::inconclusive) && (inconclusiveCheck || value->isInconclusive()))
        return false;
    return true;
}

void Project::addLibrary(const std::string& libname)
{
    if (libname.size() > 4 && libname.compare(libname.size()-4, 4, ".cfg") == 0)
        libraries.emplace(libname.substr(0, libname.size()-4));
    else
        libraries.emplace(libname);
}
