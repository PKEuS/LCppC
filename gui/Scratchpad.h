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

class MainWindow;
class CppField;
class Settings;

class Scratchpad : public wxFrame {
public:
    Scratchpad(MainWindow& parent_, const wxPoint& pos, const wxSize& size);

    void fillProject(Project& project) const;
    wxString getText() const;

private:
    MainWindow& parent;
    CppField* code;
    wxCheckBox* severities[5];
    wxCheckBox* certainties[3];
    wxTextCtrl* defines;
    wxTextCtrl* undefines;
    wxComboBox* language;
    wxComboBox* platform;

    void OnCheck(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};
