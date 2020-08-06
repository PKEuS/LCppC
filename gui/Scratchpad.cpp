#include "Scratchpad.h"
#include "MainWindow.h"
#include "CppField.h"
#include "../lib/Settings.h"

enum
{
    ID_CHECK = 50
};

wxBEGIN_EVENT_TABLE(Scratchpad, wxFrame)
EVT_BUTTON(ID_CHECK, Scratchpad::OnCheck)
wxEND_EVENT_TABLE()

static const char* stdStrings[] = { "C++20", "C++17", "C++14", "C++11", "C++03", "C11", "C99", "C89" };
static const wxString platformStrings[] = { "Unix32", "Unix64", "Win32A", "Win32W", "Win64", _("Native"), _("Unspecified") };
static const cppcheck::Platform::PlatformType platforms[] = {
    cppcheck::Platform::Unix32, cppcheck::Platform::Unix64, cppcheck::Platform::Win32A,
    cppcheck::Platform::Win32W, cppcheck::Platform::Win64, cppcheck::Platform::Native, cppcheck::Platform::Unspecified };

Scratchpad::Scratchpad(MainWindow& parent_, const wxPoint& pos, const wxSize& size)
    : wxFrame(&parent_, wxID_ANY, _("Scratchpad"), pos, size)
    , parent(parent_)
{
    SetMinSize(wxSize(400, 400));
    wxPanel* panel = new wxPanel(this);

    code = new CppField(panel, wxID_ANY);
    code->SetText(
        "int main() {\n"
        "\treturn 0;\n"
        "}");

    severities[0] = new wxCheckBox(panel, wxID_ANY, _("Error"));
    severities[1] = new wxCheckBox(panel, wxID_ANY, _("Warning"));
    severities[2] = new wxCheckBox(panel, wxID_ANY, _("Performance"));
    severities[3] = new wxCheckBox(panel, wxID_ANY, _("Portability"));
    severities[4] = new wxCheckBox(panel, wxID_ANY, _("Style"));
    severities[0]->Disable();
    severities[0]->Set3StateValue(wxCHK_CHECKED);
    severities[1]->Set3StateValue(wxCHK_CHECKED);
    severities[2]->Set3StateValue(wxCHK_CHECKED);
    severities[3]->Set3StateValue(wxCHK_CHECKED);
    severities[4]->Set3StateValue(wxCHK_CHECKED);

    certainties[0] = new wxCheckBox(panel, wxID_ANY, _("Safe"));
    certainties[1] = new wxCheckBox(panel, wxID_ANY, _("Inconclusive"));
    certainties[2] = new wxCheckBox(panel, wxID_ANY, _("Experimental"));
    certainties[0]->Disable();
    certainties[0]->Set3StateValue(wxCHK_CHECKED);
    certainties[1]->Set3StateValue(wxCHK_CHECKED);

    defines = new wxTextCtrl(panel, wxID_ANY, "");
    undefines = new wxTextCtrl(panel, wxID_ANY, "");

    language = new wxComboBox(panel, wxID_ANY, "C++20", wxDefaultPosition, wxDefaultSize, wxArrayString(sizeof(stdStrings) / sizeof(*stdStrings), stdStrings));
#ifdef _WIN32
    const char* defaultplatform = "Win64";
#else
    const char* defaultplatform = "Unix64";
#endif
    platform = new wxComboBox(panel, wxID_ANY, defaultplatform, wxDefaultPosition, wxDefaultSize, wxArrayString(sizeof(platformStrings) / sizeof(*platformStrings), platformStrings));

    wxBoxSizer* sizer2 = new wxBoxSizer(wxVERTICAL);
    sizer2->Add(code, 1, wxALL | wxEXPAND, 0);
    wxBoxSizer* sizer3 = new wxBoxSizer(wxVERTICAL);
    sizer3->Add(new wxStaticText(panel, wxID_ANY, _("Severities:")), 0, wxALL | wxEXPAND, 1);
    sizer3->Add(severities[0], 0, wxLEFT | wxRIGHT | wxEXPAND, 1);
    sizer3->Add(severities[1], 0, wxLEFT | wxRIGHT | wxEXPAND, 1);
    sizer3->Add(severities[2], 0, wxLEFT | wxRIGHT | wxEXPAND, 1);
    sizer3->Add(severities[3], 0, wxLEFT | wxRIGHT | wxEXPAND, 1);
    sizer3->Add(severities[4], 0, wxLEFT | wxRIGHT | wxEXPAND, 1);
    sizer3->AddSpacer(3);
    sizer3->Add(new wxStaticText(panel, wxID_ANY, _("Certainty:")), 0, wxALL | wxEXPAND, 1);
    sizer3->Add(certainties[0], 0, wxLEFT | wxRIGHT | wxEXPAND, 1);
    sizer3->Add(certainties[1], 0, wxLEFT | wxRIGHT | wxEXPAND, 1);
    sizer3->Add(certainties[2], 0, wxLEFT | wxRIGHT | wxEXPAND, 1);
    sizer3->AddSpacer(3);
    sizer3->Add(new wxStaticText(panel, wxID_ANY, _("Language:")), 0, wxALL | wxEXPAND, 1);
    sizer3->Add(language, 0, wxLEFT | wxRIGHT | wxEXPAND, 1);
    sizer3->AddSpacer(3);
    sizer3->Add(new wxStaticText(panel, wxID_ANY, _("Defines: (;-separated)")), 0, wxALL | wxEXPAND, 1);
    sizer3->Add(defines, 0, wxALL | wxEXPAND, 1);
    sizer3->Add(new wxStaticText(panel, wxID_ANY, _("Undefines:")), 0, wxALL | wxEXPAND, 1);
    sizer3->Add(undefines, 0, wxALL | wxEXPAND, 1);
    sizer3->AddSpacer(3);
    sizer3->Add(new wxStaticText(panel, wxID_ANY, _("Platform:")), 0, wxALL | wxEXPAND, 1);
    sizer3->Add(platform, 0, wxLEFT | wxRIGHT | wxEXPAND, 1);
    sizer3->AddStretchSpacer();
    sizer3->Add(new wxButton(panel, ID_CHECK, _("Check")), 0, wxALL | wxEXPAND, 0);

    wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
    sizer1->Add(sizer2, 1, wxEXPAND, 0);
    sizer1->Add(new wxStaticLine(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), 0, wxEXPAND, 0);
    sizer1->Add(sizer3, 0, wxEXPAND, 0);

    panel->SetSizerAndFit(sizer1);

    wxAcceleratorEntry entries[1];
    entries[0].Set(wxACCEL_NORMAL, WXK_F5, ID_CHECK);
    wxAcceleratorTable accel(sizeof(entries)/sizeof(*entries), entries);
    this->SetAcceleratorTable(accel);
}

void Scratchpad::OnCheck(wxCommandEvent&)
{
    parent.CheckScratchpad();
}

void Scratchpad::fillSettings(Settings& settings) const
{
    settings.force = true;

    settings.severity.setEnabled(Severity::error, severities[0]->IsChecked());
    settings.severity.setEnabled(Severity::warning, severities[1]->IsChecked());
    settings.severity.setEnabled(Severity::performance, severities[2]->IsChecked());
    settings.severity.setEnabled(Severity::portability, severities[3]->IsChecked());
    settings.severity.setEnabled(Severity::style, severities[4]->IsChecked());

    settings.certainty.setEnabled(Certainty::safe, certainties[0]->IsChecked());
    settings.certainty.setEnabled(Certainty::inconclusive, certainties[1]->IsChecked());
    settings.certainty.setEnabled(Certainty::experimental, certainties[2]->IsChecked());

    settings.userDefines = defines->GetValue().ToStdString();
    wxString str = defines->GetValue();
    for (wxString::size_type startPos = 0U; startPos < str.size();) {
        wxString::size_type endPos;
        if (str[startPos] == '\"') {
            endPos = str.find("\"", startPos + 1);
            if (endPos < str.size())
                endPos++;
        }
        else {
            endPos = str.find(';', startPos + 1);
        }
        if (endPos == wxString::npos) {
            settings.userUndefs.emplace(str.substr(startPos));
            break;
        }
        settings.userUndefs.emplace(str.substr(startPos, endPos - startPos));
        startPos = str.find_first_not_of(";", endPos);
    }
    for (size_t i = 0; i < sizeof(platforms) / sizeof(*platforms); i++)
    {
        if (platformStrings[i] == platform->GetValue())
        {
            settings.platform(platforms[i]);
            break;
        }
    }
    bool cpp = settings.standards.setCPP(language->GetValue().ToStdString());
    if (!cpp)
        settings.standards.setC(language->GetValue().ToStdString());
    settings.enforcedLang = cpp ? Settings::CPP : Settings::C;
}

wxString Scratchpad::getText() const
{
    return code->GetText();
}
