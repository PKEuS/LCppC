#pragma once
#include "precompiled.h"
#include "Scratchpad.h"

class CppField;
class UIErrorLogger;


class MainWindow : public wxFrame
{
    wxWeakRef<Scratchpad> scratchpad;
    wxSharedPtr<UIErrorLogger> errorlogger;
    wxTextCtrl* verboseMessage;
    CppField* codeView;
    wxMenuItem* checkItem, *reCheckItem;

public:
    MainWindow(const wxPoint& pos, const wxSize& size);

    void EnableChecking();

    void CheckScratchpad();
    void CheckProject();

private:
    void OnScratchpad(wxCommandEvent& event);
    void OnCheck(wxCommandEvent& event);
    void OnReCheck(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnResultSelect(wxTreeListEvent& event);
    wxDECLARE_EVENT_TABLE();
};
