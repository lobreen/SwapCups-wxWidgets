#include "SwapCupsApp.h"
#include "SwapCupsFrame.h"

bool SwapCupsApp::OnInit() {
    auto* frame = new SwapCupsFrame("Swap the Cups");
    frame->Show(true);
    return true;
}

wxIMPLEMENT_APP(SwapCupsApp);
