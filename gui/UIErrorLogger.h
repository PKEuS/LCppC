#pragma once
#include "precompiled.h"
#include "../lib/errorlogger.h"

class UIErrorMessage : public wxTreeItemData, public ErrorMessage {
public:
    UIErrorMessage() = default;
    UIErrorMessage(const UIErrorMessage&) = default;
    UIErrorMessage(const ErrorMessage& msg) : ErrorMessage(msg) {};
};

class UIErrorLogger : public ErrorLogger {
    wxTreeListCtrl* control;
    wxSharedPtr<wxGauge> progress;
public:
    UIErrorLogger(wxWindow* parent, wxWindowID id, wxGauge* progress_);

    wxTreeListCtrl* getResultsTree() {
        return control;
    }
    void clear();

    void reportOut(const std::string& outmsg) final;
    void reportErr(const ErrorMessage& msg) final;
    void reportStatus(std::size_t fileindex, std::size_t filecount, std::size_t sizedone, std::size_t sizetotal) final;
};
