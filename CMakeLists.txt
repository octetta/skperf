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

# macOS App Bundle & Ad-Hoc Code Signing Configuration
if(APPLE)
  set_target_properties(skperf PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "skperf"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.skred.skperf"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
  )

  find_program(CODESIGN_EXECUTABLE codesign)
  if(CODESIGN_EXECUTABLE)
    add_custom_command(TARGET skperf POST_BUILD
      COMMAND ${CODESIGN_EXECUTABLE} --force --deep --sign - $<TARGET_BUNDLE_DIR:skperf>
      COMMENT "Ad-hoc code signing skperf.app bundle"
    )
  endif()
endif()

# Install
install(TARGETS skperf DESTINATION bin)
