#include "SwapCupsFrame.h"
#include "GamePanel.h"

#include <algorithm>

SwapCupsFrame::SwapCupsFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(560, 660)) {
    SetBackgroundColour(*wxWHITE);

    auto* root = new wxBoxSizer(wxVERTICAL);

    auto* titleText = new wxStaticText(this, wxID_ANY, "Swap the Cups");
    titleText->SetFont(wxFont(wxFontInfo(22).Bold()));
    root->Add(titleText, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, 18);

    m_status = new wxStaticText(this, wxID_ANY,
                                "Pick a cup to hide the ball, then enter swaps.",
                                wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
    m_status->SetForegroundColour(wxColour(110, 110, 115));
    root->Add(m_status, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxRIGHT | wxTOP, 8);

    m_panel = new GamePanel(this);
    m_panel->SetMinSize(wxSize(520, 260));
    root->Add(m_panel, 1, wxEXPAND | wxALL, 16);

    // Initialization radios: Left / Middle / Right (none selected at first).
    auto* radioRow = new wxBoxSizer(wxHORIZONTAL);
    const char* labels[3] = {"Left", "Middle", "Right"};
    for (int i = 0; i < 3; ++i) {
        const long style = (i == 0) ? wxRB_GROUP : 0;
        m_radios[i] = new wxRadioButton(this, wxID_ANY, labels[i],
                                        wxDefaultPosition, wxDefaultSize, style);
        m_radios[i]->SetValue(false);
        m_radios[i]->Bind(wxEVT_RADIOBUTTON, [this, i](wxCommandEvent&) { OnRadio(i); });
        radioRow->Add(m_radios[i], 0, wxALL, 12);
    }
    root->Add(radioRow, 0, wxALIGN_CENTER_HORIZONTAL);

    auto* caption = new wxStaticText(this, wxID_ANY, wxString::FromUTF8(
        "Swaps  (A = left\xe2\x86\x94middle,  B = middle\xe2\x86\x94right,  C = left\xe2\x86\x94right)"));
    caption->SetForegroundColour(wxColour(130, 130, 135));
    root->Add(caption, 0, wxLEFT | wxRIGHT | wxTOP, 16);

    m_swapInput = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
                                 wxTE_PROCESS_ENTER);
    m_swapInput->SetHint("e.g. ACCABCBAACABCBACBA");
    m_swapInput->Bind(wxEVT_TEXT, &SwapCupsFrame::OnText, this);
    m_swapInput->Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent&) { OnSwap(); });
    root->Add(m_swapInput, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 16);

    auto* btnRow = new wxBoxSizer(wxHORIZONTAL);
    m_swapBtn = new wxButton(this, wxID_ANY, "Swap!");
    m_swapBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnSwap(); });
    m_resetBtn = new wxButton(this, wxID_ANY, "Reset");
    m_resetBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnReset(); });
    btnRow->Add(m_swapBtn, 0, wxRIGHT, 12);
    btnRow->Add(m_resetBtn, 0);
    root->Add(btnRow, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 18);

    SetSizer(root);
    SetMinSize(wxSize(560, 660));
    Centre();

    m_panel->SetStatusCallback([this](const wxString& s) {
        m_status->SetLabel(s);
        Layout();
    });
    m_panel->SetBusyCallback([this](bool busy) { SetBusy(busy); });
}

void SwapCupsFrame::OnRadio(int slot) {
    m_panel->Initialize(slot);
}

void SwapCupsFrame::OnSwap() {
    m_panel->RunSwaps(m_swapInput->GetValue());
}

void SwapCupsFrame::OnReset() {
    m_panel->ResetGame();
    m_swapInput->ChangeValue("");
    for (auto* r : m_radios) r->SetValue(false);
}

void SwapCupsFrame::OnText(wxCommandEvent&) {
    const wxString value = m_swapInput->GetValue();
    wxString cleaned;
    for (wxUniChar ch : value) {
        const wxUniChar up = wxToupper(ch);
        if (up == 'A' || up == 'B' || up == 'C') cleaned += up;
    }
    if (cleaned != value) {
        const long ip = m_swapInput->GetInsertionPoint();
        m_swapInput->ChangeValue(cleaned);  // ChangeValue() does not re-fire wxEVT_TEXT
        m_swapInput->SetInsertionPoint(std::min<long>(ip, cleaned.length()));
    }
}

void SwapCupsFrame::SetBusy(bool busy) {
    for (auto* r : m_radios) r->Enable(!busy);
    m_swapInput->Enable(!busy);
    m_swapBtn->Enable(!busy);
    m_resetBtn->Enable(!busy);
}
