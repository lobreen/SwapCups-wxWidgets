#pragma once

#include <wx/wx.h>

// The application object. Creates and shows the main frame on startup.
class SwapCupsApp : public wxApp {
public:
    bool OnInit() override;
};
