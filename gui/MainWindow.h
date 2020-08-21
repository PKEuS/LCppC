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
#include "Scratchpad.h"

class CppField;
class UIErrorLogger;


class MainWindow : public wxFrame {
    wxWeakRef<Scratchpad> scratchpad;
    wxSharedPtr<UIErrorLogger> errorlogger;
    wxTextCtrl* verboseMessage;
    CppField* codeView;
    wxMenuItem* checkItem, *reCheckItem;
    wxGauge* progressBar;

public:
    MainWindow(const wxPoint& pos, const wxSize& size);

    void EnableChecking();

    void CheckScratchpad();
    void CheckProject();

private:
    void OnScratchpad(wxCommandEvent& event);
    void OnCheckDirectory(wxCommandEvent& event);
    void OnCheck(wxCommandEvent& event);
    void OnReCheck(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnResultSelect(wxTreeListEvent& event);
    wxDECLARE_EVENT_TABLE();
};
