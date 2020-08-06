#pragma once
#include "precompiled.h"

class MainWindow;
class CppField;
class Settings;

class Scratchpad : public wxFrame
{
public:
    Scratchpad(MainWindow& parent_, const wxPoint& pos, const wxSize& size);

    void fillSettings(Settings& settings) const;
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
