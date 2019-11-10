project(SYNTHV1)

cmake_minimum_required(VERSION 3.1)

set (VERSION "0.9.12")

set (CONFIG_VERSION ${VERSION})
execute_process (
  COMMAND git describe --tags --dirty --abbrev=6
  OUTPUT_VARIABLE GIT_DESCRIBE_OUTPUT
  RESULT_VARIABLE GIT_DESCRIBE_RESULT
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if (GIT_DESCRIBE_RESULT EQUAL 0)
  set (VERSION "${GIT_DESCRIBE_OUTPUT}")
  string (REGEX REPLACE "^[^_]+"   "" VERSION "${VERSION}")
  string (REGEX REPLACE "^[_vV]+"  "" VERSION "${VERSION}")
  string (REGEX REPLACE "-g"   "git." VERSION "${VERSION}")
  string (REGEX REPLACE "[_|-]+"  "." VERSION "${VERSION}")
  execute_process (
    COMMAND git rev-parse --abbrev-ref HEAD
    OUTPUT_VARIABLE GIT_REVPARSE_OUTPUT
    RESULT_VARIABLE GIT_REVPARSE_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if (GIT_REVPARSE_RESULT EQUAL 0 AND NOT GIT_REVPARSE_OUTPUT STREQUAL "master")
    set (VERSION "${VERSION} [${GIT_REVPARSE_OUTPUT}]")
  endif ()
else ()
  set (VERSION "${VERSION}")
endif ()

set (PACKAGE_NAME "synthv1")
set (PACKAGE_VERSION "${VERSION}")
set (PACKAGE_BUGREPORT "rncbc@rncbc.org")
set (PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set (PACKAGE_TARNAME "synthv1")

set (CONFIG_BUILD_VERSION "${PACKAGE_VERSION}")

if (CMAKE_BUILD_TYPE)
  set (CONFIG_BUILD_TYPE ${CMAKE_BUILD_TYPE})
else ()
  set (CONFIG_BUILD_TYPE "release")
endif ()

set (CONFIG_DEBUG 0)
if (CONFIG_BUILD_TYPE MATCHES "debug")
  set (CONFIG_DEBUG 1)
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

option (CONFIG_LV2_UI_X11 "Enable LV2 plug-in X11 UI support (default=yes)" 1)

option (CONFIG_LV2_UI_EXTERNAL "Enable LV2 plug-in External UI support (default=yes)" 1)

option (CONFIG_LV2_UI_IDLE "Enable LV2 UI Idle interface support (default=yes)" 1)

option (CONFIG_LV2_UI_SHOW "Enable LV2 UI Show interface support (default=yes)" 1)

option (CONFIG_LV2_UI_RESIZE "Enable LV2 UI Resize interface support (default=yes)" 1)

option (CONFIG_LV2_PROGRAMS "Enable LV2 plug-in Programs support (default=yes)" 1)

option (CONFIG_LV2_PATCH "Enable LV2 plug-in Patch support (default=yes)" 1)

# Enable liblo availability.
option (CONFIG_LIBLO "Enable liblo interface (default=yes)" 1)

# Enable NSM support.
option (CONFIG_NSM "Enable NSM support (default=yes)" 1)


# Fix for new CMAKE_REQUIRED_LIBRARIES policy.
if (POLICY CMP0075)
  cmake_policy (SET CMP0075 NEW)
endif ()

# Check for Qt
find_package (Qt5 REQUIRED COMPONENTS Core Gui Widgets Xml)

#find_package (Qt5LinguistTools)

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


# Check for LV2 headers.
if (CONFIG_LV2)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/src/lv2)
endif ()

# Find package modules
find_package (PkgConfig REQUIRED)

# Check for JACK libraries.
if (CONFIG_JACK)
  pkg_check_modules (JACK jack>=0.100.0)
  set (CONFIG_JACK ${JACK_FOUND})
  if (CONFIG_JACK)
    include_directories (${JACK_INCLUDE_DIRS})
    link_directories (${JACK_LIBRARY_DIRS})
  # link_libraries (${JACK_LIBRARIES})
    set (CMAKE_REQUIRED_LIBRARIES "${JACK_LIBRARIES};${CMAKE_REQUIRED_LIBRARIES}")
    # Check for JACK MIDI headers availability.
    if (CONFIG_JACK_MIDI)
      check_include_file (jack/midiport.h CONFIG_JACK_MIDI)
    endif ()
    # Check for JACK session headers availability.
    if (CONFIG_JACK_SESSION)
      check_include_file (jack/session.h CONFIG_JACK_SESSION)
    endif ()
    # Check for JACK session event callback availability.
    if (CONFIG_JACK_SESSION)
      check_function_exists (jack_set_session_callback CONFIG_JACK_SESSION)
    endif ()
    # Check for ALSA libraries.
    if (CONFIG_ALSA_MIDI)
      pkg_check_modules (ALSA alsa)
      set (CONFIG_ALSA_MIDI ${ALSA_FOUND})
      if (CONFIG_ALSA_MIDI)
        include_directories (${ALSA_INCLUDE_DIRS})
        link_directories (${ALSA_LIBRARY_DIRS})
        link_libraries (${ALSA_LIBRARIES})
      # set (CMAKE_REQUIRED_LIBRARIES "${ALSA_LIBRARIES};${CMAKE_REQUIRED_LIBRARIES}")
      else ()
        message (WARNING "*** ALSA library not found.")
      endif ()
    endif ()
  else ()
    message (WARNING "*** JACK library not found.")
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
  pkg_check_modules (LIBLO liblo)
  set (CONFIG_LIBLO ${LIBLO_FOUND})
  if (CONFIG_LIBLO)
    include_directories (${LIBLO_INCLUDE_DIRS})
    link_directories (${LIBLO_LIBRARY_DIRS})
    link_libraries (${LIBLO_LIBRARIES})
  # set (CMAKE_REQUIRED_LIBRARIES "${LIBLO_LIBRARIES};${CMAKE_REQUIRED_LIBRARIES}")
  else ()
    message (WARNING "*** LIBLO library not found.")
  endif ()
endif ()

# Check for LV2 support.
if (CONFIG_LV2)
  pkg_check_modules (LV2 lv2)
  set (CONFIG_LV2 ${LV2_FOUND})
  if (CONFIG_LV2)
    include_directories (${LV2_INCLUDE_DIRS})
    link_directories (${LV2_LIBRARY_DIRS})
    link_libraries (${LV2_LIBRARIES})
  # set (CMAKE_REQUIRED_LIBRARIES "${LV2_LIBRARIES};${CMAKE_REQUIRED_LIBRARIES}")
    # Check for LV2 Atom support.
    check_include_file (lv2/lv2plug.in/ns/ext/atom/atom.h CONFIG_LV2_ATOM)
    set (CONFIG_LV2_ATOM_FORGE_OBJECT ${CONFIG_LV2_ATOM})
    set (CONFIG_LV2_ATOM_FORGE_KEY ${CONFIG_LV2_ATOM})
    # Check for LV2 UI support.
    check_include_file (lv2/lv2plug.in/ns/extensions/ui/ui.h CONFIG_LV2_UI)
    if (NOT CONFIG_LV2_UI)
      set (CONFIG_LV2_UI_X11 0)
      set (CONFIG_LV2_UI_EXTERNAL 0)
      set (CONFIG_LV2_UI_IDLE 0)
      set (CONFIG_LV2_UI_SHOW 0)
      set (CONFIG_LV2_UI_RESIZE 0)
    endif ()
  endif ()
endif ()

if (CONFIG_LV2)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/src/lv2)
else ()
  set (CONFIG_LV2_UI_X11 0)
  set (CONFIG_LV2_UI_EXTERNAL 0)
  set (CONFIG_LV2_UI_IDLE 0)
  set (CONFIG_LV2_UI_SHOW 0)
  set (CONFIG_LV2_UI_RESIZE 0)
  set (CONFIG_LV2_PROGRAMS 0)
  set (CONFIG_LV2_PATCH 0)
endif ()

if (CONFIG_LV2_UI_EXTERNAL)
  check_include_file (lv2_external_ui.h, CONFIG_LV2_UI_EXTERNAL)
endif ()

if (CONFIG_LV2_PROGRAMS)
  check_include_file (lv2_programs.h CONFIG_LV2_PROGRAMS)
endif ()

if (CONFIG_LV2_PATCH)
  check_include_file (lv2/lv2plug.in/ns/ext/patch/patch.h CONFIG_LV2_PATCH)
endif ()


add_subdirectory (src)


configure_file (synthv1.spec.in synthv1.spec IMMEDIATE @ONLY)

install (FILES synthv1.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
install (FILES synthv1.fr.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/fr/man1 RENAME synthv1.1)

# Configuration status
macro (SHOW_OPTION text value)
  if (${value})
    message ("${text}: yes")
  else ()
    message ("${text}: no")
  endif ()
endmacro ()


message   ("\n  ${PACKAGE_NAME} ${PACKAGE_VERSION}")
message   ("\n  Build target . . . . . . . . . . . . . . . . . . .: ${CONFIG_BUILD_TYPE}\n")
show_option ("  JACK stand-alone build . . . . . . . . . . . . . ." CONFIG_JACK)
show_option ("  JACK session support . . . . . . . . . . . . . . ." CONFIG_JACK_SESSION)
show_option ("  JACK MIDI support  . . . . . . . . . . . . . . . ." CONFIG_JACK_MIDI)
show_option ("  ALSA MIDI support  . . . . . . . . . . . . . . . ." CONFIG_ALSA_MIDI)
show_option ("  LV2 plug-in build  . . . . . . . . . . . . . . . ." CONFIG_LV2)
show_option ("  LV2 plug-in X11 UI support   . . . . . . . . . . ." CONFIG_LV2_UI_X11)
show_option ("  LV2 plug-in External UI support  . . . . . . . . ." CONFIG_LV2_UI_EXTERNAL)
show_option ("  LV2 plug-in UI Idle interface support  . . . . . ." CONFIG_LV2_UI_IDLE)
show_option ("  LV2 plug-in UI Show interface support  . . . . . ." CONFIG_LV2_UI_SHOW)
show_option ("  LV2 plug-in UI Resize interface support  . . . . ." CONFIG_LV2_UI_RESIZE)
show_option ("  LV2 plug-in Programs support . . . . . . . . . . ." CONFIG_LV2_PROGRAMS)
show_option ("  LV2 plug-in Patch support  . . . . . . . . . . . ." CONFIG_LV2_PATCH)
show_option ("  OSC service support (liblo)  . . . . . . . . . . ." CONFIG_LIBLO)
show_option ("  NSM (Non Session Management) support . . . . . . ." CONFIG_NSM)
message   ("\n  Install prefix . . . . . . . . . . . . . . . . . .: ${CMAKE_INSTALL_PREFIX}")
message   ("\nNow type 'make', followed by 'make install' as root.\n")
