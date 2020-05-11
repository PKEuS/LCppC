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

#include "threadexecutor.h"

#include "config.h"
#include "cppcheck.h"
#include "cppcheckexecutor.h"
#include "settings.h"
#include "suppressions.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <thread>
#include <future>
#include <functional>


ThreadExecutor::ThreadExecutor(const std::map<std::string, std::size_t> &files, Settings &settings, ErrorLogger &errorLogger)
    : mFiles(files), mSettings(settings), mErrorLogger(errorLogger)
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

unsigned int ThreadExecutor::check()
{
    mItNextFile = mFiles.begin();

    mResult = 0;

    mProcessedFiles = 0;
    mProcessedSize = 0;
    mTotalFiles = mFiles.size();
    mTotalFileSize = 0;
    for (std::map<std::string, std::size_t>::const_iterator i = mFiles.begin(); i != mFiles.end(); ++i) {
        mTotalFileSize += i->second;
    }

    std::vector<std::thread> threadHandles;
    threadHandles.reserve(mSettings.jobs);
    for (unsigned int i = 0; i < mSettings.jobs; ++i) {
        threadHandles.emplace_back(std::bind(&ThreadExecutor::threadProc, this));
    }

    for (unsigned int i = 0; i < mSettings.jobs; ++i) {
        threadHandles[i].join();
    }

    return mResult;
}

void ThreadExecutor::threadProc()
{
    Settings settings = mSettings; // Create one copy per thread to avoid side effects across threads. Might be unnecessary.
    CppCheck fileChecker(*this, settings, false, CppCheckExecutor::executeCommand);

    for (;;) {
        mFileSync.lock();
        if (mItNextFile == mFiles.end()) {
            mFileSync.unlock();
            break;
        }

        std::size_t fileSize = 0;
        const std::string &file = mItNextFile->first;
        fileSize = mItNextFile->second;
        ++mItNextFile;

        mFileSync.unlock();

        const std::map<std::string, std::string>::const_iterator fileContent = mFileContents.find(file);
        if (fileContent != mFileContents.end()) {
            // File content was given as a string
            mResult += fileChecker.check(file, fileContent->second);
        } else {
            // Read file from a file
            mResult += fileChecker.check(file);
        }

        if (mSettings.output.isEnabled(Output::progress)) {
            mReportSync.lock();
            mProcessedSize += fileSize;
            mProcessedFiles++;
            CppCheckExecutor::reportStatus(mProcessedFiles, mTotalFiles, mProcessedSize, mTotalFileSize);
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
    report(msg, MessageType::REPORT_ERROR);
}

void ThreadExecutor::reportInfo(const ErrorMessage &msg)
{
    (void)msg;
}

void ThreadExecutor::report(const ErrorMessage &msg, MessageType msgType)
{
    if (mSettings.nomsg.isSuppressed(msg.toSuppressionsErrorMessage()))
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

        switch (msgType) {
        case MessageType::REPORT_ERROR:
            mErrorLogger.reportErr(msg);
            break;
        case MessageType::REPORT_INFO:
            mErrorLogger.reportInfo(msg);
            break;
        }

        mReportSync.unlock();
    }
}
