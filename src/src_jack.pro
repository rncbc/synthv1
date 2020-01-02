# synthv1_jack.pro
#
NAME = synthv1

TARGET = $${NAME}_jack
TEMPLATE = app

unix {
	LIBS += -L. -l$${NAME} -l$${NAME}_ui
	PRE_TARGETDEPS += lib$${NAME}.a lib$${NAME}_ui.a
}

include(src_jack.pri)

HEADERS = \
	config.h \
	synthv1_nsm.h \
	synthv1_jack.h \
	synthv1widget_jack.h

SOURCES = \
	synthv1_nsm.cpp \
	synthv1_jack.cpp \
	synthv1widget_jack.cpp


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

	CONFIG(release, debug|release):QMAKE_POST_LINK += strip $(TARGET)
}

QT += widgets xml
