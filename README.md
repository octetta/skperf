# Skred Audio Performance Monitor

Cross-platform FLTK GUI for real-time strip chart of audio app UDP metrics.

## Build (vendored FLTK - no system deps needed)

```bash
cd skperf
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
./audio_monitor
```
