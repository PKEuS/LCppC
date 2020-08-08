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
