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
#ifndef timerH
#define timerH
//---------------------------------------------------------------------------

#include "config.h"
#include "settings.h"

#include <ctime>
#include <map>
#include <string>


class CPPCHECKLIB TimerResults {
public:
    struct Data {
        std::clock_t mClocks;
        std::clock_t mAdditionalClocks;
        std::size_t mNumberOfResults;

        Data()
            : mClocks(0)
            , mAdditionalClocks(0)
            , mNumberOfResults(0) {
        }

        double seconds() const {
            const double ret = (double)(mClocks) / (double)CLOCKS_PER_SEC;
            return ret;
        }

        double fullSeconds() const {
            const double ret = (double)(mClocks + mAdditionalClocks) / (double)CLOCKS_PER_SEC;
            return ret;
        }
    };

    std::map<std::string, Data> mResults;
};

class CPPCHECKLIB Timer {
public:
    Timer(const std::string& str, Settings::SHOWTIME_MODES showtimeMode);
    ~Timer();
    void start(bool intermediate = false);
    void stop(bool intermediate = false);

    const std::string& getName() const {
        return mStr;
    }

    static TimerResults results;

private:
    Timer(const Timer& other) = delete;
    Timer& operator=(const Timer&) = delete;

    const std::string mStr;
    std::clock_t mStart;
    std::clock_t mAccumulated;
    const Settings::SHOWTIME_MODES mShowTimeMode;
    bool mStopped;
};

//---------------------------------------------------------------------------
#endif // timerH
