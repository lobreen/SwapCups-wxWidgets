# SwapCups (C++ / wxWidgets)

![macOS](https://img.shields.io/badge/macOS-tested-2ea44f)
![Linux](https://img.shields.io/badge/Linux-planned-lightgrey)
![FreeBSD](https://img.shields.io/badge/FreeBSD-planned-lightgrey)
![Windows ARM64](https://img.shields.io/badge/Windows%20ARM64-planned-lightgrey)

> Platforms tested: **macOS**. wxWidgets is cross-platform, so Linux (GTK),
> FreeBSD, and Windows (incl. ARM64) are expected to work and are planned for
> testing — not yet verified.

A C++ wxWidgets port of the Swap Cups shell game (previously built in Python and
SwiftUI). Three cups sit inverted on a table; one hides a blue ball. The player
hides the ball, runs a sequence of swaps (A = left↔middle, B = middle↔right,
C = left↔right), and finds the ball afterward.

## Requirements

- macOS with Homebrew
- `brew install wxwidgets cmake`
- A C++17 compiler (clang)

## Build & run

```sh
cmake -S . -B build
cmake --build build
open build/SwapCups.app    # or: ./build/SwapCups.app/Contents/MacOS/SwapCups
```

## Layout

```
SwapCups/
├── CMakeLists.txt
├── src/
│   ├── SwapCupsApp.{h,cpp}    # wxApp entry point
│   ├── SwapCupsFrame.{h,cpp}  # main window
│   └── GamePanel.{h,cpp}      # game canvas + model (table, cups, ball)
└── build/                     # generated (gitignored)
```

## Status

Complete: ball-hide init flash, swap animation with pre-swap green/blue dots,
and post-game tap-to-reveal (ball flashes 7×). All animation runs through a
`wxTimer`-based step scheduler in `GamePanel`. Rendering is optimized with
**dirty-rect repaint** — a persistent backing bitmap plus per-element bounding
boxes repaint only the region that changed each frame, cutting CPU ~44% and
RAM ~21% versus full-canvas repaints (measured over a 156-swap run).

## Sibling ports

The same game in other stacks:

- [SwapCups-SwiftUI](https://github.com/lobreen/SwapCups-SwiftUI) — SwiftUI / macOS
- [SwapCups-Python](https://github.com/lobreen/SwapCups-Python) — Python / tkinter
