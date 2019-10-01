# synthv1_ui.pro
#
NAME = synthv1

TARGET = $${NAME}_ui
TEMPLATE = lib
CONFIG += static

unix { LIBS += -L. -l$${NAME} }

include(src_ui.pri)

HEADERS = \
	config.h \
	synthv1_ui.h \
	synthv1widget.h \
	synthv1widget_env.h \
	synthv1widget_filt.h \
	synthv1widget_wave.h \
	synthv1widget_param.h \
	synthv1widget_keybd.h \
	synthv1widget_preset.h \
	synthv1widget_status.h \
	synthv1widget_programs.h \
	synthv1widget_controls.h \
	synthv1widget_control.h \
	synthv1widget_config.h

SOURCES = \
	synthv1_ui.cpp \
	synthv1widget.cpp \
	synthv1widget_env.cpp \
	synthv1widget_filt.cpp \
	synthv1widget_wave.cpp \
	synthv1widget_param.cpp \
	synthv1widget_keybd.cpp \
	synthv1widget_preset.cpp \
	synthv1widget_status.cpp \
	synthv1widget_programs.cpp \
	synthv1widget_controls.cpp \
	synthv1widget_control.cpp \
	synthv1widget_config.cpp

FORMS = \
	synthv1widget.ui \
	synthv1widget_control.ui \
	synthv1widget_config.ui

RESOURCES += synthv1.qrc


unix {

	OBJECTS_DIR = .obj_ui
	MOC_DIR     = .moc_ui
	UI_DIR      = .ui_ui

	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}

	isEmpty(LIBDIR) {
		TARGET_ARCH = $$system(uname -m)
		contains(TARGET_ARCH, x86_64) {
			LIBDIR = $${PREFIX}/lib64
		} else {
			LIBDIR = $${PREFIX}/lib
		}
	}

	INSTALLS += target

	target.path = $${LIBDIR}
}

QT += widgets xml
