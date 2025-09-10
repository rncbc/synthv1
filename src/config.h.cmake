#ifndef CONFIG_H
#define CONFIG_H

/* Define to the name of this package. */
#cmakedefine PROJECT_NAME "@PROJECT_NAME@"

/* Define to the version of this package. */
#cmakedefine PROJECT_VERSION "@PROJECT_VERSION@"

/* Define to the description of this package. */
#cmakedefine PROJECT_DESCRIPTION "@PROJECT_DESCRIPTION@"

/* Define to the homepage of this package. */
#cmakedefine PROJECT_HOMEPAGE_URL "@PROJECT_HOMEPAGE_URL@"

/* Define to the copyright of this package. */
#cmakedefine PROJECT_COPYRIGHT "@PROJECT_COPYRIGHT@"

/* Define to the domain of this package. */
#cmakedefine PROJECT_DOMAIN "@PROJECT_DOMAIN@"


/* Default installation prefix. */
#cmakedefine CONFIG_PREFIX "@CONFIG_PREFIX@"

/* Define to target installation dirs. */
#cmakedefine CONFIG_BINDIR "@CONFIG_BINDIR@"
#cmakedefine CONFIG_LIBDIR "@CONFIG_LIBDIR@"
#cmakedefine CONFIG_DATADIR "@CONFIG_DATADIR@"
#cmakedefine CONFIG_MANDIR "@CONFIG_MANDIR@"

/* Define if debugging is enabled. */
#cmakedefine CONFIG_DEBUG @CONFIG_DEBUG@

/* Define to 1 if you have the <signal.h> header file. */
#cmakedefine HAVE_SIGNAL_H @HAVE_SIGNAL_H@

/* Define if JACK library is available. */
#cmakedefine CONFIG_JACK @CONFIG_JACK@

/* Define if ALSA MIDI support is enabled. */
#cmakedefine CONFIG_ALSA_MIDI @CONFIG_ALSA_MIDI@

/* Define if LIBLO library is available. */
#cmakedefine CONFIG_LIBLO @CONFIG_LIBLO@

/* Define if JACK session support is available. */
#cmakedefine CONFIG_JACK_SESSION @CONFIG_JACK_SESSION@

/* Define if JACK MIDI support is enabled. */
#cmakedefine CONFIG_JACK_MIDI @CONFIG_JACK_MIDI@

/* Define if LV2 plug-in build is enabled. */
#cmakedefine CONFIG_LV2 @CONFIG_LV2@

/* Define if LV2 old headers are enabled. */
#cmakedefine CONFIG_LV2_OLD_HEADERS @CONFIG_LV2_OLD_HEADERS@

/* Define if lv2_atom_forge_object is available. */
#cmakedefine CONFIG_LV2_ATOM_FORGE_OBJECT @CONFIG_LV2_ATOM_FORGE_OBJECT@

/* Define if lv2_atom_forge_key is available. */
#cmakedefine CONFIG_LV2_ATOM_FORGE_KEY @CONFIG_LV2_ATOM_FORGE_KEY@

/* Define if LV2 X11 UI support is available. */
#cmakedefine CONFIG_LV2_UI_X11 @CONFIG_LV2_UI_X11@

/* Define if LV2 Windows UI support is available. */
#cmakedefine CONFIG_LV2_UI_WINDOWS @CONFIG_LV2_UI_WINDOWS@

/* Define if LV2 External UI extension is available. */
#cmakedefine CONFIG_LV2_UI_EXTERNAL @CONFIG_LV2_UI_EXTERNAL@

/* Define if LV2 UI Idle interface support is available. */
#cmakedefine CONFIG_LV2_UI_IDLE @CONFIG_LV2_UI_IDLE@

/* Define if LV2 UI Show interface support is available. */
#cmakedefine CONFIG_LV2_UI_SHOW @CONFIG_LV2_UI_SHOW@

/* Define if LV2 UI Resize interface support is available. */
#cmakedefine CONFIG_LV2_UI_RESIZE @CONFIG_LV2_UI_RESIZE@

/* Define if LV2 Programs extension is available. */
#cmakedefine CONFIG_LV2_PROGRAMS @CONFIG_LV2_PROGRAMS@

/* Define if LV2 Patch is supported. */
#cmakedefine CONFIG_LV2_PATCH @CONFIG_LV2_PATCH@

/* Define if LV2 Port-event is supported. */
#cmakedefine CONFIG_LV2_PORT_EVENT @CONFIG_LV2_PORT_EVENT@

/* Define if LV2 Port-change request is supported. */
#cmakedefine CONFIG_LV2_PORT_CHANGE_REQUEST @CONFIG_LV2_PORT_CHANGE_REQUEST@

/* Define if NSM support is available. */
#cmakedefine CONFIG_NSM @CONFIG_NSM@


#endif /* CONFIG_H */
