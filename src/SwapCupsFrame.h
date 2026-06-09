#pragma once

#include <wx/wx.h>
#include <array>

class GamePanel;

// Main window: title, status line, game canvas, init radios, swap input and
// the Swap!/Reset buttons.
class SwapCupsFrame : public wxFrame {
public:
    explicit SwapCupsFrame(const wxString& title);

private:
    void OnRadio(int slot);
    void OnSwap();
    void OnReset();
    void OnText(wxCommandEvent& event);
    void SetBusy(bool busy);

    GamePanel* m_panel = nullptr;
    wxStaticText* m_status = nullptr;
    wxTextCtrl* m_swapInput = nullptr;
    wxButton* m_swapBtn = nullptr;
    wxButton* m_resetBtn = nullptr;
    std::array<wxRadioButton*, 3> m_radios{};
};
