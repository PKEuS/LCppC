#pragma once
#include "precompiled.h"
#include "../lib/errorlogger.h"

class UIErrorMessage : public wxTreeItemData, public ErrorMessage
{
public:
	UIErrorMessage() = default;
	UIErrorMessage(const UIErrorMessage&) = default;
	UIErrorMessage(const ErrorMessage& msg) : ErrorMessage(msg) {};
};

class UIErrorLogger : public ErrorLogger
{
	wxTreeListCtrl* control;
public:
	UIErrorLogger(wxWindow* parent, wxWindowID id);

	wxTreeListCtrl* getResultsTree() { return control; }
	void clear();

	void reportOut(const std::string& outmsg) final;
	void reportErr(const ErrorMessage& msg) final;
	void reportProgress(const std::string& filename, const char stage[], const std::size_t value) final;
	void reportInfo(const ErrorMessage& msg) final;
};
