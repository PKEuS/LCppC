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

#include "precompiled.h"
#include "MainWindow.h"
#include "CheckExecutor.h"


class LCppC_GUI : public wxApp {
public:
    bool OnInit() final {
        MainWindow* frame = new MainWindow(wxDefaultPosition, wxSize(800, 600));
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(LCppC_GUI);
