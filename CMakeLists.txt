cmake_minimum_required (VERSION 3.15)

project (synthv1
  VERSION 1.0.0
  DESCRIPTION "an old-school polyphonic synthesizer"
  HOMEPAGE_URL "https://synthv1.sourceforge.io"
  LANGUAGES C CXX)

set (PROJECT_COPYRIGHT "Copyright (C) 2012-2024, rncbc aka Rui Nuno Capela. All rights reserved.")
set (PROJECT_DOMAIN    "rncbc.org")

execute_process (
  COMMAND git describe --tags --dirty --abbrev=6
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_DESCRIBE_OUTPUT
  RESULT_VARIABLE GIT_DESCRIBE_RESULT
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if (GIT_DESCRIBE_RESULT EQUAL 0)
  set (GIT_VERSION "${GIT_DESCRIBE_OUTPUT}")
  string (REGEX REPLACE "^[^0-9]+" "" GIT_VERSION "${GIT_VERSION}")
  string (REGEX REPLACE "^1_"      "" GIT_VERSION "${GIT_VERSION}")
  string (REGEX REPLACE "^[_vV]+"  "" GIT_VERSION "${GIT_VERSION}")
  string (REGEX REPLACE "-g"   "git." GIT_VERSION "${GIT_VERSION}")
  string (REGEX REPLACE "[_|-]"   "." GIT_VERSION "${GIT_VERSION}")
  execute_process (
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_REVPARSE_OUTPUT
    RESULT_VARIABLE GIT_REVPARSE_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if (GIT_REVPARSE_RESULT EQUAL 0 AND NOT GIT_REVPARSE_OUTPUT STREQUAL "main")
    set (GIT_VERSION "${GIT_VERSION} [${GIT_REVPARSE_OUTPUT}]")
  endif ()
  set (PROJECT_VERSION "${GIT_VERSION}")
endif ()


if (CMAKE_BUILD_TYPE MATCHES "Debug")
  set (CONFIG_DEBUG 1)
  set (CONFIG_BUILD_TYPE "debug")
else ()
  set (CONFIG_DEBUG 0)
  set (CONFIG_BUILD_TYPE "release")
  set (CMAKE_BUILD_TYPE "Release")
endif ()

set (CONFIG_PREFIX "${CMAKE_INSTALL_PREFIX}")

include (GNUInstallDirs)
set (CONFIG_BINDIR  "${CONFIG_PREFIX}/${CMAKE_INSTALL_BINDIR}")
set (CONFIG_LIBDIR  "${CONFIG_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
set (CONFIG_DATADIR "${CONFIG_PREFIX}/${CMAKE_INSTALL_DATADIR}")
set (CONFIG_MANDIR  "${CONFIG_PREFIX}/${CMAKE_INSTALL_MANDIR}")


# Enable JACK standalone build.
option (CONFIG_JACK "Enable JACK stand-alone build (default=yes)" 1)

# Enable JACK session support.
option (CONFIG_JACK_SESSION "Enable JACK session support (default=yes)" 1)

# Enable JACK MIDI support option.
option (CONFIG_JACK_MIDI "Enable JACK MIDI support (default=yes)" 1)

# Enable ALSA MIDI support option.
option (CONFIG_ALSA_MIDI "Enable ALSA MIDI support (default=yes)" 1)

# Enable LV2 plugin build.
option (CONFIG_LV2 "Enable LV2 plug-in build (default=yes)" 1)

if (WIN32)
  option (CONFIG_LV2_UI_WINDOWS "Enable LV2 plug-in Windows UI support (default=yes)" 1)
  # Windows install target path.
  set (CONFIG_WINDOWS_LV2_PATH "$ENV{SYSTEMDRIVE}/Program Files/Common Files/LV2" CACHE STRING "Specify Windows LV2 install path")
else ()
  option (CONFIG_LV2_UI_X11 "Enable LV2 plug-in X11 UI support (default=yes)" 1)
endif ()

option (CONFIG_LV2_UI_EXTERNAL "Enable LV2 plug-in External UI support (default=yes)" 1)

option (CONFIG_LV2_UI_IDLE "Enable LV2 UI Idle interface support (default=yes)" 1)

option (CONFIG_LV2_UI_SHOW "Enable LV2 UI Show interface support (default=yes)" 1)

option (CONFIG_LV2_UI_RESIZE "Enable LV2 UI Resize interface support (default=yes)" 1)

option (CONFIG_LV2_PROGRAMS "Enable LV2 plug-in Programs support (default=yes)" 1)

option (CONFIG_LV2_PATCH "Enable LV2 plug-in Patch support (default=yes)" 1)

option (CONFIG_LV2_PORT_EVENT "Enable LV2 plug-in Port-event support (default=yes)" 1)

option (CONFIG_LV2_PORT_CHANGE_REQUEST "Enable LV2 plug-in Port-change request support (default=yes)" 1)


# Enable liblo availability.
option (CONFIG_LIBLO "Enable liblo interface (default=yes)" 1)

# Enable NSM support.
option (CONFIG_NSM "Enable NSM support (default=yes)" 1)

# Enable Wayland support option.
option (CONFIG_WAYLAND "Enable Wayland support (EXPERIMENTAL) (default=no)" 0)

# Enable Qt6 build preference.
option (CONFIG_QT6 "Enable Qt6 build (default=yes)" 1)


# Fix for new CMAKE_REQUIRED_LIBRARIES policy.
if (POLICY CMP0075)
  cmake_policy (SET CMP0075 NEW)
endif ()

# Fix for CXX_VISIBILITY_PRESET policy.
if (POLICY CMP0063)
  cmake_policy (SET CMP0063 NEW)
  set (CMAKE_CXX_VISIBILITY_PRESET hidden)
  set (CMAKE_VISIBILITY_INLINES_HIDDEN 1)
endif ()

# Check for Qt...
if (CONFIG_QT6)
  find_package (Qt6 QUIET)
  if (NOT Qt6_FOUND)
    set (CONFIG_QT6 0)
  endif ()
endif ()

if (CONFIG_QT6)
  find_package (QT QUIET NAMES Qt6)
else ()
  find_package (QT QUIET NAMES Qt5)
endif ()

find_package (Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Xml Svg)

#find_package (Qt${QT_VERSION_MAJOR}LinguistTools)

include (CheckIncludeFile)
include (CheckIncludeFiles)
include (CheckIncludeFileCXX)
include (CheckFunctionExists)
include (CheckLibraryExists)

# Make sure we get some subtle optimizations out there...
add_compile_options (-ffast-math)

# Checks for header files.
if (UNIX AND NOT APPLE)
  check_include_files ("fcntl.h;unistd.h;signal.h" HAVE_SIGNAL_H)
endif ()


# Find package modules
include (FindPkgConfig)

# Check for JACK libraries.
if (CONFIG_JACK)
  pkg_check_modules (JACK IMPORTED_TARGET jack>=0.100.0)
  if (JACK_FOUND)
    find_library (JACK_LIBRARY NAMES ${JACK_LIBRARIES} HINTS ${JACK_LIBDIR})
  endif ()
  if (JACK_LIBRARY)
    set (CONFIG_JACK 1)
    set (CMAKE_REQUIRED_LIBRARIES "${JACK_LIBRARY};${CMAKE_REQUIRED_LIBRARIES}")
    # Check for JACK MIDI headers availability.
    if (CONFIG_JACK_MIDI)
      check_include_file (jack/midiport.h HAVE_JACK_MIDIPORT_H)
      if (NOT HAVE_JACK_MIDIPORT_H)
        set (CONFIG_JACK_MIDI 0)
      endif ()
    endif ()
    # Check for JACK session headers availability.
    if (CONFIG_JACK_SESSION)
      check_include_file (jack/session.h HAVE_JACK_SESSION_H)
      if (NOT HAVE_JACK_SESSION_H)
        set (CONFIG_JACK_SESSION 0)
      endif ()
    endif ()
    # Check for JACK session event callback availability.
    if (CONFIG_JACK_SESSION)
      check_function_exists (jack_set_session_callback CONFIG_JACK_SESSION)
    endif ()
    # Check for ALSA libraries.
    if (CONFIG_ALSA_MIDI)
      pkg_check_modules (ALSA IMPORTED_TARGET alsa)
      if (ALSA_FOUND)
        find_library (ALSA_LIBRARY NAMES ${ALSA_LIBRARIES} HINTS ${ALSA_LIBDIR})
      endif ()
      if (ALSA_LIBRARY)
        set (CONFIG_ALSA_MIDI 1)
       #set (CMAKE_REQUIRED_LIBRARIES "${ALSA_LIBRARY};${CMAKE_REQUIRED_LIBRARIES}")
      else ()
        message (WARNING "*** ALSA library not found.")
      endif ()
    endif ()
  else ()
    message (WARNING "*** JACK library not found.")
    set (CONFIG_JACK 0)
  endif ()
endif ()

if (NOT CONFIG_JACK)
  set (CONFIG_JACK_SESSION 0)
  set (CONFIG_JACK_MIDI 0)
  set (CONFIG_ALSA_MIDI 0)
  set (CONFIG_LIBLO 0)
  set (CONFIG_NSM 0)
endif ()

# Check for LIBLO libraries.
if (CONFIG_LIBLO)
  pkg_check_modules (LIBLO IMPORTED_TARGET liblo)
  if (NOT LIBLO_FOUND)
    message (WARNING "*** LIBLO library not found.")
    set (CONFIG_LIBLO 0)
  endif ()
endif ()

# Check for LV2 support.
if (CONFIG_LV2)
  pkg_check_modules (LV2 lv2)
  if (LV2_FOUND)
    include_directories (${LV2_INCLUDE_DIRS})
    # Check for LV2 old/new headers style.
    check_include_file (lv2/urid/urid.h HAVE_LV2_URID_H)
    if (NOT HAVE_LV2_URID_H)
      check_include_file (lv2/lv2plug.in/ns/ext/urid/urid.h HAVE_OLD_LV2_URID_H)
      if (NOT HAVE_OLD_LV2_URID_H)
        set (CONFIG_LV2 0)
      else ()
        set (CONFIG_LV2_OLD_HEADERS 1)
      endif ()
    else ()
      set (CONFIG_LV2_OLD_HEADERS 0)
    endif ()
  endif ()
  if (NOT CONFIG_LV2)
    message (WARNING "*** LV2 SDK not found.")
  endif ()
endif ()

if (CONFIG_LV2)
  # Check for LV2 Atom support.
  if (CONFIG_LV2_OLD_HEADERS)
    check_include_file (lv2/lv2plug.in/ns/ext/atom/atom.h HAVE_LV2_ATOM_H)
  else ()
    check_include_file (lv2/atom/atom.h HAVE_LV2_ATOM_H)
  endif ()
  if (NOT HAVE_LV2_ATOM_H)
    set (CONFIG_LV2_ATOM 0)
  else ()
    set (CONFIG_LV2_ATOM 1)
  endif ()
  set (CONFIG_LV2_ATOM_FORGE_OBJECT ${CONFIG_LV2_ATOM})
  set (CONFIG_LV2_ATOM_FORGE_KEY ${CONFIG_LV2_ATOM})
  # Check for LV2 UI support.
  if (CONFIG_LV2_OLD_HEADERS)
    check_include_file (lv2/lv2plug.in/ns/extensions/ui/ui.h HAVE_LV2_UI_H)
  else ()
    check_include_file (lv2/ui/ui.h HAVE_LV2_UI_H)
  endif ()
  if (NOT HAVE_LV2_UI_H)
    set (CONFIG_LV2_UI 0)
  else ()
    set (CONFIG_LV2_UI 1)
  endif ()
  if (NOT CONFIG_LV2_UI)
    set (CONFIG_LV2_UI_X11 0)
    set (CONFIG_LV2_UI_WINDOWS 0)
    set (CONFIG_LV2_UI_EXTERNAL 0)
    set (CONFIG_LV2_UI_IDLE 0)
    set (CONFIG_LV2_UI_SHOW 0)
    set (CONFIG_LV2_UI_RESIZE 0)
  endif ()
endif ()

# Check for LV2 headers.
if (CONFIG_LV2)
  set (LV2_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src/lv2)
  set (CMAKE_REQUIRED_INCLUDES "${LV2_INCLUDES};${CMAKE_REQUIRED_INCLUDES}")
  include_directories (${LV2_INCLUDES})
  if (CONFIG_LV2_OLD_HEADERS)
    set (CMAKE_REQUIRED_DEFINITIONS "-DCONFIG_LV2_OLD_HEADERS;${CMAKE_REQUIRED_DEFINITIONS}")
  endif ()
else ()
  set (CONFIG_LV2_UI_X11 0)
  set (CONFIG_LV2_UI_WINDOWS 0)
  set (CONFIG_LV2_UI_EXTERNAL 0)
  set (CONFIG_LV2_UI_IDLE 0)
  set (CONFIG_LV2_UI_SHOW 0)
  set (CONFIG_LV2_UI_RESIZE 0)
  set (CONFIG_LV2_PROGRAMS 0)
  set (CONFIG_LV2_PATCH 0)
  set (CONFIG_LV2_PORT_EVENT 0)
  set (CONFIG_LV2_PORT_CHANGE_REQUEST 0)
endif ()

if (CONFIG_LV2_UI_EXTERNAL)
  check_include_file (lv2_external_ui.h HAVE_LV2_EXTERNAL_UI_H)
  if (NOT HAVE_LV2_EXTERNAL_UI_H)
    set (CONFIG_LV2_UI_EXTERNAL 0)
  endif ()
endif ()

if (CONFIG_LV2_PROGRAMS)
  check_include_file (lv2_programs.h HAVE_LV2_PROGRAMS_H)
  if (NOT HAVE_LV2_PROGRAMS_H)
    set (CONFIG_LV2_PROGRAMS 0)
  endif ()
endif ()

if (CONFIG_LV2_PATCH)
  if (CONFIG_LV2_OLD_HEADERS)
    check_include_file (lv2/lv2plug.in/ns/ext/patch/patch.h HAVE_LV2_PATCH_H)
  else ()
    check_include_file (lv2/patch/patch.h HAVE_LV2_PATCH_H)
 endif ()
  if (NOT HAVE_LV2_PATCH_H)
    set (CONFIG_LV2_PATCH 0)
  endif ()
endif ()

if (CONFIG_LV2_PORT_CHANGE_REQUEST)
  check_include_file (lv2_port_change_request.h HAVE_LV2_PORT_CHANGE_REQUEST_H)
  if (NOT HAVE_LV2_PORT_CHANGE_REQUEST_H)
    set (CONFIG_LV2_PORT_CHANGE_REQUEST 0)
  endif ()
endif ()


add_subdirectory (src)


# Configuration status
macro (SHOW_OPTION text value)
  if (${value})
    message ("${text}: yes")
  else ()
    message ("${text}: no")
  endif ()
endmacro ()

message   ("\n  ${PROJECT_NAME} ${PROJECT_VERSION} (Qt ${QT_VERSION})")
message   ("\n  Build target . . . . . . . . . . . . . . . . . . .: ${CONFIG_BUILD_TYPE}\n")
show_option ("  JACK stand-alone build . . . . . . . . . . . . . ." CONFIG_JACK)
show_option ("  JACK session support . . . . . . . . . . . . . . ." CONFIG_JACK_SESSION)
show_option ("  JACK MIDI support  . . . . . . . . . . . . . . . ." CONFIG_JACK_MIDI)
show_option ("  ALSA MIDI support  . . . . . . . . . . . . . . . ." CONFIG_ALSA_MIDI)
show_option ("  LV2 plug-in build  . . . . . . . . . . . . . . . ." CONFIG_LV2)
if (WIN32)
show_option ("  LV2 plug-in Windows UI support   . . . . . . . . ." CONFIG_LV2_UI_WINDOWS)
else ()
show_option ("  LV2 plug-in X11 UI support   . . . . . . . . . . ." CONFIG_LV2_UI_X11)
endif ()
show_option ("  LV2 plug-in External UI support  . . . . . . . . ." CONFIG_LV2_UI_EXTERNAL)
show_option ("  LV2 plug-in UI Idle interface support  . . . . . ." CONFIG_LV2_UI_IDLE)
show_option ("  LV2 plug-in UI Show interface support  . . . . . ." CONFIG_LV2_UI_SHOW)
show_option ("  LV2 plug-in UI Resize interface support  . . . . ." CONFIG_LV2_UI_RESIZE)
show_option ("  LV2 plug-in Programs support . . . . . . . . . . ." CONFIG_LV2_PROGRAMS)
show_option ("  LV2 plug-in Patch support  . . . . . . . . . . . ." CONFIG_LV2_PATCH)
show_option ("  LV2 plug-in Port-event support . . . . . . . . . ." CONFIG_LV2_PORT_EVENT)
show_option ("  LV2 plug-in Port-change request (EXPERIMENTAL) . ." CONFIG_LV2_PORT_CHANGE_REQUEST)
show_option ("  OSC service support (liblo)  . . . . . . . . . . ." CONFIG_LIBLO)
show_option ("  Non/New Session Management (NSM) support . . . . ." CONFIG_NSM)
message   ("\n  Install prefix . . . . . . . . . . . . . . . . . .: ${CONFIG_PREFIX}\n")
