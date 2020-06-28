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

#include "settings.h"

#include "valueflow.h"

std::atomic<bool> Settings::mTerminated;

const char Settings::SafeChecks::XmlRootName[] = "safe-checks";
const char Settings::SafeChecks::XmlClasses[] = "class-public";
const char Settings::SafeChecks::XmlExternalFunctions[] = "external-functions";
const char Settings::SafeChecks::XmlInternalFunctions[] = "internal-functions";
const char Settings::SafeChecks::XmlExternalVariables[] = "external-variables";

Settings::Settings()
    : checkAllConfigurations(true),
      checkConfiguration(false),
      checkHeaders(true),
      checkLibrary(false),
      checkUnusedTemplates(false),
      debugnormal(false),
      debugtemplate(false),
      debugwarnings(false),
      dump(false),
      enforcedLang(None),
      exceptionHandling(false),
      exitCode(0),
      force(false),
      inlineSuppressions(false),
      jobs(1),
      jointSuppressionReport(false),
      maxConfigs(12),
      maxCtuDepth(2),
      maxTemplateRecursion(100),
      preprocessOnly(false),
      relativePaths(false),
      showtime(SHOWTIME_MODES::SHOWTIME_NONE),
      verbose(false),
      xml(false),
      xml_version(2)
{
    severity.setEnabled(Severity::error, true);
    certainty.setEnabled(Certainty::safe, true);
    checks.setEnabled("MissingInclude", false);
    output.setEnabled(Output::findings, true);
}

bool Settings::isEnabled(const ValueFlow::Value *value, bool inconclusiveCheck) const
{
    if (!severity.isEnabled(Severity::warning) && (value->condition || value->defaultArg))
        return false;
    if (!certainty.isEnabled(Certainty::inconclusive) && (inconclusiveCheck || value->isInconclusive()))
        return false;
    return true;
}
