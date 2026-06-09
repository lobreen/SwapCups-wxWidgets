# SwapCups (C++ / wxWidgets)

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

Scaffold builds and runs: window opens, table + three cups paint. Next:
initialize/ball-hide, swap animation with pre-swap green/blue dots, and the
post-game tap-to-reveal (flash the ball 7×).
