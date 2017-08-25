# synthv1_jack.pro
#
NAME = synthv1

TARGET = $${NAME}_jack
TEMPLATE = app

include(src_jack.pri)

HEADERS = \
	config.h \
	synthv1.h \
	synthv1_ui.h \
	synthv1_config.h \
	synthv1_param.h \
	synthv1_programs.h \
	synthv1_controls.h \
	synthv1_nsm.h \
	synthv1_jack.h \
	synthv1widget.h \
	synthv1widget_env.h \
	synthv1widget_filt.h \
	synthv1widget_wave.h \
	synthv1widget_param.h \
	synthv1widget_preset.h \
	synthv1widget_status.h \
	synthv1widget_programs.h \
	synthv1widget_controls.h \
	synthv1widget_control.h \
	synthv1widget_config.h \
	synthv1widget_jack.h

SOURCES = \
	synthv1_nsm.cpp \
	synthv1_jack.cpp \
	synthv1widget.cpp \
	synthv1widget_env.cpp \
	synthv1widget_filt.cpp \
	synthv1widget_wave.cpp \
	synthv1widget_param.cpp \
	synthv1widget_preset.cpp \
	synthv1widget_status.cpp \
	synthv1widget_programs.cpp \
	synthv1widget_controls.cpp \
	synthv1widget_control.cpp \
	synthv1widget_config.cpp \
	synthv1widget_jack.cpp

FORMS = \
	synthv1widget.ui \
	synthv1widget_control.ui \
	synthv1widget_config.ui

RESOURCES += synthv1.qrc


unix {

	OBJECTS_DIR = .obj_jack
	MOC_DIR     = .moc_jack
	UI_DIR      = .ui_jack

	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}

	isEmpty(BINDIR) {
		BINDIR = $${PREFIX}/bin
	}

	isEmpty(LIBDIR) {
		TARGET_ARCH = $$system(uname -m)
		contains(TARGET_ARCH, x86_64) {
			LIBDIR = $${PREFIX}/lib64
		} else {
			LIBDIR = $${PREFIX}/lib
		}
	}

	isEmpty(DATADIR) {
		DATADIR = $${PREFIX}/share
	}

	#DEFINES += DATADIR=\"$${DATADIR}\"

	INSTALLS += target desktop icon appdata \
		icon_scalable mimeinfo mimetypes mimetypes_scalable

	target.path = $${BINDIR}

	desktop.path = $${DATADIR}/applications
	desktop.files += $${NAME}.desktop

	icon.path = $${DATADIR}/icons/hicolor/32x32/apps
	icon.files += images/$${NAME}.png 

	icon_scalable.path = $${DATADIR}/icons/hicolor/scalable/apps
	icon_scalable.files += images/$${NAME}.svg

	appdata.path = $${DATADIR}/metainfo
	appdata.files += appdata/$${NAME}.appdata.xml

	mimeinfo.path = $${DATADIR}/mime/packages
	mimeinfo.files += mimetypes/$${NAME}.xml

	mimetypes.path = $${DATADIR}/icons/hicolor/32x32/mimetypes
	mimetypes.files += mimetypes/application-x-$${NAME}-preset.png

	mimetypes_scalable.path = $${DATADIR}/icons/hicolor/scalable/mimetypes
	mimetypes_scalable.files += mimetypes/application-x-$${NAME}-preset.svg

	LIBS += -L. -l$${NAME} -Wl,-rpath,$${LIBDIR}
}

QT += xml

# QT5 support
!lessThan(QT_MAJOR_VERSION, 5) {
	QT += widgets
}
