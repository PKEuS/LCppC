/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2019 Cppcheck team.
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

#include "timer.h"

#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>

namespace {
    typedef std::pair<std::string, struct TimerResultsData> dataElementType;
    bool more_second_sec(const dataElementType& lhs, const dataElementType& rhs)
    {
        return lhs.second.seconds() > rhs.second.seconds();
    }
}

void TimerResults::showResults(Settings::SHOWTIME_MODES mode) const
{
    if (mode == Settings::SHOWTIME_NONE)
        return;

    std::cout << "\nTimings: exclusive / inclusive (averages), all in seconds\n";

    TimerResultsData overallData;

    std::vector<dataElementType> data(mResults.begin(), mResults.end());
    std::sort(data.begin(), data.end(), more_second_sec);

    std::cout.precision(3);
    std::cout.setf(std::ios::fixed);
    std::cout.setf(std::ios::showpoint);

    size_t width = 0;

    for (std::vector<dataElementType>::const_iterator iter = data.begin(); iter != data.end(); ++iter)
        width = std::max(width, iter->first.size());

    size_t ordinal = 1; // maybe it would be nice to have an ordinal in output later!
    for (std::vector<dataElementType>::const_iterator iter=data.begin() ; iter!=data.end(); ++iter) {
        const double sec1 = iter->second.seconds();
        const double secAverage1 = sec1 / (double)(iter->second.mNumberOfResults);
        const double sec2 = iter->second.fullSeconds();
        const double secAverage2 = sec2 / (double)(iter->second.mNumberOfResults);
        overallData.mClocks += iter->second.mClocks;
        if ((mode != Settings::SHOWTIME_TOP5) || (ordinal<=5)) {
            std::cout << iter->first << ": " << std::string(width-iter->first.size(), ' ');
            std::cout << sec1 << " / " << sec2 << " (" << secAverage1 << " / " << secAverage2 << " - " << iter->second.mNumberOfResults;
            std::cout << (iter->second.mNumberOfResults == 1 ? " result)" : " results)") << std::endl;
        }
        ++ordinal;
    }

    const double secOverall = overallData.seconds();
    std::cout << "Overall time: " << secOverall << "s" << std::endl;
}

void TimerResults::start(Timer* timer, bool intermediate)
{
    if (!intermediate) {
        if (!mHierachy.empty())
            mHierachy.back()->stop(true);
        mHierachy.push_back(timer);
    }
}

void TimerResults::stop(Timer* timer, std::clock_t clocks, bool intermediate)
{
    if (!intermediate) {
        mHierachy.pop_back();
        if (!mHierachy.empty()) {
            mHierachy.back()->start(true);
        }
    }
    for (size_t i = 0; i < mHierachy.size(); i++)
        if (mHierachy[i] != timer)
            mResults[mHierachy[i]->getName()].mAdditionalClocks += clocks;
    mResults[timer->getName()].mClocks += clocks;
    if (!intermediate)
        mResults[timer->getName()].mNumberOfResults++;
}

Timer::Timer(const std::string& str, Settings::SHOWTIME_MODES showtimeMode, TimerResults* timerResults)
    : mStr(str)
    , mTimerResults(timerResults)
    , mStart(0)
    , mAccumulated(0)
    , mShowTimeMode(showtimeMode)
    , mStopped(true)
{
    start();
}

Timer::~Timer()
{
    stop();

    if (mShowTimeMode == Settings::SHOWTIME_FILE) {
        const double sec = (double)mAccumulated / CLOCKS_PER_SEC;
        std::cout << mStr << ": " << sec << "s" << std::endl;
    }
}

void Timer::start(bool intermediate)
{
    if (!mTimerResults)
        return;
    if (mShowTimeMode != Settings::SHOWTIME_NONE && mStopped) {
        mStopped = false;
        if (!intermediate)
            mTimerResults->start(this, intermediate);
        mStart = std::clock();
    }
}

void Timer::stop(bool intermediate)
{
    if (!mTimerResults)
        return;
    if ((mShowTimeMode != Settings::SHOWTIME_NONE) && !mStopped) {
        const std::clock_t end = std::clock();
        const std::clock_t diff = end - mStart;

        mAccumulated += diff;
        if (mShowTimeMode == Settings::SHOWTIME_FILE) {
        } else
            mTimerResults->stop(this, diff, intermediate);
    }

    mStopped = true;
}
