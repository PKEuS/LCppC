/*
 * LCppC - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2019 Cppcheck team.
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


//---------------------------------------------------------------------------
#ifndef checkboostH
#define checkboostH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"
#include "tokenize.h"

#include <string>

/// @addtogroup Checks
/// @{


/** @brief %Check Boost usage */
class CPPCHECKLIB CheckBoost : public Check {
public:
    /** This constructor is used when registering the CheckClass */
    CheckBoost() : Check(myName()) {
    }

    /** This constructor is used when running checks. */
    explicit CheckBoost(Context ctx)
        : Check(myName(), ctx) {
    }

    /** @brief Run checks against the normal token list */
    void runChecks(Context ctx) override {
        if (!ctx.tokenizer->isCPP())
            return;

        CheckBoost checkBoost(ctx);
        checkBoost.checkBoostForeachModification();
    }

    /** @brief %Check for container modification while using the BOOST_FOREACH macro */
    void checkBoostForeachModification();

private:
    void boostForeachError(const Token *tok);

    void getErrorMessages(Context ctx) const override {
        CheckBoost c(ctx);
        c.boostForeachError(nullptr);
    }

    static const char* myName() {
        return "Boost";
    }

    std::string classInfo() const override {
        return "Check for invalid usage of Boost:\n"
               "- container modification during BOOST_FOREACH\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkboostH
