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

#include "threadexecutor.h"

#include "config.h"
#include "cppcheck.h"
#include "settings.h"
#include "suppressions.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>


ThreadExecutor::ThreadExecutor(std::list<CTU::CTUInfo>& files, Settings& settings, Project& project, ErrorLogger& errorLogger)
    : mCTUs(files), mSettings(settings), mProject(project), mErrorLogger(errorLogger)
{
    mProcessedFiles = 0;
    mTotalFiles = 0;
    mProcessedSize = 0;
    mTotalFileSize = 0;
}

void ThreadExecutor::addFileContent(const std::string &path, const std::string &content)
{
    mFileContents[path] = content;
}

unsigned int ThreadExecutor::checkSync()
{
    mItNextCTU = mCTUs.begin();

    unsigned int jobs = mSettings.jobs;
    if (jobs == 0)
        jobs = std::thread::hardware_concurrency();

    mResult = 0;

    mProcessedFiles = 0;
    mProcessedSize = 0;
    mTotalFiles = mCTUs.size();
    mTotalFileSize = 0;
    for (auto i = mCTUs.begin(); i != mCTUs.end(); ++i) {
        mTotalFileSize += i->filesize;
    }

    if (jobs != 1) {
        std::vector<std::thread> threadHandles;
        threadHandles.reserve(jobs);
        for (unsigned int i = 0; i < jobs; ++i) {
            threadHandles.emplace_back(std::bind(&ThreadExecutor::threadProc, this, false));
        }
        for (unsigned int i = 0; i < jobs; ++i) {
            threadHandles[i].join();
        }

        // Second stage: Some markup files need to be processed after all c/cpp files were checked
        if (!mProject.library.markupExtensions().empty()) {
            mItNextCTU = mCTUs.begin();
            threadHandles.clear();
            for (unsigned int i = 0; i < jobs; ++i) {
                threadHandles.emplace_back(std::bind(&ThreadExecutor::threadProc, this, true));
            }
            for (unsigned int i = 0; i < jobs; ++i) {
                threadHandles[i].join();
            }
        }
    } else {
        threadProc(false);

        // Second stage: Some markup files need to be processed after all c/cpp files were checked
        if (!mProject.library.markupExtensions().empty()) {
            mItNextCTU = mCTUs.begin();
            threadProc(true);
        }
    }

    return mResult;
}

std::thread ThreadExecutor::checkAsync(std::function<void(unsigned int)> callback)
{
    return std::thread([&]() {
        unsigned int retval = checkSync();
        callback(retval);
    });
}

void ThreadExecutor::threadProc(bool markupStage)
{
    CppCheck fileChecker(*this, mSettings, mProject, false);

    while (!mSettings.terminated()) {
        mFileSync.lock();
        if (mItNextCTU == mCTUs.end()) {
            mFileSync.unlock();
            break;
        }

        CTU::CTUInfo* ctu = &*mItNextCTU;
        ++mItNextCTU;

        mFileSync.unlock();

        if (markupStage != mProject.library.processMarkupAfterCode(ctu->sourcefile))
            continue;

        const std::map<std::string, std::string>::const_iterator fileContent = mFileContents.find(ctu->sourcefile);
        if (fileContent != mFileContents.cend()) {
            // File content was given as a string
            mResult += fileChecker.check(ctu, fileContent->second);
        } else {
            // Read file from a file
            mResult += fileChecker.check(ctu);
        }

        if (mSettings.output.isEnabled(Output::progress)) {
            mReportSync.lock();
            mProcessedSize += ctu->filesize;
            mProcessedFiles++;
            mErrorLogger.reportStatus(mProcessedFiles, mTotalFiles, mProcessedSize, mTotalFileSize);
            mReportSync.unlock();
        }

    }
}

void ThreadExecutor::reportOut(const std::string &outmsg)
{
    mReportSync.lock();

    mErrorLogger.reportOut(outmsg);

    mReportSync.unlock();
}

void ThreadExecutor::reportErr(const ErrorMessage &msg)
{
    if (mProject.nomsg.isSuppressed(msg.toSuppressionsErrorMessage()))
        return;

    // Alert only about unique errors
    bool reportError = false;
    const std::string errmsg = msg.toString(mSettings.verbose);

    mErrorSync.lock();
    if (std::find(mErrorList.begin(), mErrorList.end(), errmsg) == mErrorList.end()) {
        mErrorList.emplace_back(errmsg);
        reportError = true;
    }
    mErrorSync.unlock();

    if (reportError) {
        mReportSync.lock();

        mErrorLogger.reportErr(msg);

        mReportSync.unlock();
    }
}
