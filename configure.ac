# Process this file with autoconf to produce a configure script.
AC_INIT(synthv1, 0.9.2, rncbc@rncbc.org)

AC_CONFIG_SRCDIR(src/synthv1.cpp)
AC_CONFIG_HEADERS(src/config.h)
AC_CONFIG_FILES(Makefile synthv1.spec src/src_core.pri src/src_ui.pri src/src_jack.pri src/src_lv2.pri)

# Build version string.
AC_CACHE_VAL([ac_cv_build_version], [
   ac_cv_build_version=$(git describe --tags --dirty --abbrev=6 2>/dev/null)
   if test -n "$ac_cv_build_version"; then
      ac_cv_build_version=$(echo $ac_cv_build_version | sed 's/^[[^_]]*[[_v]]//i')
      ac_cv_build_version=$(echo $ac_cv_build_version | sed 's/-g/git./')
      ac_cv_build_version=$(echo $ac_cv_build_version | sed 's/[[_|-]]\+/./g')
      ac_cv_build_version_extra=$(git rev-parse --abbrev-ref HEAD 2>/dev/null)
      if test "x$ac_cv_build_version_extra" != "xmaster"; then
         ac_cv_build_version="$ac_cv_build_version [[$ac_cv_build_version_extra]]"
      fi
   else
      ac_cv_build_version=$PACKAGE_VERSION
   fi
])
ac_build_version="$ac_cv_build_version"
AC_DEFINE_UNQUOTED(CONFIG_BUILD_VERSION, ["$ac_build_version"], [Build version string.])

# Sanitized version string.
AC_CACHE_VAL([ac_cv_version], [
   ac_cv_version=$(echo $PACKAGE_VERSION | sed -r 's/^([[0-9|\.]]+).*$/\1/')
])
ac_version="$ac_cv_version"
AC_DEFINE_UNQUOTED(CONFIG_VERSION, ["$ac_version"], [Version string.])
AC_SUBST(ac_version)

# Set default installation prefix.
AC_PREFIX_DEFAULT(/usr/local)
if test "x$prefix" = "xNONE"; then
   prefix=$ac_default_prefix
fi
if test "x$exec_prefix" = "xNONE"; then
   exec_prefix=$prefix
fi
eval ac_prefix=$prefix
AC_SUBST(ac_prefix)
AC_DEFINE_UNQUOTED(CONFIG_PREFIX, ["$ac_prefix"], [Default installation prefix.])

# Set default installation directories.
eval ac_bindir=$bindir
AC_SUBST(ac_bindir)
AC_DEFINE_UNQUOTED(CONFIG_BINDIR, ["$ac_bindir"], [Default executable binary path.])

eval ac_libdir=$libdir
AC_SUBST(ac_libdir)
AC_DEFINE_UNQUOTED(CONFIG_LIBDIR, ["$ac_libdir"], [Default object library path.])

eval datarootdir=$datarootdir
eval ac_datadir=$datadir
AC_SUBST(ac_datadir)
AC_DEFINE_UNQUOTED(CONFIG_DATADIR, ["$ac_datadir"], [Default arch-idependent data path.])

eval ac_mandir=$mandir
AC_SUBST(ac_mandir)
AC_DEFINE_UNQUOTED(CONFIG_MANDIR, ["$ac_mandir"], [Default man page path.])


# Enable debugging argument option.
AC_ARG_ENABLE(debug,
  AS_HELP_STRING([--enable-debug], [enable debugging (default=no)]),
  [ac_debug="$enableval"])

# Enable JACK standalone build.
AC_ARG_ENABLE(jack,
  AS_HELP_STRING([--enable-jack], [enable JACK stand-alone build (default=yes)]),
  [ac_jack="$enableval"],
  [ac_jack="yes"])

# Enable JACK session support.
AC_ARG_ENABLE(jack-session,
  AS_HELP_STRING([--enable-jack-session], [enable JACK session support (default=yes)]),
  [ac_jack_session="$enableval"],
  [ac_jack_session="yes"])

# Enable JACK MIDI support option.
AC_ARG_ENABLE(jack-midi,
  AS_HELP_STRING([--enable-jack-midi], [enable JACK MIDI support (default=yes)]),
  [ac_jack_midi="$enableval"],
  [ac_jack_midi="yes"])

# Enable ALSA MIDI support option.
AC_ARG_ENABLE(alsa-midi,
  AS_HELP_STRING([--enable-alsa-midi], [enable ALSA MIDI support (default=yes)]),
  [ac_alsa_midi="$enableval"],
  [ac_alsa_midi="yes"])

# Enable LV2 plugin build.
AC_ARG_ENABLE(lv2,
  AS_HELP_STRING([--enable-lv2], [enable LV2 plug-in build (default=yes)]),
  [ac_lv2="$enableval"],
  [ac_lv2="yes"])

AC_ARG_ENABLE(lv2-ui-x11,
  AS_HELP_STRING([--enable-lv2-ui-x11], [enable LV2 plug-in X11 UI support (default=yes)]),
  [ac_lv2_ui_x11="$enableval"],
  [ac_lv2_ui_x11="yes"])

AC_ARG_ENABLE(lv2-ui-external,
  AS_HELP_STRING([--enable-lv2-ui-external], [enable LV2 plug-in External UI support (default=yes)]),
  [ac_lv2_ui_external="$enableval"],
  [ac_lv2_ui_external="yes"])

AC_ARG_ENABLE(lv2-ui-idle,
  AS_HELP_STRING([--enable-lv2-ui-idle], [enable LV2 UI Idle interface support (default=yes)]),
  [ac_lv2_ui_idle="$enableval"],
  [ac_lv2_ui_idle="yes"])

AC_ARG_ENABLE(lv2-ui-show,
  AS_HELP_STRING([--enable-lv2-ui-show], [enable LV2 UI Show interface support (default=yes)]),
  [ac_lv2_ui_show="$enableval"],
  [ac_lv2_ui_show="yes"])

AC_ARG_ENABLE(lv2-programs,
  AS_HELP_STRING([--enable-lv2-programs], [enable LV2 plug-in Programs support (default=yes)]),
  [ac_lv2_programs="$enableval"],
  [ac_lv2_programs="yes"])


# Enable liblo availability.
AC_ARG_ENABLE(liblo,
  AS_HELP_STRING([--enable-liblo], [enable liblo interface (default=yes)]),
  [ac_liblo="$enableval"],
  [ac_liblo="yes"])

# Enable NSM support.
AC_ARG_ENABLE(nsm,
  AS_HELP_STRING([--enable-nsm], [enable NSM support (default=yes)]),
  [ac_nsm="$enableval"],
  [ac_nsm="yes"])


if test "x$ac_debug" = "xyes"; then
   AC_DEFINE(CONFIG_DEBUG, 1, [Define if debugging is enabled.])
   ac_debug="debug"
else
   ac_debug="release"
fi

AC_SUBST(ac_debug)


# Enable Qt4/5 availability.
AC_ARG_ENABLE(qt4,
  AS_HELP_STRING([--enable-qt4], [enable Qt4 build (default=no)]),
  [ac_qt4="$enableval"],
  [ac_qt4="no"])


# Standard installation base dirs.
ac_path=$PATH
ac_with_paths=""

# Set for alternate Qt4/5 installation dir.
AC_ARG_WITH(qt,
  AS_HELP_STRING([--with-qt=PATH], [use alternate Qt install path]),
  [ac_qt_path="$withval"], [ac_qt_path="no"])

AC_ARG_WITH(qt4,
  AS_HELP_STRING([--with-qt4=PATH], [use alternate Qt4 install path]),
  [ac_qt4_path="$withval"], [ac_qt4_path="no"])

AC_ARG_WITH(qt5,
  AS_HELP_STRING([--with-qt5=PATH], [use alternate Qt5 install path]),
  [ac_qt5_path="$withval"], [ac_qt5_path="no"])

if test "x$ac_qt_path" != "xno"; then
   ac_path="$ac_qt_path/bin:$ac_path"
fi

if test "x$ac_qt4_path" != "xno"; then
   ac_path="$ac_qt4_path/bin:$ac_path"
   ac_qt4="yes"
fi

if test "x$ac_qt5_path" != "xno"; then
   ac_path="$ac_qt5_path/bin:$ac_path"
   ac_qt4="no"
fi

# Set for alternate ALSA installation dir.
AC_ARG_WITH(alsa,
  AS_HELP_STRING([--with-alsa=PATH], [use alternate ALSA install path]),
  [ac_with_paths="$ac_with_paths $withval"])

# Set for alternate JACK installation dir.
AC_ARG_WITH(jack,
  AS_HELP_STRING([--with-jack=PATH], [use alternate JACK install path]),
  [ac_with_paths="$ac_with_paths $withval"])

# Set for alternate LV2 installation dir.
AC_ARG_WITH(lv2,
  AS_HELP_STRING([--with-lv2=PATH], [use alternate LV2 install path]),
  [ac_with_paths="$ac_with_paths $withval"])

# Set for alternate liblo installation dir.
AC_ARG_WITH(liblo,
  AS_HELP_STRING([--with-liblo=PATH], [use alternate liblo path]),
  [ac_with_paths="$ac_with_paths $withval"])


# Honor user specified flags.
ac_cflags=$CFLAGS
ac_ldflags=$LDFLAGS


# Checks for programs.
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP

# Checks for languages.
AC_LANG_CPLUSPLUS

# Check for pkg-config.
PKG_PROG_PKG_CONFIG

# Check whether -std=c++11 support is necessary (4 < g++ version < 6).
AC_CACHE_CHECK([for g++ major version], [ac_cv_gxx_version_major], [
   ac_cv_gxx_version_major=$($CC -dumpversion)
   if test -n "$ac_cv_gxx_version_major"; then
      ac_cv_gxx_version_major=$(echo $ac_cv_gxx_version_major | cut -d'.' -f1);
   fi
   ac_gxx_version_major=$(($ac_cv_gxx_version_major + 0))
])
ac_gxx_version_major=$ac_cv_gxx_version_major
if test $ac_gxx_version_major -ge 4 -a $ac_gxx_version_major -lt 6; then
   CXXFLAGS="-std=c++11 $CXXFLAGS"
fi

# Check for proper flags.
ac_arch=$(uname -m)

# Check for install paths and alternatives...
ac_libdirs="lib"

if test "x$ac_arch" = "xx86_64"; then
   ac_libdirs="$ac_libdirs lib64"
fi

CFLAGS="-fPIC $CFLAGS"
CPPFLAGS="-fPIC $CPPFLAGS"

# Prepend alternate dependencies paths.
for X in $ac_with_paths; do
  if test -d $X/bin; then
     ac_path="$X/bin:$ac_path"
  fi
  if test -d $X/include; then
     CFLAGS="-I$X/include $CFLAGS "
     CPPFLAGS="-I$X/include $CPPFLAGS"
     ac_incpath="$X/include $ac_incpath"
  fi
  for Y in $ac_libdirs; do
     if test -d $X/$Y; then
        LIBS="-L$X/$Y $LIBS"
        ac_libs="-L$X/$Y $ac_libs"
     fi
  done
done

# A common error message:
ac_errmsg="not found in current PATH. Maybe QT development environment isn't available."

# Check for proper qmake path/version alternatives.
AC_PATH_TOOL(ac_qmake, qmake, [no], $ac_path)

if test "x$ac_qmake" = "xno"; then
   if test "x$ac_qt4" = "xyes"; then
      AC_PATH_TOOL(ac_cv_qmake, qmake-qt4, [no], $ac_path)
      ac_qmake=$ac_cv_qmake
   else
      AC_PATH_TOOL(ac_cv_qmake, qmake-qt5, [no], $ac_path)
      ac_qmake=$ac_cv_qmake
   fi
fi

# Check for proper Qt major version.
AC_CACHE_CHECK([for Qt major version], [ac_cv_qt_version_major], [
   ac_cv_qt_version_major=$($ac_qmake -query QT_VERSION | cut -d'.' -f1)
   ac_cv_qt_version_major=$(($ac_cv_qt_version_major + 0))
])
ac_qt_version_major=$ac_cv_qt_version_major
if test "x$ac_qt4" = "xyes"; then
   if test $ac_qt_version_major -ne 4; then
      AC_PATH_TOOL(ac_cv_qmake, qmake-qt4, [no], $ac_path)
      ac_qmake=$ac_cv_qmake
   fi
else
   if test $ac_qt_version_major -ne 5; then
      AC_PATH_TOOL(ac_cv_qmake, qmake-qt5, [no], $ac_path)
      ac_qmake=$ac_cv_qmake
   fi
fi

if test "x$ac_qmake" = "xno"; then
   if test "x$ac_qt4" = "xyes"; then
      AC_MSG_ERROR([qmake-qt4 $ac_errmsg (qt4-devel)])
   else
      AC_MSG_ERROR([qmake-qt5 $ac_errmsg (qt5-devel)])
   fi
fi

# Check for proper Qt install path.
AC_CACHE_CHECK([for Qt install path], [ac_cv_qt_install_path], [
   ac_cv_qt_install_path=$($ac_qmake -query QT_INSTALL_BINS)
])
ac_qt_install_path=$ac_cv_qt_install_path
if test -d $ac_qt_install_path; then
   ac_path="$ac_qt_install_path:$ac_path"
fi

# Check it again, now with updated PATH, just in case...
AC_PATH_TOOL(ac_cv_qmake, qmake, [no], $ac_path)
ac_qmake=$ac_cv_qmake
if test "x$ac_qmake" = "xno"; then
   AC_MSG_ERROR([qmake $ac_errmsg])
fi

AC_SUBST(ac_qmake)

AC_CACHE_CHECK([for Qt install headers], [ac_cv_qt_install_headers], [
   ac_cv_qt_install_headers=$($ac_qmake -query QT_INSTALL_HEADERS)
])
ac_qt_install_headers=$ac_cv_qt_install_headers
if test -d $ac_qt_install_headers; then
   CFLAGS="-I$ac_qt_install_headers $CFLAGS "
   CPPFLAGS="-I$ac_qt_install_headers $CPPFLAGS"
   ac_incpath="$ac_qt_install_headers $ac_incpath"
fi

AC_CACHE_CHECK([for Qt install libraries], [ac_cv_qt_install_libs], [
   ac_cv_qt_install_libs=$($ac_qmake -query QT_INSTALL_LIBS)
])
ac_qt_install_libs=$ac_cv_qt_install_libs
if test -d $ac_qt_install_libs; then
   LIBS="-L$ac_qt_install_libs $LIBS"
   ac_libs="-L$ac_qt_install_libs $ac_libs"
fi

# Finally, check for proper Qt4/5 version.
if test "x$ac_qt4" = "xyes"; then
   AC_CACHE_CHECK([for Qt library version >= 4.4],
      ac_cv_qtversion, [
      AC_TRY_COMPILE([#include "QtCore/qglobal.h"], [
         #if QT_VERSION < 0x040400 || QT_VERSION >= 0x050000
         #error Qt library 4.4 or greater required.
         #endif
      ], ac_cv_qtversion="yes", [
         echo "no; Qt 4.4 or greater is required"
         exit 1
      ])
   ])
else
   AC_CACHE_CHECK([for Qt library version >= 5.1],
      ac_cv_qtversion, [
      AC_TRY_COMPILE([#include "QtCore/qglobal.h"], [
         #if QT_VERSION < 0x050100 || QT_VERSION >= 0x060000
         #error Qt library 5.1 or greater required.
         #endif
      ], ac_cv_qtversion="yes", [
         echo "no; Qt 5.1 or greater is required"
         exit 1
      ])
   ])
fi

# Check for Qt moc utility.
AC_PATH_TOOL(ac_moc, moc, [no], $ac_path)
if test "x$ac_moc" = "xno"; then
   AC_MSG_ERROR([moc $ac_errmsg])
fi
AC_SUBST(ac_moc)

# Check for Qt uic utility.
AC_PATH_TOOL(ac_uic, uic, [no], $ac_path)
if test "x$ac_uic" = "xno"; then
   AC_MSG_ERROR([uic $ac_errmsg])
fi
AC_SUBST(ac_uic)

# Check for Qt lupdate utility.
AC_PATH_TOOL(ac_lupdate, lupdate, [no], $ac_path)
if test "x$ac_lupdate" = "xno"; then
   AC_MSG_ERROR([lupdate $ac_errmsg])
fi
AC_SUBST(ac_lupdate)

# Check for Qt lrelease utility.
AC_PATH_TOOL(ac_lrelease, lrelease, [no], $ac_path)
if test "x$ac_release" = "xno"; then
   AC_MSG_ERROR([lrelease $ac_errmsg])
fi
AC_SUBST(ac_lrelease)


# Checks for libraries.
AC_CHECK_LIB(m, main)
AC_CHECK_LIB(X11, main)
AC_CHECK_LIB(Xext, main)

ac_cflags="-ffast-math $ac_cflags"

ac_jack_cflags="$ac_cflags"
ac_jack_ldflags="$ac_ldflags"
ac_jack_incpath="$ac_incpath"
ac_jack_libs="$ac_libs"

ac_lv2_cflags="$ac_cflags"
ac_lv2_ldflags="$ac_ldflags"
ac_lv2_incpath="$ac_incpath"
ac_lv2_libs="$ac_libs"

ac_all_targets=""
ac_install_targets=""
ac_uninstall_targets=""
ac_clean_targets=""


# Check for JACK libraries.
if test "x$ac_jack" = "xyes"; then
   PKG_CHECK_MODULES([JACK], [jack >= 0.100.0], [ac_jack="yes"], [ac_jack="no"])
   if test "x$ac_jack" = "xyes"; then
      AC_DEFINE(CONFIG_JACK, 1, [Define if JACK library is available.])
      ac_jack_cflags="$ac_jack_cflags $JACK_CFLAGS"
      ac_jack_libs="$ac_jack_libs $JACK_LIBS"
      CFLAGS="$CFLAGS $JACK_CFLAGS"
      CPPFLAGS="$CPPFLAGS $JACK_CFLAGS"
      LIBS="$LIBS $JACK_LIBS"
   else
      AC_MSG_WARN([*** JACK library not found.])
      AC_MSG_WARN([*** JACK stand-alone build will be disabled.])
   fi
fi

if test "x$ac_jack" = "xyes"; then
   # Check for JACK session event callback availability.
   if test "x$ac_jack_session" = "xyes"; then
      AC_CHECK_LIB(jack, jack_set_session_callback, [ac_jack_session="yes"], [ac_jack_session="no"])
   else
      AC_MSG_WARN([*** JACK session support will be disabled.])
   fi
   # Check for ALSA libraries.
   if test "x$ac_alsa_midi" = "xyes"; then
      PKG_CHECK_MODULES([ALSA], [alsa], [ac_alsa_midi="yes"], [ac_alsa_midi="no"])
   fi
   if test "x$ac_alsa_midi" = "xyes"; then
      AC_CHECK_LIB(asound, main, [ac_alsa_midi="yes"], [ac_alsa_midi="no"])
      if test "x$ac_alsa_midi" = "xyes"; then
         AC_DEFINE(CONFIG_ALSA_MIDI, 1, [Define if ALSA MIDI support is enabled.])
         ac_jack_cflags="$ac_jack_cflags $ALSA_CFLAGS"
         ac_jack_libs="$ac_jack_libs $ALSA_LIBS"
      else
         AC_MSG_WARN([*** ALSA library not found.])
         AC_MSG_WARN([*** ALSA MIDI support will be disabled.])
      fi
   fi
fi

# Check for JACK build support.
if test "x$ac_jack" = "xyes"; then
   ac_all_targets="jack $ac_all_targets"
   ac_install_targets="install_jack $ac_install_targets"
   ac_uninstall_targets="uninstall_jack $ac_uninstall_targets"
   ac_clean_targets="clean_jack $ac_clean_targets"
else
  ac_jack_session="no"
  ac_jack_midi="no"
  ac_alsa_midi="no"
  ac_liblo="no"
  ac_nsm="no"
fi

# Check for optional LIBLO library.
if test "x$ac_liblo" = "xyes"; then
   PKG_CHECK_MODULES([LIBLO], [liblo], [ac_liblo="yes"], [ac_liblo="no"])
   if test "x$ac_liblo" = "xyes"; then
      AC_DEFINE(CONFIG_LIBLO, 1, [Define if LIBLO library is available.])
      ac_jack_cflags="$ac_jack_cflags $LIBLO_CFLAGS"
      ac_jack_libs="$ac_jack_libs $LIBLO_LIBS"
   else
      AC_MSG_WARN([*** LIBLO library not found.])
      AC_MSG_WARN([*** OSC service support will be disabled.])
      ac_nsm="no"
   fi
else
   ac_nsm="no"
fi


# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h sys/ioctl.h sys/stat.h unistd.h signal.h)


# Check for JACK session headers availability.
if test "x$ac_jack_session" = "xyes"; then
   AC_CHECK_HEADER(jack/session.h, [ac_jack_session="yes"], [ac_jack_session="no"])
   if test "x$ac_jack_session" = "xyes"; then
      AC_DEFINE(CONFIG_JACK_SESSION, 1, [Define if JACK session support is available.])
   else
      AC_MSG_WARN([*** JACK session header file not found.])
      AC_MSG_WARN([*** JACK session support will be disabled.])
   fi
fi

# Check for JACK MIDI headers availability.
if test "x$ac_jack_midi" = "xyes"; then
   AC_CHECK_HEADER(jack/midiport.h, [ac_jack_midi="yes"], [ac_jack_midi="no"])
   if test "x$ac_jack_midi" = "xyes"; then
      AC_DEFINE(CONFIG_JACK_MIDI, 1, [Define if JACK MIDI support is enabled.])
   else
      AC_MSG_WARN([*** JACK MIDI header files not found.])
      AC_MSG_WARN([*** JACK MIDI support will be disabled.])
   fi
fi

# Check for LV2 headers.
if test "x$ac_lv2" = "xyes"; then
   AC_CHECK_HEADERS(lv2.h \
      lv2/lv2plug.in/ns/ext/atom/atom.h \
      lv2/lv2plug.in/ns/ext/atom/forge.h \
      lv2/lv2plug.in/ns/ext/atom/util.h,
      [ac_lv2="yes"], [ac_lv2="no"])
   if test "x$ac_lv2" = "xyes"; then
      AC_DEFINE(CONFIG_LV2, 1, [Define if LV2 plug-in build is enabled.])
      ac_all_targets="lv2 $ac_all_targets"
      ac_install_targets="install_lv2 $ac_install_targets"
      ac_uninstall_targets="uninstall_lv2 $ac_uninstall_targets"
      ac_clean_targets="clean_lv2 $ac_clean_targets"
      AC_CACHE_CHECK([for lv2_atom_forge_object],
         ac_cv_lv2_atom_forge_object, [
         AC_TRY_LINK([#include "lv2/lv2plug.in/ns/ext/atom/forge.h"], [
            // Checking for lv2_atom_forge_object...
            LV2_Atom_Forge *forge;
            LV2_Atom_Forge_Frame *frame;
            lv2_atom_forge_object(forge, frame, 0, 101);
         ], ac_cv_lv2_atom_forge_object="yes", ac_cv_lv2_atom_forge_object="no")
      ])
      ac_lv2_atom_forge_object=$ac_cv_lv2_atom_forge_object
      if test "x$ac_lv2_atom_forge_object" = "xyes"; then
         AC_DEFINE(CONFIG_LV2_ATOM_FORGE_OBJECT, 1, [Define if lv2_atom_forge_object is available.])
      fi
      AC_CACHE_CHECK([for lv2_atom_forge_key],
         ac_cv_lv2_atom_forge_key, [
         AC_TRY_LINK([#include "lv2/lv2plug.in/ns/ext/atom/forge.h"], [
            // Checking for lv2_atom_forge_key...
            LV2_Atom_Forge *forge;
            lv2_atom_forge_key(forge, 102);
         ], ac_cv_lv2_atom_forge_key="yes", ac_cv_lv2_atom_forge_key="no")
      ])
      ac_lv2_atom_forge_key=$ac_cv_lv2_atom_forge_key
      if test "x$ac_lv2_atom_forge_key" = "xyes"; then
         AC_DEFINE(CONFIG_LV2_ATOM_FORGE_KEY, 1, [Define if lv2_atom_forge_key is available.])
      fi
   else
      AC_MSG_WARN([*** LV2 header files not found.])
      AC_MSG_WARN([*** LV2 plug-in build will be disabled.])
   fi
fi

if test "x$ac_lv2" = "xno"; then
   ac_lv2_ui_x11="no"
   ac_lv2_ui_external="no"
   ac_lv2_ui_idle="no"
   ac_lv2_ui_show="no"
   ac_lv2_programs="no"
fi

if test "x$ac_qt4" = "xyes"; then
   ac_lv2_ui_x11="no"
fi

if test "x$ac_lv2_ui_x11" = "xyes"; then
   AC_DEFINE(CONFIG_LV2_UI_X11, 1, [Define if LV2 X11 UI support is available.])
else
   AC_MSG_WARN([*** LV2 X11 UI support will be disabled.])
fi

if test "x$ac_lv2_ui_external" = "xyes"; then
   CFLAGS="$CFLAGS -Isrc/lv2"
   CPPFLAGS="$CPPFLAGS -Isrc/lv2"
   AC_CHECK_HEADER(lv2_external_ui.h, [ac_lv2_ui_external="yes"], [ac_lv2_ui_external="no"])
   if test "x$ac_lv2_ui_external" = "xyes"; then
      AC_DEFINE(CONFIG_LV2_UI_EXTERNAL, 1, [Define if LV2 External UI extension is available.])
      ac_lv2_incpath="lv2 $ac_lv2_incpath"
   else
      AC_MSG_WARN([*** LV2 External UI extension will be disabled.])
   fi
fi

if test "x$ac_lv2_ui_idle" = "xyes"; then
   AC_CACHE_CHECK([for LV2 UI Idle interface],
      ac_cv_lv2_ui_idle, [
      AC_TRY_LINK([#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"], [
         // Checking for LV2 UI Idle interface...
         LV2UI_Idle_Interface idle;
      ], ac_cv_lv2_ui_idle="yes", ac_cv_lv2_ui_idle="no")
   ])
   ac_lv2_ui_idle=$ac_cv_lv2_ui_idle
   if test "x$ac_lv2_ui_idle" = "xyes"; then
      AC_DEFINE(CONFIG_LV2_UI_IDLE, 1, [Define if LV2 UI Idle interface support is available.])
   else
      AC_MSG_WARN([*** LV2 UI Idle interface support will be disabled.])
   fi
fi

if test "x$ac_lv2_ui_show" = "xyes"; then
   AC_CACHE_CHECK([for LV2 UI Show interface],
      ac_cv_lv2_ui_show, [
      AC_TRY_LINK([#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"], [
         // Checking for LV2 UI Show interface...
         LV2UI_Show_Interface show;
      ], ac_cv_lv2_ui_show="yes", ac_cv_lv2_ui_show="no")
   ])
   ac_lv2_ui_show=$ac_cv_lv2_ui_show
   if test "x$ac_lv2_ui_show" = "xyes"; then
      AC_DEFINE(CONFIG_LV2_UI_SHOW, 1, [Define if LV2 UI Show interface support is available.])
   else
      AC_MSG_WARN([*** LV2 UI Show interface support will be disabled.])
   fi
fi

if test "x$ac_lv2_programs" = "xyes"; then
   AC_CHECK_HEADER(lv2_programs.h, [ac_lv2_programs="yes"], [ac_lv2_programs="no"])
   if test "x$ac_lv2_programs" = "xyes"; then
      AC_DEFINE(CONFIG_LV2_PROGRAMS, 1, [Define if LV2 Programs extension is available.])
      ac_lv2_incpath="lv2 $ac_lv2_incpath"
   else
      AC_MSG_WARN([*** LV2 Programs extension will be disabled.])
   fi
fi

# Check for NSM support.
if test "x$ac_nsm" = "xyes"; then
   AC_DEFINE(CONFIG_NSM, 1, [Define if NSM support is available.])
fi


# Checks for build targets
if test "x$ac_jack" = "xno" -a "x$ac_lv2" = "xno"; then
   AC_MSG_ERROR([*** JACK and LV2 build options disabled.])
fi

if test "x$ac_jack" = "xyes"; then
   if test "x$ac_jack_midi" = "xno" -a "x$ac_alsa_midi" = "xno"; then
      AC_MSG_ERROR([*** JACK MIDI and ALSA MIDI support disabled.])
   fi
fi

AC_SUBST(ac_cflags)
AC_SUBST(ac_ldflags)
AC_SUBST(ac_incpath)
AC_SUBST(ac_libs)

AC_SUBST(ac_jack_cflags)
AC_SUBST(ac_jack_ldflags)
AC_SUBST(ac_jack_incpath)
AC_SUBST(ac_jack_libs)

AC_SUBST(ac_lv2_cflags)
AC_SUBST(ac_lv2_ldflags)
AC_SUBST(ac_lv2_incpath)
AC_SUBST(ac_lv2_libs)

AC_SUBST(ac_all_targets)
AC_SUBST(ac_install_targets)
AC_SUBST(ac_uninstall_targets)
AC_SUBST(ac_clean_targets)


# Checks for typedefs, structures, and compiler characteristics.
# AC_C_CONST

# Finally produce a configure header file and the makefiles.
AC_OUTPUT

# make clean > /dev/null 2>&1

# Output summary message

echo
echo "  $PACKAGE_NAME $ac_build_version"
echo
echo "  Build target . . . . . . . . . . . . . . . . . . .: $ac_debug"
echo
echo "  JACK stand-alone build . . . . . . . . . . . . . .: $ac_jack"
echo "  JACK session support . . . . . . . . . . . . . . .: $ac_jack_session"
echo "  JACK MIDI support  . . . . . . . . . . . . . . . .: $ac_jack_midi"
echo "  ALSA MIDI support  . . . . . . . . . . . . . . . .: $ac_alsa_midi"
echo "  LV2 plug-in build  . . . . . . . . . . . . . . . .: $ac_lv2"
echo "  LV2 plug-in X11 UI support   . . . . . . . . . . .: $ac_lv2_ui_x11"
echo "  LV2 plug-in External UI support  . . . . . . . . .: $ac_lv2_ui_external"
echo "  LV2 plug-in UI Idle interface support  . . . . . .: $ac_lv2_ui_idle"
echo "  LV2 plug-in UI Show interface support  . . . . . .: $ac_lv2_ui_show"
echo "  LV2 plug-in Programs support . . . . . . . . . . .: $ac_lv2_programs"
echo "  OSC service support (liblo)  . . . . . . . . . . .: $ac_liblo"
echo "  NSM (Non Session Management) support . . . . . . .: $ac_nsm"
echo
echo "  Install prefix . . . . . . . . . . . . . . . . . .: $ac_prefix"
echo
echo "Now type 'make', followed by 'make install' as root."
echo
