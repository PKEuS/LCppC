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

#include <iostream>
#include <vector>


TimerResults Timer::results;

namespace {
    struct TimerResultsCollector {
        void start(Timer* timer, bool intermediate);
        void stop(Timer* timer, std::clock_t clocks, bool intermediate);

        std::vector<Timer*> mHierachy;
    };

    thread_local TimerResultsCollector trc;
}

void TimerResultsCollector::start(Timer* timer, bool intermediate)
{
    if (!intermediate) {
        if (!mHierachy.empty())
            mHierachy.back()->stop(true);
        mHierachy.push_back(timer);
    }
}

void TimerResultsCollector::stop(Timer* timer, std::clock_t clocks, bool intermediate)
{
    if (!intermediate) {
        mHierachy.pop_back();
        if (!mHierachy.empty()) {
            mHierachy.back()->start(true);
        }
    }
    for (size_t i = 0; i < mHierachy.size(); i++)
        if (mHierachy[i] != timer)
            Timer::results.mResults[mHierachy[i]->getName()].mAdditionalClocks += clocks;
    Timer::results.mResults[timer->getName()].mClocks += clocks;
    if (!intermediate)
        Timer::results.mResults[timer->getName()].mNumberOfResults++;
}

Timer::Timer(const std::string& str, Settings::SHOWTIME_MODES showtimeMode)
    : mStr(str)
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
    if (mShowTimeMode == Settings::SHOWTIME_NONE)
        return;
    if (mStopped) {
        mStopped = false;
        if (!intermediate)
            trc.start(this, intermediate);
        mStart = std::clock();
    }
}

void Timer::stop(bool intermediate)
{
    if (mShowTimeMode == Settings::SHOWTIME_NONE)
        return;
    if (!mStopped) {
        const std::clock_t end = std::clock();
        const std::clock_t diff = end - mStart;

        mAccumulated += diff;
        if (mShowTimeMode == Settings::SHOWTIME_FILE) {
        } else
            trc.stop(this, diff, intermediate);
    }

    mStopped = true;
}
