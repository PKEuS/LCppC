/*
 * LCppC - A tool for static C/C++ code analysis
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

#pragma once
#include "precompiled.h"
#include "../lib/errorlogger.h"

class UIErrorMessage : public wxTreeItemData, public ErrorMessage {
public:
    UIErrorMessage() = default;
    UIErrorMessage(const UIErrorMessage&) = default;
    UIErrorMessage(const ErrorMessage& msg) : ErrorMessage(msg) {};
};

class UIErrorLogger : public ErrorLogger {
    wxTreeListCtrl* control;
    wxSharedPtr<wxGauge> progress;
public:
    UIErrorLogger(wxWindow* parent, wxWindowID id, wxGauge* progress_);

    wxTreeListCtrl* getResultsTree() {
        return control;
    }
    void clear();

    void reportOut(const std::string& outmsg) final;
    void reportErr(const ErrorMessage& msg) final;
    void reportStatus(std::size_t fileindex, std::size_t filecount, std::size_t sizedone, std::size_t sizetotal) final;
};
