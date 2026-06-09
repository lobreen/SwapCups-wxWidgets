#pragma once

#include <wx/wx.h>
#include <wx/stopwatch.h>
#include <array>
#include <vector>
#include <functional>

// The game canvas. Owns the shell-game model (see the canonical spec), paints
// the table/cups/ball/dots, and drives all animation through a wxTimer-based
// step scheduler.
//
// Model: positions[slot] = cup id currently in that slot.
//        slot 0 = left, 1 = middle, 2 = right. Cups keep their identity.
class GamePanel : public wxPanel {
public:
    explicit GamePanel(wxWindow* parent);

    static constexpr int kCupCount = 3;

    // Game actions (no-ops while an animation is running).
    void Initialize(int slot);          // hide the ball under the cup at `slot`
    void RunSwaps(const wxString& swaps);
    void ResetGame();

    void SetStatusCallback(std::function<void(const wxString&)> cb) { m_statusCb = std::move(cb); }
    void SetBusyCallback(std::function<void(bool)> cb) { m_busyCb = std::move(cb); }

private:
    // A single timed animation step. `onStart` fires once when the step begins;
    // `onUpdate(t)` fires each frame with t in [0,1]. Either may be null.
    struct Step {
        double duration;
        std::function<void()> onStart;
        std::function<void(double)> onUpdate;
    };

    void OnPaint(wxPaintEvent&);
    void OnTimer(wxTimerEvent&);
    void OnLeftDown(wxMouseEvent&);
    void OnSize(wxSizeEvent&);

    void DrawScene(wxGraphicsContext* gc, const wxSize& client);

    // Bounding boxes of the animated elements, for minimal-region repaints.
    wxRect CupBox(double slotPos, double lift) const;
    wxRect BallBox(double slotPos) const;
    wxRect DotBox(int slot) const;

    void Peek(int cupId);               // lift a tapped cup after the game ends
    void PushBallFlashes(int times);    // append 7-style in/out ball flashes
    void StartSequence(std::function<void()> onComplete);
    void StopAnim();

    double SlotX(double slot) const;    // pixel x for a (possibly fractional) slot
    int SlotOf(int cupId) const;        // current slot of a cup id
    void status(const wxString& s) { if (m_statusCb) m_statusCb(s); }

    // --- Game model ---
    std::array<int, kCupCount> m_positions{0, 1, 2};
    int m_ballCup = -1;
    int m_revealedCup = -1;
    bool m_gameFinished = false;

    // --- Animated quantities ---
    std::array<double, kCupCount> m_cupSlotPos{0, 1, 2};  // fractional slot during slides
    std::array<double, kCupCount> m_cupLift{0, 0, 0};     // lift in pixels
    double m_ballOpacity = 0.0;
    int m_dotSlotA = -1, m_dotSlotB = -1;
    double m_dotOpacity = 0.0;
    bool m_dotsActive = false;

    // Scratch used by slide/lift interpolation.
    std::array<double, kCupCount> m_slideStart{};
    std::array<double, kCupCount> m_slideTarget{};
    double m_liftStart = 0.0;

    // --- Scheduler ---
    std::vector<Step> m_steps;
    size_t m_stepIndex = 0;
    double m_stepElapsed = 0.0;
    std::function<void()> m_onComplete;
    wxTimer m_timer;
    wxStopWatch m_stopwatch;
    bool m_busy = false;

    // Persistent backing store so partial repaints keep untouched pixels.
    wxBitmap m_backing;
    bool m_fullRedraw = true;

    std::function<void(const wxString&)> m_statusCb;
    std::function<void(bool)> m_busyCb;
};
