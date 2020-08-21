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

#include "UIErrorLogger.h"


UIErrorLogger::UIErrorLogger(wxWindow* parent, wxWindowID id, wxGauge* progress_)
    : control(new wxTreeListCtrl(parent, id)), progress(progress_)
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

void UIErrorLogger::reportStatus(std::size_t fileindex, std::size_t filecount, std::size_t sizedone, std::size_t sizetotal)
{
    progress->SetRange(sizetotal);
    progress->SetValue(sizedone);
}
