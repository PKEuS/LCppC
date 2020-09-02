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

#include "MainWindow.h"
#include "SettingsWindow.h"
#include "CheckExecutor.h"
#include "CppField.h"
#include "UIErrorLogger.h"
#include "../lib/version.h"
#include "../lib/settings.h"
#include "../lib/errorlogger.h"

enum {
    ID_NEW_PROJECT = 2, ID_OPEN_PROJECT, ID_SAVE_PROJECT, ID_SAVE_PROJECT_AS,
    ID_CHECK, ID_RECHECK, ID_CHECK_DIRECTORY, ID_SCRATCHPAD,
    ID_PROJECT_SETTINGS, ID_LCPPC_SETTINGS,
    ID_RESULTSTREE
};

class StatusProgressBar : public wxStatusBar {
public:
    wxGauge* progressBar;

    StatusProgressBar(wxWindow* parent)
        : wxStatusBar(parent, wxID_ANY, wxSTB_ELLIPSIZE_END | wxSTB_SHOW_TIPS | wxFULL_REPAINT_ON_RESIZE) {
        progressBar = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(200, GetSize().GetHeight() - 6));
        int widths[2] = { -1, 200 };
        SetFieldsCount(2, widths);
    }

    void OnSize(wxSizeEvent& w) {
        wxRect rect;
        GetFieldRect(1, rect);
        progressBar->Move(rect.x, rect.y);
        progressBar->SetSize(rect.width, rect.height);
        w.Skip();
    }

    wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(StatusProgressBar, wxStatusBar)
    EVT_SIZE(StatusProgressBar::OnSize)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
    EVT_MENU(ID_SCRATCHPAD, MainWindow::OnScratchpad)
    EVT_MENU(ID_CHECK_DIRECTORY, MainWindow::OnCheckDirectory)
    EVT_MENU(ID_CHECK, MainWindow::OnCheck)
    EVT_MENU(ID_RECHECK, MainWindow::OnReCheck)
    EVT_MENU(ID_LCPPC_SETTINGS, MainWindow::OnLCppCSettings)
    EVT_MENU(wxID_EXIT, MainWindow::OnExit)
    EVT_MENU(wxID_ABOUT, MainWindow::OnAbout)
    EVT_TREELIST_SELECTION_CHANGED(ID_RESULTSTREE, MainWindow::OnResultSelect)
wxEND_EVENT_TABLE()

MainWindow::MainWindow(const wxPoint& pos, const wxSize& size)
    : wxFrame(NULL, wxID_ANY, PROGRAMNAME, pos, size)
{
    SetMinSize(wxSize(400, 300));

    wxMenu* menuFile = new wxMenu;
    menuFile->Append(ID_NEW_PROJECT,
                     _("&New Project...\tCtrl-N"),
                     _("Create a new project file, using a wizard to configure it."))->Enable(false);
    menuFile->Append(ID_OPEN_PROJECT,
                     _("&Open Project...\tCtrl-O"),
                     _("Open an existing project file."))->Enable(false);
    menuFile->Append(ID_SAVE_PROJECT,
                     _("&Save Project\tCtrl-S"),
                     _("Save project file."))->Enable(false);
    menuFile->Append(ID_SAVE_PROJECT_AS,
                     _("&Save Project As..."),
                     _("Select file name when saving the project file."))->Enable(false);
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    wxMenu* menuCheck = new wxMenu;
    checkItem = menuCheck->Append(ID_CHECK,
                                  _("&Check\tF5"),
                                  _("(Incremental) check of given source code."));
    checkItem->Enable(false);
    reCheckItem = menuCheck->Append(ID_RECHECK,
                                    _("&Full Re-Check\tCtrl-F5"),
                                    _("Full check of given source code."));
    reCheckItem->Enable(false);
    menuCheck->AppendSeparator();
    menuCheck->Append(ID_CHECK_DIRECTORY,
                      _("&Check directory...\tF9"),
                      _("Check a directory directly without a project file."));
    menuCheck->Append(ID_SCRATCHPAD,
                      _("&Open Scratchpad...\tF8"),
                      _("Scratchpad can be used to enter code directly and view the results."));
    wxMenu* menuConfigure = new wxMenu;
    menuConfigure->Append(ID_PROJECT_SETTINGS,
                          _("&Project settings..."),
                          _("Configure open project file."))->Enable(false);
    menuConfigure->Append(ID_LCPPC_SETTINGS,
                          _("&LCppC settings..."),
                          _("Configure LCppC GUI."));
    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, _("&File"));
    menuBar->Append(menuCheck, _("&Check"));
    menuBar->Append(menuConfigure, _("&Configure"));
    menuBar->Append(menuHelp, _("&Help"));
    SetMenuBar(menuBar);

    StatusProgressBar* statusbar = new StatusProgressBar(this);
    progressBar = statusbar->progressBar;
    SetStatusBar(statusbar);
    SetStatusText(_("Welcome to LCppC GUI! Open or create a project file or use the scratchpad to continue..."));

    wxPanel* panel = new wxPanel(this);

    errorlogger = new UIErrorLogger(panel, ID_RESULTSTREE, progressBar);
    CheckExecutor::setErrorLogger(errorlogger.get());
    verboseMessage = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_RICH | wxTE_READONLY | wxNO_BORDER | wxTE_BESTWRAP);
    codeView = new CppField(panel, wxID_ANY);
    codeView->SetEditable(false);

    wxBoxSizer* sizer2 = new wxBoxSizer(wxVERTICAL);
    sizer2->Add(errorlogger->getResultsTree(), 4, wxALL | wxEXPAND, 0);
    sizer2->Add(new wxStaticLine(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), 0, wxDOWN | wxEXPAND, 1);
    sizer2->Add(verboseMessage, 1, wxRIGHT | wxLEFT | wxEXPAND, 3);
    sizer2->Add(new wxStaticLine(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), 0, wxUP | wxDOWN | wxEXPAND, 1);
    sizer2->Add(codeView, 2, wxRIGHT | wxLEFT | wxEXPAND, 3);

    panel->SetSizerAndFit(sizer2);
}
void MainWindow::OnCheck(wxCommandEvent&)
{
    if (scratchpad)
        CheckScratchpad();
    else
        CheckProject();
}
void MainWindow::OnReCheck(wxCommandEvent& event)
{
    OnCheck(event);
}
void MainWindow::OnExit(wxCommandEvent&)
{
    Close(true);
}
void MainWindow::OnLCppCSettings(wxCommandEvent&)
{
    SettingsWindow dialog(this, CheckExecutor::settings);
    dialog.ShowModal();
}
void MainWindow::OnAbout(wxCommandEvent&)
{
    wxAboutDialogInfo info;
    info.SetName(PROGRAMNAME);
    info.SetVersion(CPPCHECK_VERSION_STRING);
    info.SetDescription(_("Graphical user interface to LCppC, a static code analysis tool for C and C++ code."));
    info.SetCopyright(wxT(LEGALCOPYRIGHT));
    wxAboutBox(info);
}
void MainWindow::OnScratchpad(wxCommandEvent&)
{
    SetStatusText(_("Enter code snippet to the Scratchpad, then press F5 to check it..."));
    EnableChecking();
    if (!scratchpad)
        scratchpad = new Scratchpad(*this, wxDefaultPosition, wxSize(600, 450));
    scratchpad->Show();
}
void MainWindow::OnCheckDirectory(wxCommandEvent&)
{
    wxDirDialog dlg(this, _("Choose directory"), wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK) {
    }
}
void MainWindow::OnResultSelect(wxTreeListEvent& event)
{
    const UIErrorMessage* msg = dynamic_cast<const UIErrorMessage*>(errorlogger->getResultsTree()->GetItemData(event.GetItem()));
    if (msg) {
        verboseMessage->SetValue(msg->verboseMessage());
        if (msg->callStack.empty())
            codeView->SetFirstVisibleLine(0);
        else
            codeView->SetFirstVisibleLine(std::max(msg->callStack.back().line-3, 0));
    } else
        verboseMessage->SetValue("");
}

void MainWindow::EnableChecking()
{
    checkItem->Enable();
    reCheckItem->Enable();
}

void MainWindow::CheckScratchpad()
{
    verboseMessage->SetValue("");
    errorlogger->clear();

    if (!scratchpad)
        return;

    Project project;
    scratchpad->fillProject(project);
    CheckExecutor::init(project);
    CheckExecutor::check(project, project.enforcedLang == Project::CPP ? "scratch.cpp" : "scratch.c", scratchpad->getText());

    codeView->SetEditable(true);
    codeView->SetText(scratchpad->getText());
    codeView->SetEditable(false);
}

void MainWindow::CheckProject()
{
}
