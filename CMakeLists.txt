cmake_minimum_required(VERSION 3.14)
project(skperf VERSION 1.0.0 LANGUAGES CXX)

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

add_executable(skperf
    src/main.cpp
    src/udp_client.cpp
    src/strip_chart.cpp
)

target_include_directories(skperf PRIVATE include)
target_link_libraries(skperf PRIVATE fltk fltk_images)

# Install
install(TARGETS skperf DESTINATION bin)
