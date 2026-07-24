# skperf - Audio Performance Monitor

**skperf** is a modern, lightweight, cross-platform C++/FLTK portable application designed for real-time UDP audio telemetry monitoring and performance strip charting.

---

## Features

- **High-Performance Strip Charting**: Built using custom FLTK vector drawing (`fl_draw.H`) featuring vibrant 8-color curve strokes, semi-transparent area fills, endpoint markers, dynamic Y-axis min/mid/max scaling, and real-time numerical readouts.
- **Dual Display Modes**:
  - `Mode: Stacked` (Default): Individual sub-chart plots stacked vertically, each auto-scaled to its own metric bounds.
  - `Mode: Overlay`: All active metric curves plotted on a single unified canvas with a side legend readout.
- **Interactive Metric Line Toggling**: Dedicated color-coded metric toggle buttons (`[ overruns ]`, `[ cb_ms_last ]`, `[ cb_ms_avg ]`, etc.) directly above the strip chart lines to quickly show or hide specific metrics.
- **Flat UI Design**: Modern border box styling (`FL_BORDER_BOX`) with zero 3D bevel clutter, clean field alignment, and high-visibility text insertion cursors.
- **Light / Dark Mode**: One-click `Theme: Dark` / `Theme: Light` toggle button to dynamically restyle the application and plot canvas.
- **Double-Buffered Flicker-Free Window**: Built on `Fl_Double_Window` to eliminate screen flashing, tearing, or flickering during fast data streaming or window resizing.
- **Cross-Platform & Self-Contained**: Written in standard C++17 with vendored FLTK 1.4.5 (via CMake `FetchContent`). Runs natively on **Linux**, **macOS**, and **Windows** with zero external GUI system dependencies.

---

## User Interface Overview

1. **Top Control Bar**:
   - `IP Address`: Target UDP server IP (Default: `127.0.0.1`).
   - `UDP Port`: Target UDP server port (Default: `60440`).
   - `Refresh (s)`: Polling interval in seconds (Default: `2`).
   - `Connect / Disconnect`: Starts or stops telemetry polling.
   - `Status`: Connection badge (`Disconnected` / `Connected`).
   - `Refresh count`: Total number of telemetry packets received.
   - `Last response`: Timestamp of the most recent telemetry packet.

2. **Metric Lines Toolbar**:
   - `Metric Lines`: Color-coded toggle buttons for `overruns`, `cb_ms_last`, `cb_ms_avg`, `load_last`, `load_avg`, `late_starts`, `discont`, and `clipped`.
   - `Mode: Stacked / Overlay`: Switches between stacked sub-charts and overlay graph.
   - `Theme: Dark / Light`: Toggles between dark and light color themes.
   - `Clear`: Resets telemetry history and clears the chart.

3. **Strip Chart Canvas**:
   - Main real-time graph area displaying metric history curves, axis scale bounds, and current values.

---

## UDP Telemetry Protocol & Payload Contract (`/a?`)

`skperf` communicates with the target audio server via non-blocking UDP sockets.

### Protocol Flow
1. **Connection**: Upon clicking `Connect`, `skperf` sends an initialization packet:
   ```text
   log 1
   ```
2. **Polling**: At the configured refresh interval, `skperf` sends the telemetry query command:
   ```text
   /a?
   ```
3. **Response**: The server responds with a plain ASCII text string containing key-value performance telemetry metrics.

### Expected Payload Contract

The UDP server's response to `/a?` is expected to contain plain ASCII text matching the following key prefixes:

| Metric Key | Telemetry Keyword | Description | Unit / Scale |
| :--- | :--- | :--- | :--- |
| `overruns` | `overruns:` | Audio buffer overrun count | Integer count |
| `cb_ms_last` | `last` (after callback block) | Duration of the last audio callback | Milliseconds (`ms`) |
| `cb_ms_avg` | `avg` (after callback block) | Average audio callback duration | Milliseconds (`ms`) |
| `load_last` | `last` (after load block) | Last audio engine CPU load | Percentage (`%`) |
| `load_avg` | `avg` (after load block) | Average audio engine CPU load | Percentage (`%`) |
| `late_starts` | `late-starts:` | Number of late-started audio buffers | Integer count |
| `discont` | `discontinuities:` | Audio stream buffer drops / discontinuities | Integer count |
| `clipped` | `clipped-samples:` | Number of clipped audio samples | Integer count |

### Example Server Response String
```text
overruns: 0 cb_ms: last 1.42 avg 1.15 load: last 12.5% avg 10.2% late-starts: 0 discontinuities: 0 clipped-samples: 0
```

---

## Building and Running

### Build Prerequisites & Platform Expectations

- **Linux**:
  - Compiler: GCC 8+ or Clang 8+ (C++17 support).
  - Tools: `cmake` (>= 3.14), `make` or `ninja`.
  - System Dependencies: Standard X11/OpenGL development libraries (e.g. `libx11-dev`, `libxft-dev`, `libxext-dev`, `libxinerama-dev`, `libxcursor-dev`, `libxrender-dev`, `libgl1-mesa-dev` on Debian/Ubuntu, or `xorg-x11-server-devel`, `libXft-devel` on Fedora/RHEL).
- **macOS**:
  - Tools: Xcode Command Line Tools (`clang` / `clang++`), `cmake` (>= 3.14).
  - Output: Package target `skperf.app` native application bundle with automatic ad-hoc code signing (`codesign --force --deep --sign -`).
- **Windows (Standard MSVC / MinGW)**:
  - Tools: Visual Studio 2019/2022 (with C++ workload), MinGW-w64, or Clang, plus `cmake` (>= 3.14).
  - Libraries: Automatically links `ws2_32` for Windows Winsock sockets.
- **Windows (Zig + Ninja Toolchain)**:
  - Tools: `zig` (0.10+), `ninja`, `cmake` (>= 3.14).
  - `zig c++` acts as a drop-in Clang compiler driver for CMake.

---

### Quick Build (Linux / macOS / Posix)
```bash
make builder
./build/skperf
```

### Standard CMake Build (Cross-Platform)

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### Windows Build with Zig & Ninja

If building on Windows using **Zig** as the compiler with **Ninja**:

```cmd
mkdir build
cd build
cmake .. -G Ninja -DCMAKE_C_COMPILER="zig;cc" -DCMAKE_CXX_COMPILER="zig;c++" -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### macOS App Bundle & Execution
When building on macOS, the native `.app` bundle is generated and signed automatically:

```bash
open ./build/skperf.app
```
