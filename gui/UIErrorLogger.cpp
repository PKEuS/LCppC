#include "UIErrorLogger.h"


UIErrorLogger::UIErrorLogger(wxWindow* parent, wxWindowID id)
    : control(new wxTreeListCtrl(parent, id))
{
    control->AppendColumn(_("File"), wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
    control->AppendColumn(_("Line"), wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxCOL_RESIZABLE);
    control->AppendColumn(_("Severity"), wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
    control->AppendColumn(_("Certainty"), wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
    control->AppendColumn(_("Short Message"), wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxCOL_RESIZABLE | wxCOL_SORTABLE);

}

void UIErrorLogger::clear()
{
    control->DeleteAllItems();
}

void UIErrorLogger::reportOut(const std::string& outmsg)
{
}

void UIErrorLogger::reportErr(const ErrorMessage& msg)
{
    wxTreeListItem item;
    if (!msg.callStack.empty()) {
        item = control->AppendItem(control->GetRootItem(), msg.callStack.back().getFile());
        control->SetItemText(item, 1, std::to_string(msg.callStack.back().line));
    } else
        item = control->AppendItem(control->GetRootItem(), "-");
    control->SetItemText(item, 2, Severity::toString(msg.severity));
    control->SetItemText(item, 3, Certainty::toString(msg.certainty));
    control->SetItemText(item, 4, msg.shortMessage());
    control->SetItemData(item, new UIErrorMessage(msg));
}

void UIErrorLogger::reportProgress(const std::string& filename, const char stage[], const std::size_t value)
{
}

void UIErrorLogger::reportInfo(const ErrorMessage& msg)
{
}
