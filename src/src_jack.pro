# synthv1_jack.pro
#
NAME = synthv1

TARGET = $${NAME}_jack
TEMPLATE = app

include(src_jack.pri)

HEADERS = \
	config.h \
	synthv1.h \
	synthv1_config.h \
	synthv1_jack.h \
	synthv1_wave.h \
	synthv1_ramp.h \
	synthv1_list.h \
	synthv1_fx.h \
	synthv1_nsm.h \
	synthv1_param.h \
	synthv1_sched.h \
	synthv1_reverb.h \
	synthv1widget.h \
	synthv1widget_env.h \
	synthv1widget_filt.h \
	synthv1widget_wave.h \
	synthv1widget_knob.h \
	synthv1widget_preset.h \
	synthv1widget_status.h \
	synthv1widget_jack.h

SOURCES = \
	synthv1.cpp \
	synthv1_config.cpp \
	synthv1_jack.cpp \
	synthv1_wave.cpp \
	synthv1_nsm.cpp \
	synthv1_param.cpp \
	synthv1_sched.cpp \
	synthv1widget.cpp \
	synthv1widget_env.cpp \
	synthv1widget_filt.cpp \
	synthv1widget_wave.cpp \
	synthv1widget_knob.cpp \
	synthv1widget_preset.cpp \
	synthv1widget_status.cpp \
	synthv1widget_jack.cpp

FORMS = \
	synthv1widget.ui

RESOURCES += synthv1.qrc


unix {

	OBJECTS_DIR = .obj_jack
	MOC_DIR     = .moc_jack
	UI_DIR      = .ui_jack

	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}

	BINDIR = $${PREFIX}/bin
	DATADIR = $${PREFIX}/share

	DEFINES += DATADIR=\"$${DATADIR}\"

	INSTALLS += target desktop icon \
		icon_scalable mimeinfo mimetypes mimetypes_scalable

	target.path = $${BINDIR}

	desktop.path = $${DATADIR}/applications
	desktop.files += $${NAME}.desktop

	icon.path = $${DATADIR}/icons/hicolor/32x32/apps
	icon.files += images/$${NAME}.png 

	icon_scalable.path = $${DATADIR}/icons/hicolor/scalable/apps
	icon_scalable.files += images/$${NAME}.svg

	mimeinfo.path = $${DATADIR}/mime/packages
	mimeinfo.files += mimetypes/$${NAME}.xml

	mimetypes.path = $${DATADIR}/icons/hicolor/32x32/mimetypes
	mimetypes.files += mimetypes/application-x-$${NAME}-preset.png

	mimetypes_scalable.path = $${DATADIR}/icons/hicolor/scalable/mimetypes
	mimetypes_scalable.files += mimetypes/application-x-$${NAME}-preset.svg
}

QT += xml


# QT5 support
!lessThan(QT_MAJOR_VERSION, 5) {
	QT += widgets
}
