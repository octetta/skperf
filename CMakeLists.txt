cmake_minimum_required(VERSION 3.14)
project(AudioMonitor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Vendored FLTK 1.4.5 via FetchContent (no system install needed)
include(FetchContent)
FetchContent_Declare(
  fltk
  GIT_REPOSITORY https://github.com/fltk/fltk.git
  GIT_TAG release-1.4.5
)
FetchContent_MakeAvailable(fltk)

add_executable(audio_monitor
    src/main.cpp
    src/udp_client.cpp
    src/strip_chart.cpp
)

target_include_directories(audio_monitor PRIVATE include)
target_link_libraries(audio_monitor PRIVATE fltk fltk_images)

# Install
install(TARGETS audio_monitor DESTINATION bin)
