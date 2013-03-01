# Process this file with autoconf to produce a configure script.
AC_INIT(synthv1, 0.3.2, rncbc@rncbc.org)

AC_CONFIG_SRCDIR(src/synthv1.cpp)
AC_CONFIG_HEADERS(src/config.h)
AC_CONFIG_FILES(Makefile synthv1.spec src/src.pri src/synthv1.desktop)

# Set default installation prefix.
AC_PREFIX_DEFAULT(/usr/local)
ac_prefix=$prefix
if test "x$ac_prefix" = "xNONE"; then
    ac_prefix=$ac_default_prefix
fi
AC_SUBST(ac_prefix)

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


if test "x$ac_debug" = "xyes"; then
   AC_DEFINE(CONFIG_DEBUG, 1, [Define if debugging is enabled.])
   ac_debug="debug"
else
   ac_debug="release"
fi

AC_SUBST(ac_debug)


# Standard installation base dirs.
ac_with_paths="/usr /usr/local"

# Some a-la-debian alternatives...
for X in /usr/lib /usr/lib64 /usr/share; do
  for Y in qt qt4; do
    if test -d $X/$Y/bin; then
      ac_with_paths="$ac_with_paths $X/$Y"
    fi
  done
done

# Set for alternate Qt installation dir.
AC_ARG_WITH(qt,
  AC_HELP_STRING([--with-qt=PATH], [use alternate Qt install path]),
  [ac_with_paths="$ac_with_paths $withval"])

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


# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_GCC_TRADITIONAL

# Checks for languages.
AC_LANG_C
AC_LANG_CPLUSPLUS

# Prepend alternate dependencies paths.
ac_path=$PATH
for X in $ac_with_paths; do
  if test -d $X/bin; then
    ac_path="$X/bin:$ac_path"
  fi
  if test -x $X/qmake; then
    ac_path="$X:$ac_path"
  fi
  if test -d $X/include; then
    for Y in qt qt4; do
      if test -d $X/include/$Y; then
        CFLAGS="-I$X/include/$Y $CFLAGS"
        CPPFLAGS="-I$X/include/$Y $CPPFLAGS"
        ac_incpath="$X/include/$Y $ac_incpath"
      fi
    done
    CFLAGS="-I$X/include $CFLAGS"
    CPPFLAGS="-I$X/include $CPPFLAGS"
    ac_incpath="$X/include $ac_incpath"
  fi
  if test -d $X/lib64; then
    LIBS="-L$X/lib64 $LIBS"
    ac_libs="-L$X/lib64 $ac_libs"
  fi
  if test -d $X/lib; then
    LIBS="-L$X/lib $LIBS"
    ac_libs="-L$X/lib $ac_libs"
  fi
done


# Check for proper Qt version.
AC_CACHE_CHECK([for Qt library version >= 4.4],
   ac_cv_qtversion, [
   AC_TRY_LINK([#include "QtCore/qglobal.h"], [
      #if QT_VERSION < 0x040400
      #error Qt library 4.4 or greater required.
      #endif
   ], ac_cv_qtversion="yes", [
      echo "no; Qt 4.4 or greater is required"
      exit 1
   ])
])


ac_cflags="-ffast-math $ac_cflags"
AC_SUBST(ac_cflags)


# A common error message:
ac_errmsg="not found in current PATH. Maybe QT development environment isn't available (qt-devel)."

# Check for Qt qmake utility.
AC_PATH_PROG(ac_qmake, qmake, [no], $ac_path)
if test "x$ac_qmake" = "xno"; then
   AC_MSG_ERROR([qmake $ac_errmsg])
fi
AC_SUBST(ac_qmake)

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


# Checks for libraries.
AC_CHECK_LIB(m, main)
AC_CHECK_LIB(X11, main)
AC_CHECK_LIB(Xext, main)

# Check for JACK libraries.
if test "x$ac_jack" = "xyes"; then
   AC_CHECK_LIB(jack, main, [ac_jack="yes"], [ac_jack="no"])
   if test "x$ac_jack" = "xno"; then
      AC_MSG_WARN([*** JACK library not found.])
      AC_MSG_WARN([*** JACK stand-alone build will be disabled.])
   else
      ac_libs="$ac_libs -ljack"
   fi
else
  ac_jack_session="no"
  ac_jack_midi="no"
  ac_alsa_midi="no"
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
         ac_libs="$ac_libs -lasound -lpthread"
      fi
   fi
fi

AC_SUBST(ac_libs)


# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h sys/ioctl.h sys/stat.h unistd.h signal.h)

# Check for JACK headers.
if test "x$ac_jack" = "xyes"; then
   AC_CHECK_HEADERS(jack/jack.h, [ac_jack="yes"], [ac_jack="no"])
   if test "x$ac_jack" = "xno"; then
      AC_MSG_WARN([*** JACK header files not found.])
      AC_MSG_WARN([*** JACK stand-alone build will be disabled.])
   else
      AC_DEFINE(CONFIG_JACK, 1, [Define if JACK stand-alone build is enabled.])
   fi
else
  ac_jack_session="no"
  ac_jack_midi="no"
  ac_alsa_midi="no"
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
   fi
fi

AC_SUBST(ac_incpath)


# Checks for build targets
if test "x$ac_jack" = "xno" -a "x$ac_lv2" = "xno"; then
   AC_MSG_ERROR([*** JACK and LV2 build options disabled.])
fi

if test "x$ac_jack" = "xyes"; then
   if test "x$ac_jack_midi" = "xno" -a "x$ac_alsa_midi" = "xno"; then
      AC_MSG_ERROR([*** JACK MIDI and ALSA MIDI support disabled.])
   fi
fi


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
echo "  LV2 plugin-in build  . . . . . . . . . . . . . . .: $ac_lv2"
echo
echo "  Install prefix . . . . . . . . . . . . . . . . . .: $ac_prefix"
echo
echo "Now type 'make', followed by 'make install' as root."
echo
