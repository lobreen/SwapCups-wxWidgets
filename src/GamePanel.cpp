#include "GamePanel.h"

#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <algorithm>
#include <cmath>

namespace {
constexpr double kCupWidth = 96;
constexpr double kCupHeight = 120;
constexpr double kSlotSpacing = 130;
constexpr double kLiftHeight = 70;
constexpr double kBallRadius = 23;
constexpr double kBallDrop = 36;     // ball centre below table centre
constexpr double kDotRise = 88;      // dot centre above table centre
constexpr double kDotRadius = 9;
constexpr double kTableHeight = 230;

double lerp(double a, double b, double t) { return a + (b - a) * t; }
double easeOut(double t) { return 1.0 - (1.0 - t) * (1.0 - t); }
double easeIn(double t) { return t * t; }
double easeInOut(double t) {
    return t < 0.5 ? 2 * t * t : 1.0 - std::pow(-2 * t + 2, 2) / 2.0;
}

// The two slots a swap exchanges.
void slotsFor(wxChar c, int& a, int& b) {
    switch (c) {
        case 'A': a = 0; b = 1; break;  // left  <-> middle
        case 'B': a = 1; b = 2; break;  // middle <-> right
        default:  a = 0; b = 2; break;  // 'C': left <-> right
    }
}
}  // namespace

GamePanel::GamePanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);  // for wxAutoBufferedPaintDC
    SetBackgroundColour(*wxWHITE);
    m_timer.SetOwner(this);
    Bind(wxEVT_PAINT, &GamePanel::OnPaint, this);
    Bind(wxEVT_TIMER, &GamePanel::OnTimer, this);
    Bind(wxEVT_LEFT_DOWN, &GamePanel::OnLeftDown, this);
    Bind(wxEVT_SIZE, &GamePanel::OnSize, this);
}

void GamePanel::OnSize(wxSizeEvent& event) {
    m_fullRedraw = true;   // geometry moved; repaint the whole canvas next time
    Refresh();
    event.Skip();
}

wxRect GamePanel::CupBox(double slotPos, double lift) const {
    const double cx = SlotX(slotPos);
    const double cy = GetClientSize().GetHeight() / 2.0;
    const double top = cy - kCupHeight / 2.0 - lift;
    return wxRect(static_cast<int>(std::floor(cx - kCupWidth / 2.0 - 3)),
                  static_cast<int>(std::floor(top - 8)),
                  static_cast<int>(std::ceil(kCupWidth + 6)),
                  static_cast<int>(std::ceil(kCupHeight + lift + 14)));
}

wxRect GamePanel::BallBox(double slotPos) const {
    const double cx = SlotX(slotPos);
    const double by = GetClientSize().GetHeight() / 2.0 + kBallDrop;
    const int r = static_cast<int>(kBallRadius) + 2;
    return wxRect(static_cast<int>(cx) - r, static_cast<int>(by) - r, 2 * r, 2 * r);
}

wxRect GamePanel::DotBox(int slot) const {
    const double cx = SlotX(slot);
    const double dy = GetClientSize().GetHeight() / 2.0 - kDotRise;
    const int r = static_cast<int>(kDotRadius) + 2;
    return wxRect(static_cast<int>(cx) - r, static_cast<int>(dy) - r, 2 * r, 2 * r);
}

double GamePanel::SlotX(double slot) const {
    return GetClientSize().GetWidth() / 2.0 + (slot - 1.0) * kSlotSpacing;
}

int GamePanel::SlotOf(int cupId) const {
    for (int slot = 0; slot < kCupCount; ++slot)
        if (m_positions[slot] == cupId) return slot;
    return cupId;
}

// MARK: - Painting

void GamePanel::OnPaint(wxPaintEvent&) {
    const wxSize client = GetClientSize();
    if (client.GetWidth() <= 0 || client.GetHeight() <= 0) {
        wxPaintDC dc(this);
        return;
    }

    if (!m_backing.IsOk() || m_backing.GetSize() != client) {
        m_backing = wxBitmap(client.GetWidth(), client.GetHeight(), 32);
        m_fullRedraw = true;
    }

    // Repaint only the invalidated region (the dirty rect from OnTimer), unless a
    // full redraw is needed (first paint, resize). The backing bitmap keeps every
    // pixel outside the region, so we never re-render the whole canvas per frame.
    wxRect box = GetUpdateRegion().GetBox();
    if (m_fullRedraw || box.IsEmpty()) box = wxRect(0, 0, client.GetWidth(), client.GetHeight());
    m_fullRedraw = false;

    wxMemoryDC mdc(m_backing);
    if (wxGraphicsContext* gc = wxGraphicsContext::Create(mdc)) {
        gc->Clip(box.x, box.y, box.width, box.height);
        DrawScene(gc, client);
        delete gc;   // flush drawing into the backing bitmap
    }

    wxPaintDC pdc(this);
    pdc.Blit(box.x, box.y, box.width, box.height, &mdc, box.x, box.y);
}

void GamePanel::DrawScene(wxGraphicsContext* gc, const wxSize& client) {
    const double cy = client.GetHeight() / 2.0;

    // Background (only the clipped region is actually rasterized).
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->SetBrush(wxBrush(GetBackgroundColour()));
    gc->DrawRectangle(0, 0, client.GetWidth(), client.GetHeight());

    // Table surface.
    gc->SetPen(*wxTRANSPARENT_PEN);
    wxGraphicsBrush table = gc->CreateLinearGradientBrush(
        0, cy - kTableHeight / 2, 0, cy + kTableHeight / 2,
        wxColour(82, 51, 30), wxColour(51, 31, 18));
    gc->SetBrush(table);
    gc->DrawRoundedRectangle(16, cy - kTableHeight / 2,
                             client.GetWidth() - 32, kTableHeight, 16);

    // Ball, drawn under its cup (revealed when that cup lifts).
    if (m_ballCup >= 0 && m_ballOpacity > 0.01) {
        const double bx = SlotX(m_cupSlotPos[m_ballCup]);
        const double by = cy + kBallDrop;
        const int a = static_cast<int>(std::round(std::clamp(m_ballOpacity, 0.0, 1.0) * 255));
        gc->SetBrush(wxBrush(wxColour(26, 89, 217, a)));
        gc->DrawEllipse(bx - kBallRadius, by - kBallRadius, kBallRadius * 2, kBallRadius * 2);
        gc->SetBrush(wxBrush(wxColour(150, 200, 255, static_cast<int>(a * 0.8))));
        gc->DrawEllipse(bx - kBallRadius * 0.45, by - kBallRadius * 0.55,
                        kBallRadius * 0.8, kBallRadius * 0.8);
    }

    // Cups, at their current (animated) slot positions and lift heights.
    for (int id = 0; id < kCupCount; ++id) {
        const double cx = SlotX(m_cupSlotPos[id]);
        const double left = cx - kCupWidth / 2.0;
        const double top = cy - kCupHeight / 2.0 - m_cupLift[id];
        const double inset = kCupWidth * 0.13;

        wxGraphicsPath path = gc->CreatePath();
        path.MoveToPoint(left, top + kCupHeight);
        path.AddLineToPoint(left + inset, top + 12);
        path.AddQuadCurveToPoint(left + kCupWidth / 2.0, top - 6,
                                 left + kCupWidth - inset, top + 12);
        path.AddLineToPoint(left + kCupWidth, top + kCupHeight);
        path.CloseSubpath();

        wxGraphicsBrush cup = gc->CreateLinearGradientBrush(
            left, top, left + kCupWidth, top + kCupHeight,
            wxColour(219, 61, 51), wxColour(153, 26, 20));
        gc->SetBrush(cup);
        gc->SetPen(wxPen(wxColour(0, 0, 0, 64), 1));
        gc->DrawPath(path);
    }

    // Flashing dots above the two cups about to swap.
    if (m_dotsActive) {
        const double dy = cy - kDotRise;
        const int a = static_cast<int>(std::round(std::clamp(m_dotOpacity, 0.0, 1.0) * 255));
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->SetBrush(wxBrush(wxColour(60, 200, 80, a)));
        const double gx = SlotX(m_dotSlotA);
        gc->DrawEllipse(gx - kDotRadius, dy - kDotRadius, kDotRadius * 2, kDotRadius * 2);
        gc->SetBrush(wxBrush(wxColour(50, 120, 235, a)));
        const double bx = SlotX(m_dotSlotB);
        gc->DrawEllipse(bx - kDotRadius, dy - kDotRadius, kDotRadius * 2, kDotRadius * 2);
    }
}

// MARK: - Scheduler

void GamePanel::StartSequence(std::function<void()> onComplete) {
    m_onComplete = std::move(onComplete);
    m_busy = true;
    if (m_busyCb) m_busyCb(true);
    m_stepIndex = 0;
    m_stepElapsed = 0.0;
    if (!m_steps.empty() && m_steps[0].onStart) m_steps[0].onStart();
    m_stopwatch.Start();
    m_timer.Start(16);
    Refresh();
}

void GamePanel::StopAnim() {
    m_timer.Stop();
    m_busy = false;
    if (m_onComplete) {
        auto cb = m_onComplete;
        m_onComplete = nullptr;
        cb();
    }
    if (m_busyCb) m_busyCb(false);
    m_steps.clear();
    Refresh();
}

void GamePanel::OnTimer(wxTimerEvent&) {
    const double dt = m_stopwatch.Time() / 1000.0;
    m_stopwatch.Start();
    m_stepElapsed += dt;

    // Snapshot the drawn state so we can repaint only what actually moved.
    const auto prevSlot = m_cupSlotPos;
    const auto prevLift = m_cupLift;
    const double prevBallOp = m_ballOpacity;
    const double prevBallSlot = (m_ballCup >= 0) ? m_cupSlotPos[m_ballCup] : 0.0;
    const bool prevDots = m_dotsActive;
    const double prevDotOp = m_dotOpacity;

    while (m_stepIndex < m_steps.size()) {
        Step& s = m_steps[m_stepIndex];
        if (s.duration <= 0.0) {                 // instant step
            if (s.onUpdate) s.onUpdate(1.0);
            ++m_stepIndex;
            if (m_stepIndex < m_steps.size() && m_steps[m_stepIndex].onStart)
                m_steps[m_stepIndex].onStart();
            continue;
        }
        if (m_stepElapsed < s.duration) {        // mid-step
            if (s.onUpdate) s.onUpdate(m_stepElapsed / s.duration);
            break;                               // hold step -> nothing drawn changed
        }
        // step complete; carry leftover time into the next one
        if (s.onUpdate) s.onUpdate(1.0);
        m_stepElapsed -= s.duration;
        ++m_stepIndex;
        if (m_stepIndex < m_steps.size() && m_steps[m_stepIndex].onStart)
            m_steps[m_stepIndex].onStart();
    }

    if (m_stepIndex >= m_steps.size()) {
        StopAnim();              // full Refresh() at the end settles final state
        return;
    }

    // Union the old+new boxes of every element that changed -> minimal repaint.
    wxRect dirty;
    auto add = [&dirty](const wxRect& r) { dirty = dirty.IsEmpty() ? r : dirty.Union(r); };

    for (int i = 0; i < kCupCount; ++i) {
        if (prevSlot[i] != m_cupSlotPos[i] || prevLift[i] != m_cupLift[i]) {
            add(CupBox(prevSlot[i], prevLift[i]));
            add(CupBox(m_cupSlotPos[i], m_cupLift[i]));
        }
    }
    const bool ballPrev = m_ballCup >= 0 && prevBallOp > 0.01;
    const bool ballNow = m_ballCup >= 0 && m_ballOpacity > 0.01;
    const double curBallSlot = (m_ballCup >= 0) ? m_cupSlotPos[m_ballCup] : 0.0;
    if ((ballPrev || ballNow) && (prevBallOp != m_ballOpacity || prevBallSlot != curBallSlot)) {
        if (ballPrev) add(BallBox(prevBallSlot));
        if (ballNow) add(BallBox(curBallSlot));
    }
    if ((prevDots || m_dotsActive) && (prevDots != m_dotsActive || prevDotOp != m_dotOpacity)) {
        add(DotBox(m_dotSlotA));
        add(DotBox(m_dotSlotB));
    }

    if (!dirty.IsEmpty()) RefreshRect(dirty);
}

void GamePanel::PushBallFlashes(int times) {
    for (int i = 0; i < times; ++i) {
        m_steps.push_back({0.26, nullptr,
            [this](double t) { m_ballOpacity = std::min(1.0, t / 0.45); }});
        m_steps.push_back({0.18, nullptr,
            [this](double t) { m_ballOpacity = std::max(0.0, 1.0 - t / 0.6); }});
    }
}

// MARK: - Actions

void GamePanel::Initialize(int slot) {
    if (m_busy) return;
    m_gameFinished = false;
    m_ballCup = m_positions[slot];

    m_steps.clear();
    m_steps.push_back({0.0, [this] {
        m_revealedCup = m_ballCup;
        m_liftStart = m_cupLift[m_ballCup];
        status(wxString::FromUTF8("Watch closely \xe2\x80\x94 remember where the ball is!"));
    }, nullptr});
    m_steps.push_back({0.35, nullptr, [this](double t) {
        m_cupLift[m_revealedCup] = lerp(m_liftStart, kLiftHeight, easeOut(t));
    }});
    m_steps.push_back({0.3, nullptr, nullptr});
    PushBallFlashes(7);
    m_steps.push_back({0.0, [this] { m_liftStart = m_cupLift[m_revealedCup]; }, nullptr});
    m_steps.push_back({0.3, nullptr, [this](double t) {
        m_cupLift[m_revealedCup] = lerp(m_liftStart, 0.0, easeIn(t));
    }});

    const int slotCopy = slot;
    StartSequence([this, slotCopy] {
        m_revealedCup = -1;
        m_ballOpacity = 0.0;
        const char* names[3] = {"left", "middle", "right"};
        status(wxString::Format("Ball hidden under the %s cup. Enter swaps and press Swap!",
                                names[slotCopy]));
    });
}

void GamePanel::RunSwaps(const wxString& swaps) {
    if (m_busy) return;
    if (m_ballCup < 0) { status("Pick a cup to hide the ball first."); return; }
    if (swaps.IsEmpty()) { status("Enter at least one swap (A, B or C)."); return; }

    m_gameFinished = false;
    m_steps.clear();

    const int total = static_cast<int>(swaps.length());
    for (int i = 0; i < total; ++i) {
        const wxChar c = swaps[i];
        int a, b;
        slotsFor(c, a, b);
        const int idx = i;

        // Flash green/blue dots over the pair for ~1/3 second.
        m_steps.push_back({0.0, [this, a, b, idx, total, c] {
            m_dotSlotA = a;
            m_dotSlotB = b;
            m_dotsActive = true;
            m_dotOpacity = 0.12;
            status(wxString::Format("Swap %d of %d:  %c", idx + 1, total, c));
        }, nullptr});
        for (int k = 0; k < 2; ++k) {
            m_steps.push_back({0.08, nullptr, [this](double t) { m_dotOpacity = 0.12 + 0.88 * t; }});
            m_steps.push_back({0.08, nullptr, [this](double t) { m_dotOpacity = 1.0 - 0.88 * t; }});
        }

        // Dots vanish the instant the swap begins; slide the two cups.
        m_steps.push_back({0.0, [this, a, b] {
            m_dotsActive = false;
            m_dotOpacity = 0.0;
            std::swap(m_positions[a], m_positions[b]);
            for (int id = 0; id < kCupCount; ++id) {
                m_slideStart[id] = m_cupSlotPos[id];
                m_slideTarget[id] = SlotOf(id);
            }
        }, nullptr});
        m_steps.push_back({0.55, nullptr, [this](double t) {
            const double e = easeInOut(t);
            for (int id = 0; id < kCupCount; ++id)
                m_cupSlotPos[id] = lerp(m_slideStart[id], m_slideTarget[id], e);
        }});
        m_steps.push_back({0.15, nullptr, nullptr});

        // Lift the ball's cup to reveal its new spot, hold, then lower.
        m_steps.push_back({0.0, [this] {
            m_revealedCup = m_ballCup;
            m_liftStart = m_cupLift[m_ballCup];
        }, nullptr});
        m_steps.push_back({0.35, nullptr, [this](double t) {
            m_cupLift[m_revealedCup] = lerp(m_liftStart, kLiftHeight, easeOut(t));
            m_ballOpacity = easeOut(t);
        }});
        m_steps.push_back({0.55, nullptr, nullptr});
        m_steps.push_back({0.0, [this] { m_liftStart = m_cupLift[m_revealedCup]; }, nullptr});
        m_steps.push_back({0.30, nullptr, [this](double t) {
            m_cupLift[m_revealedCup] = lerp(m_liftStart, 0.0, easeIn(t));
            m_ballOpacity = 1.0 - easeIn(t);
        }});
        m_steps.push_back({0.10, [this] {
            m_revealedCup = -1;
            m_ballOpacity = 0.0;
        }, nullptr});
    }

    StartSequence([this] {
        m_revealedCup = -1;
        m_ballOpacity = 0.0;
        m_gameFinished = true;
        status("Done! Tap any cup to lift it and see what's underneath.");
    });
}

void GamePanel::Peek(int cupId) {
    if (m_busy || !m_gameFinished) return;
    const bool foundBall = (cupId == m_ballCup);

    m_steps.clear();
    m_steps.push_back({0.0, [this, cupId] {
        m_revealedCup = cupId;
        m_liftStart = m_cupLift[cupId];
    }, nullptr});
    m_steps.push_back({0.3, nullptr, [this](double t) {
        m_cupLift[m_revealedCup] = lerp(m_liftStart, kLiftHeight, easeOut(t));
    }});

    if (foundBall) {
        m_steps.push_back({0.0, [this] {
            status(wxString::FromUTF8("You found the ball! \xf0\x9f\x8e\x89"));
        }, nullptr});
        PushBallFlashes(7);
    } else {
        m_steps.push_back({0.0, [this] {
            status(wxString::FromUTF8("Nothing under that cup \xe2\x80\x94 keep looking!"));
        }, nullptr});
        m_steps.push_back({0.7, nullptr, nullptr});
    }

    m_steps.push_back({0.0, [this] { m_liftStart = m_cupLift[m_revealedCup]; }, nullptr});
    m_steps.push_back({0.3, nullptr, [this](double t) {
        m_cupLift[m_revealedCup] = lerp(m_liftStart, 0.0, easeIn(t));
    }});

    StartSequence([this] {
        m_revealedCup = -1;
        m_ballOpacity = 0.0;
    });
}

void GamePanel::ResetGame() {
    if (m_busy) return;
    m_positions = {0, 1, 2};
    m_ballCup = -1;
    m_revealedCup = -1;
    m_gameFinished = false;
    m_cupSlotPos = {0, 1, 2};
    m_cupLift = {0, 0, 0};
    m_ballOpacity = 0.0;
    m_dotsActive = false;
    m_dotOpacity = 0.0;
    status("Pick a cup to hide the ball, then enter swaps.");
    Refresh();
}

void GamePanel::OnLeftDown(wxMouseEvent& event) {
    if (m_busy || !m_gameFinished) return;
    const wxPoint p = event.GetPosition();
    const double cy = GetClientSize().GetHeight() / 2.0;
    for (int id = 0; id < kCupCount; ++id) {
        const double cx = SlotX(m_cupSlotPos[id]);
        const double top = cy - kCupHeight / 2.0 - m_cupLift[id];
        if (p.x >= cx - kCupWidth / 2 && p.x <= cx + kCupWidth / 2 &&
            p.y >= top && p.y <= top + kCupHeight) {
            Peek(id);
            return;
        }
    }
}
