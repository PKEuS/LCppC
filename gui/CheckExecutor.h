#pragma once
#include "precompiled.h"

class ErrorLogger;
class Settings;


class CheckExecutor {
	static ErrorLogger* errorlogger;

public:
	static void setErrorLogger(ErrorLogger* errorLogger) { errorlogger = errorLogger; }
	static void init(Settings& settings);
	static void check(Settings& settings);
	static void check(Settings& settings, const wxString& filename, const wxString& code);
};
