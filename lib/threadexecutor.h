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

#ifndef THREADEXECUTOR_H
#define THREADEXECUTOR_H

#include "config.h"
#include "errorlogger.h"
#include "ctu.h"

#include <cstddef>
#include <list>
#include <map>
#include <string>
#include <mutex>
#include <atomic>


class Settings;

/// @addtogroup CLI
/// @{

/**
 * This class will take a list of filenames and settings and check then
 * all files using threads.
 */
class CPPCHECKLIB ThreadExecutor : public ErrorLogger {
public:
    ThreadExecutor(std::list<CTU::CTUInfo>& files, Settings &settings, ErrorLogger &errorLogger);
    ThreadExecutor(const ThreadExecutor &) = delete;
    void operator=(const ThreadExecutor &) = delete;
    unsigned int check();

    void reportOut(const std::string &outmsg) override;
    void reportErr(const ErrorMessage &msg) override;

    /**
     * @brief Add content to a file, to be used in unit testing.
     *
     * @param path File name (used as a key to link with real file).
     * @param content If the file would be a real file, this should be
     * the content of the file.
     */
    void addFileContent(const std::string &path, const std::string &content);

private:
    std::list<CTU::CTUInfo>& mCTUs;
    Settings &mSettings;
    ErrorLogger &mErrorLogger;

private:
    std::map<std::string, std::string> mFileContents;
    std::list<CTU::CTUInfo>::iterator mItNextCTU;
    std::mutex mFileSync;

    std::list<std::string> mErrorList;
    std::mutex mErrorSync;

    std::mutex mReportSync;

    std::atomic<std::size_t> mProcessedFiles;
    std::atomic<std::size_t> mTotalFiles;
    std::atomic<std::size_t> mProcessedSize;
    std::atomic<std::size_t> mTotalFileSize;
    std::atomic<unsigned int> mResult;

    void threadProc(bool markupStage);
};

/// @}

#endif // THREADEXECUTOR_H
