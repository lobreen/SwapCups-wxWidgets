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

## Verifying on a new OS

wxWidgets and CMake are cross-platform; only the dependency package names and
the run path differ. Install the deps, then the build is the same everywhere:

```sh
cmake -S . -B build
cmake --build build
```

The `MACOSX_BUNDLE` target option is ignored off macOS, so on Linux/FreeBSD you
get a plain `build/SwapCups` executable (and `SwapCups.exe` on Windows) rather
than a `.app`.

| OS | Install deps | Run |
|---|---|---|
| Linux (Debian/Ubuntu) | `sudo apt install cmake g++ libwxgtk3.2-dev` | `./build/SwapCups` |
| Linux (Fedora) | `sudo dnf install cmake gcc-c++ wxGTK-devel` | `./build/SwapCups` |
| Linux (Arch) | `sudo pacman -S cmake wxwidgets-gtk3` | `./build/SwapCups` |
| FreeBSD | `pkg install cmake wx32-gtk3` | `./build/SwapCups` |
| Windows (MSYS2, incl. ARM64) | install `cmake` + `wxWidgets` from the matching MSYS2 toolchain | `build\SwapCups.exe` |

(Exact package names vary by distro/version — e.g. some Debian releases use
`libwxgtk3.0-gtk3-dev`, and the FreeBSD/MSYS2 names track the wx 3.x version.)

**Functional check (same on every OS):**
1. Pick a cup — it lifts and the ball flashes 7×.
2. Enter swaps (e.g. `ABCAB`) and press **Swap!** — green/blue dots blink over each pair before the cups slide.
3. When done, click/tap a cup — the ball flashes 7× if found, else "Nothing under that cup."

When a platform passes, flip its badge from `planned-lightgrey` to
`tested-2ea44f` and add the topic, e.g. `gh repo edit … --add-topic linux`.

## Sibling ports

The same game in other stacks:

- [SwapCups-SwiftUI](https://github.com/lobreen/SwapCups-SwiftUI) — SwiftUI / macOS
- [SwapCups-Python](https://github.com/lobreen/SwapCups-Python) — Python / tkinter
