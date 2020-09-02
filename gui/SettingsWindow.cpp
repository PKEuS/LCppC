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

#include "SettingsWindow.h"
#include "../lib/settings.h"

wxBEGIN_EVENT_TABLE(SettingsWindow, wxPropertySheetDialog)
wxEND_EVENT_TABLE()

SettingsWindow::SettingsWindow(wxWindow* parent, Settings& settings)
{
    wxPropertySheetDialog::Create(parent, wxID_ANY, _("Settings"));
    CreateButtons(wxOK | wxCANCEL);
    // Add pages
    {
        wxPanel* panel = new wxPanel(GetBookCtrl());
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
        wxSpinCtrl* numThreads = new wxSpinCtrl(panel);
        numThreads->SetMin(0);
        numThreads->SetMax(0xFFFF);
        numThreads->SetValue(settings.jobs);
        numThreads->SetValidator(wxGenericValidator((int*)&settings.jobs));
        sizer2->Add(new wxStaticText(panel, wxID_ANY, _("Number of threads (0: auto):")), 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 3);
        sizer2->Add(numThreads, 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 2);
        sizer->Add(sizer2, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        panel->SetSizer(sizer);
        GetBookCtrl()->AddPage(panel, "General");
    }
    {
        wxPanel* panel = new wxPanel(GetBookCtrl());
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        wxCheckBox* relPath = new wxCheckBox(panel, wxID_ANY, _("Use relative paths"));
        relPath->SetValue(settings.relativePaths);
        relPath->SetValidator(wxGenericValidator(&settings.relativePaths));
        sizer->Add(relPath, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        wxCheckBox* checkConfig = new wxCheckBox(panel, wxID_ANY, _("Check library configuration"));
        //checkConfig->SetValue(settings.relativePaths);
        //checkConfig->SetValidator(wxGenericValidator(&settings.relativePaths));
        sizer->Add(checkConfig, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        panel->SetSizer(sizer);
        GetBookCtrl()->AddPage(panel, "Output");
    }
    {
        wxPanel* panel = new wxPanel(GetBookCtrl());
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        panel->SetSizer(sizer);
        GetBookCtrl()->AddPage(panel, "Addons");
    }
    {
        wxPanel* panel = new wxPanel(GetBookCtrl());
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        panel->SetSizer(sizer);
        GetBookCtrl()->AddPage(panel, "Debugging");
    }
    LayoutDialog();
}
