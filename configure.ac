# Process this file with autoconf to produce a configure script.
AC_INIT(synthv1, 0.7.4.5, rncbc@rncbc.org)

AC_CONFIG_SRCDIR(src/synthv1.cpp)
AC_CONFIG_HEADERS(src/config.h)
AC_CONFIG_FILES(Makefile synthv1.spec src/src_jack.pri src/src_lv2.pri src/synthv1.desktop)

# Set default installation prefix.
AC_PREFIX_DEFAULT(/usr/local)
ac_prefix=$prefix
if test "x$ac_prefix" = "xNONE"; then
   ac_prefix=$ac_default_prefix
fi
AC_SUBST(ac_prefix)
AC_DEFINE_UNQUOTED(CONFIG_PREFIX, ["$ac_prefix"], [Default installation prefix.]) 

# Set default library/plugin prefix.
if test "x$exec_prefix" = "xNONE"; then
   exec_prefix=$ac_prefix
fi
eval ac_libdir=$libdir
AC_SUBST(ac_libdir)
AC_DEFINE_UNQUOTED(CONFIG_LIBDIR, ["$ac_libdir"], [Default library path.])

# Enable debugging argument option.
AC_ARG_ENABLE(debug,
  AC_HELP_STRING([--enable-debug], [enable debugging (default=no)]),
  [ac_debug="$enableval"])

# Enable JACK standalone build.
AC_ARG_ENABLE(jack,
  AC_HELP_STRING([--enable-jack], [enable JACK stand-alone build (default=yes)]),
  [ac_jack="$enableval"],
  [ac_jack="yes"])

# Enable JACK session support.
AC_ARG_ENABLE(jack-session,
  AC_HELP_STRING([--enable-jack-session], [enable JACK session support (default=yes)]),
  [ac_jack_session="$enableval"],
  [ac_jack_session="yes"])

# Enable JACK MIDI support option.
AC_ARG_ENABLE(jack-midi,
  AC_HELP_STRING([--enable-jack-midi], [enable JACK MIDI support (default=yes)]),
  [ac_jack_midi="$enableval"],
  [ac_jack_midi="yes"])

# Enable ALSA MIDI support option.
AC_ARG_ENABLE(alsa-midi,
  AC_HELP_STRING([--enable-alsa-midi], [enable ALSA MIDI support (default=yes)]),
  [ac_alsa_midi="$enableval"],
  [ac_alsa_midi="yes"])

# Enable LV2 plugin build.
AC_ARG_ENABLE(lv2,
  AC_HELP_STRING([--enable-lv2], [enable LV2 plug-in build (default=yes)]),
  [ac_lv2="$enableval"],
  [ac_lv2="yes"])

AC_ARG_ENABLE(lv2-external-ui,
  AC_HELP_STRING([--enable-lv2-external-ui], [enable LV2 plug-in External UI support (default=yes)]),
  [ac_lv2_external_ui="$enableval"],
  [ac_lv2_external_ui="yes"])

AC_ARG_ENABLE(lv2-ui-idle,
  AC_HELP_STRING([--enable-lv2-ui-idle], [enable LV2 UI Idle interface support (default=yes)]),
  [ac_lv2_ui_idle="$enableval"],
  [ac_lv2_ui_idle="yes"])

AC_ARG_ENABLE(lv2-ui-show,
  AC_HELP_STRING([--enable-lv2-ui-show], [enable LV2 UI Show interface support (default=yes)]),
  [ac_lv2_ui_show="$enableval"],
  [ac_lv2_ui_show="yes"])

AC_ARG_ENABLE(lv2-programs,
  AC_HELP_STRING([--enable-lv2-programs], [enable LV2 plug-in Programs support (default=yes)]),
  [ac_lv2_programs="$enableval"],
  [ac_lv2_programs="yes"])


# Enable liblo availability.
AC_ARG_ENABLE(liblo,
  AC_HELP_STRING([--enable-liblo], [enable liblo interface (default=yes)]),
  [ac_liblo="$enableval"],
  [ac_liblo="yes"])

# Enable NSM support.
AC_ARG_ENABLE(nsm,
  AC_HELP_STRING([--enable-nsm], [enable NSM support (default=yes)]),
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
  AC_HELP_STRING([--enable-qt4], [enable Qt4 build (default=no)]),
  [ac_qt4="$enableval"],
  [ac_qt4="no"])


# Standard installation base dirs.
ac_with_paths="/usr /usr/local"

# Set for alternate Qt4/5 installation dir.
AC_ARG_WITH(qt4,
  AC_HELP_STRING([--with-qt4=PATH], [use alternate Qt4 install path]),
  [ac_qt4_path="$withval"], [ac_qt4_path="no"])

AC_ARG_WITH(qt5,
  AC_HELP_STRING([--with-qt5=PATH], [use alternate Qt5 install path]),
  [ac_qt5_path="$withval"], [ac_qt5_path="no"])

if test "x$ac_qt4_path" != "xno"; then
   ac_with_paths="$ac_with_paths $ac_qt4_path"
   ac_qt4="yes"
fi

if test "x$ac_qt5_path" != "xno"; then
   ac_with_paths="$ac_with_paths $ac_qt5_path"
   ac_qt4="no"
fi

# Set for alternate ALSA installation dir.
AC_ARG_WITH(alsa,
  AC_HELP_STRING([--with-alsa=PATH], [use alternate ALSA install path]),
  [ac_with_paths="$ac_with_paths $withval"])

# Set for alternate JACK installation dir.
AC_ARG_WITH(jack,
  AC_HELP_STRING([--with-jack=PATH], [use alternate JACK install path]),
  [ac_with_paths="$ac_with_paths $withval"])

# Set for alternate LV2 installation dir.
AC_ARG_WITH(lv2,
  AC_HELP_STRING([--with-lv2=PATH], [use alternate LV2 install path]),
  [ac_with_paths="$ac_with_paths $withval"])

# Set for alternate liblo installation dir.
AC_ARG_WITH(liblo,
  AC_HELP_STRING([--with-liblo=PATH], [use alternate liblo path]),
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


# Check for proper flags.
ac_arch=`uname -m`

# Check for install paths and alternatives...
ac_libdirs="lib"

if test "x$ac_arch" = "xx86_64"; then
   ac_libdirs="$ac_libdirs lib64"
fi

CFLAGS="-fPIC $CFLAGS"
CPPFLAGS="-fPIC $CPPFLAGS"

# Prepend alternate dependencies paths.
ac_path=$PATH

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

if test "x$ac_qt4" = "xyes"; then
   AC_PATH_PROG(ac_qmake, qmake-qt4, [no], $ac_path)
else
   AC_PATH_PROG(ac_qmake, qmake-qt5, [no], $ac_path)
fi
if test "x$ac_qmake" = "xno"; then
   AC_PATH_PROG(ac_cv_qmake, qmake, [no], $ac_path)
   ac_qmake=$ac_cv_qmake
fi
if test "x$ac_qmake" = "xno"; then
   AC_MSG_ERROR([qmake $ac_errmsg])
fi

# Check for proper Qt4/5 major version.
ac_qt_version_major=`$ac_qmake -query QT_VERSION | cut -d. -f1`
if test "x$ac_qt4" = "xyes"; then
   if test "x$ac_qt_version_major" != "x4"; then
      AC_MSG_ERROR([qmake-qt4 $ac_errmsg (qt4-devel)])
   fi
else
   if test "x$ac_qt_version_major" != "x5"; then
      AC_MSG_ERROR([qmake-qt5 $ac_errmsg (qt5-devel)])
   fi
fi

ac_qt_install_bins=`$ac_qmake -query QT_INSTALL_BINS`
if test -d $ac_qt_install_bins; then
   ac_path="$ac_qt_install_bins:$ac_path"
fi

# Check it again, now with updated PATH, just in case...
AC_PATH_PROG(ac_cv_qmake, qmake, [no], $ac_path)
ac_qmake=$ac_cv_qmake
if test "x$ac_qmake" = "xno"; then
   AC_MSG_ERROR([qmake $ac_errmsg])
fi

AC_SUBST(ac_qmake)

ac_qt_install_headers=`$ac_qmake -query QT_INSTALL_HEADERS`
if test -d $ac_qt_install_headers; then
   CFLAGS="-I$ac_qt_install_headers $CFLAGS "
   CPPFLAGS="-I$ac_qt_install_headers $CPPFLAGS"
   ac_incpath="$ac_qt_install_headers $ac_incpath"
fi

ac_qt_install_libs=`$ac_qmake -query QT_INSTALL_LIBS`
if test -d $ac_qt_install_libs; then
   LIBS="-L$ac_qt_install_libs $LIBS"
   ac_libs="-L$ac_qt_install_libs $ac_libs"
fi

# Check for proper Qt4/5 version.
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
AC_PATH_PROG(ac_moc, moc, [no], $ac_path)
if test "x$ac_moc" = "xno"; then
   AC_MSG_ERROR([moc $ac_errmsg])
fi
AC_SUBST(ac_moc)

# Check for Qt uic utility.
AC_PATH_PROG(ac_uic, uic, [no], $ac_path)
if test "x$ac_uic" = "xno"; then
   AC_MSG_ERROR([uic $ac_errmsg])
fi
AC_SUBST(ac_uic)

# Check for Qt lupdate utility.
AC_PATH_PROG(ac_lupdate, lupdate, [no], $ac_path)
if test "x$ac_lupdate" = "xno"; then
   AC_MSG_ERROR([lupdate $ac_errmsg])
fi
AC_SUBST(ac_lupdate)

# Check for Qt lrelease utility.
AC_PATH_PROG(ac_lrelease, lrelease, [no], $ac_path)
if test "x$ac_release" = "xno"; then
   AC_MSG_ERROR([lrelease $ac_errmsg])
fi
AC_SUBST(ac_lrelease)


ac_cflags="-ffast-math $ac_cflags"
AC_SUBST(ac_cflags)


# Checks for libraries.
AC_CHECK_LIB(m, main)
AC_CHECK_LIB(X11, main)
AC_CHECK_LIB(Xext, main)

ac_jack_libs="$ac_libs"
ac_lv2_libs="$ac_libs"

# Check for JACK libraries.
if test "x$ac_jack" = "xyes"; then
   AC_CHECK_LIB(jack, main, [ac_jack="yes"], [ac_jack="no"])
   if test "x$ac_jack" = "xno"; then
      AC_MSG_WARN([*** JACK library not found.])
      AC_MSG_WARN([*** JACK stand-alone build will be disabled.])
   else
      ac_jack_libs="$ac_jack_libs -ljack"
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
      AC_CHECK_LIB(pthread, main, [ac_alsa_midi="yes"], [ac_alsa_midi="no"])
   fi
   if test "x$ac_alsa_midi" = "xyes"; then
      AC_CHECK_LIB(asound, main, [ac_alsa_midi="yes"], [ac_alsa_midi="no"])
      if test "x$ac_alsa_midi" = "xno"; then
         AC_MSG_WARN([*** ALSA library not found.])
         AC_MSG_WARN([*** ALSA MIDI support will be disabled.])
      else
         ac_jack_libs="$ac_jack_libs -lasound -lpthread"
      fi
   fi
fi

# Check for optional liblo library.
if test "x$ac_liblo" = "xyes"; then
   AC_CHECK_LIB(lo, main, [ac_liblo="yes"], [ac_liblo="no"])
   if test "x$ac_liblo" = "xyes"; then
      AC_DEFINE(CONFIG_LIBLO, 1, [Define if liblo is available.])
      ac_jack_libs="$ac_jack_libs -llo"
   else
      AC_MSG_WARN([*** liblo library not found.])
      AC_MSG_WARN([*** OSC service support will be disabled.])
      ac_nsm="no"
   fi
else
   ac_nsm="no"
fi

AC_SUBST(ac_jack_libs)
AC_SUBST(ac_lv2_libs)


# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h sys/ioctl.h sys/stat.h unistd.h signal.h)

ac_jack_incpath="$ac_incpath"
ac_lv2_incpath="$ac_incpath"

ac_all_targets=""
ac_install_targets=""
ac_uninstall_targets=""
ac_clean_targets=""

# Check for JACK headers.
if test "x$ac_jack" = "xyes"; then
   AC_CHECK_HEADERS(jack/jack.h, [ac_jack="yes"], [ac_jack="no"])
   if test "x$ac_jack" = "xno"; then
      AC_MSG_WARN([*** JACK header files not found.])
      AC_MSG_WARN([*** JACK stand-alone build will be disabled.])
   else
      AC_DEFINE(CONFIG_JACK, 1, [Define if JACK stand-alone build is enabled.])
      ac_all_targets="jack $ac_all_targets"
      ac_install_targets="install_jack $ac_install_targets"
      ac_uninstall_targets="uninstall_jack $ac_uninstall_targets"
      ac_clean_targets="clean_jack $ac_clean_targets"
   fi
fi

if test "x$ac_jack" = "xno"; then
  ac_jack_session="no"
  ac_jack_midi="no"
  ac_alsa_midi="no"
  ac_liblo="no"
  ac_nsm="no"
fi

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

# Check for ALSA MIDI headers availability.
if test "x$ac_alsa_midi" = "xyes"; then
   AC_CHECK_HEADER(alsa/asoundlib.h, [ac_alsa_midi="yes"], [ac_alsa_midi="no"])
   if test "x$ac_alsa_midi" = "xyes"; then
      AC_DEFINE(CONFIG_ALSA_MIDI, 1, [Define if ALSA MIDI support is enabled.])
   else
      AC_MSG_WARN([*** ALSA MIDI header files not found.])
      AC_MSG_WARN([*** ALSA MIDI support will be disabled.])
   fi
fi

# Check for LV2 headers.
if test "x$ac_lv2" = "xyes"; then
   AC_CHECK_HEADERS(lv2.h, [ac_lv2="yes"], [ac_lv2="no"])
   if test "x$ac_lv2" = "xno"; then
      AC_MSG_WARN([*** LV2 header files not found.])
   else
      AC_DEFINE(CONFIG_LV2, 1, [Define if LV2 plug-in build is enabled.])
      ac_all_targets="lv2 lv2ui $ac_all_targets"
      ac_install_targets="install_lv2 $ac_install_targets"
      ac_uninstall_targets="uninstall_lv2 $ac_uninstall_targets"
      ac_clean_targets="clean_lv2 $ac_clean_targets"
   fi
fi

if test "x$ac_lv2" = "xno"; then
   lv2_external_ui="no"
   ac_lv2_ui_idle="no"
   ac_lv2_ui_show="no"
   ac_lv2_programs="no"
fi

if test "x$ac_lv2_external_ui" = "xyes"; then
   CFLAGS="$CFLAGS -Isrc/lv2"
   CPPFLAGS="$CPPFLAGS -Isrc/lv2"
   AC_CHECK_HEADER(lv2_external_ui.h, [ac_lv2_external_ui="yes"], [ac_lv2_external_ui="no"])
   if test "x$ac_lv2_external_ui" = "xyes"; then
      AC_DEFINE(CONFIG_LV2_EXTERNAL_UI, 1, [Define if LV2 External UI extension is available.])
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
   else
      AC_MSG_WARN([*** LV2 Programs extension will be disabled.])
   fi
fi

# Check for liblo headers.
if test "x$ac_liblo" = "xyes"; then
   AC_CHECK_HEADER(lo/lo.h, [ac_lo_h="yes"], [ac_lo_h="no"])
   if test "x$ac_lo_h" = "xno"; then
      AC_MSG_ERROR([lo.h header file not found.])
      ac_liblo="no"
      ac_nsm="no"
   fi
fi

AC_SUBST(ac_all_targets)
AC_SUBST(ac_install_targets)
AC_SUBST(ac_uninstall_targets)
AC_SUBST(ac_clean_targets)

AC_SUBST(ac_jack_incpath)
AC_SUBST(ac_lv2_incpath)


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


# Checks for typedefs, structures, and compiler characteristics.
# AC_C_CONST

# Finally produce a configure header file and the makefiles.
AC_OUTPUT

# make clean > /dev/null 2>&1

# Output summary message

echo
echo "  $PACKAGE_NAME $PACKAGE_VERSION"
echo
echo "  Build target . . . . . . . . . . . . . . . . . . .: $ac_debug"
echo
echo "  JACK stand-alone build . . . . . . . . . . . . . .: $ac_jack"
echo "  JACK session support . . . . . . . . . . . . . . .: $ac_jack_session"
echo "  JACK MIDI support  . . . . . . . . . . . . . . . .: $ac_jack_midi"
echo "  ALSA MIDI support  . . . . . . . . . . . . . . . .: $ac_alsa_midi"
echo "  LV2 plug-in build  . . . . . . . . . . . . . . . .: $ac_lv2"
echo "  LV2 plug-in External UI support  . . . . . . . . .: $ac_lv2_external_ui"
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
