#include "MainWindow.h"
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

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
    EVT_MENU(ID_SCRATCHPAD, MainWindow::OnScratchpad)
    EVT_MENU(ID_CHECK, MainWindow::OnCheck)
    EVT_MENU(ID_RECHECK, MainWindow::OnReCheck)
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
                     _("Create a new project file, using a wizard to configure it."));
    menuFile->Append(ID_OPEN_PROJECT,
                     _("&Open Project...\tCtrl-O"),
                     _("Open an existing project file."));
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
                      _("&Check directory..."),
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
    CreateStatusBar();
    SetStatusText(_("Welcome to LCppC GUI! Open or create a project file or use the scratchpad to continue..."));

    wxPanel* panel = new wxPanel(this);

    errorlogger = new UIErrorLogger(panel, ID_RESULTSTREE);
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

    Settings settings;
    scratchpad->fillSettings(settings);
    CheckExecutor::init(settings);
    CheckExecutor::check(settings, settings.enforcedLang == Settings::CPP ? "scratch.cpp" : "scratch.c", scratchpad->getText());

    codeView->SetEditable(true);
    codeView->SetText(scratchpad->getText());
    codeView->SetEditable(false);
}

void MainWindow::CheckProject()
{
}
