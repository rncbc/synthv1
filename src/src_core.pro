# synthv1.pro
#
NAME = synthv1

TARGET = $${NAME}
TEMPLATE = lib
CONFIG += shared

include(src_core.pri)

HEADERS = \
	config.h \
	synthv1.h \
	synthv1_ui.h \
	synthv1_config.h \
	synthv1_filter.h \
	synthv1_formant.h \
	synthv1_wave.h \
	synthv1_ramp.h \
	synthv1_list.h \
	synthv1_fx.h \
	synthv1_reverb.h \
	synthv1_param.h \
	synthv1_sched.h \
	synthv1_programs.h \
	synthv1_controls.h

SOURCES = \
	synthv1.cpp \
	synthv1_ui.cpp \
	synthv1_config.cpp \
	synthv1_formant.cpp \
	synthv1_wave.cpp \
	synthv1_param.cpp \
	synthv1_sched.cpp \
	synthv1_programs.cpp \
	synthv1_controls.cpp


unix {

	OBJECTS_DIR = .obj_core
	MOC_DIR     = .moc_core
	UI_DIR      = .ui_core

	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}

	isEmpty(LIBDIR) {
		LIBDIR = $${PREFIX}/lib
	}

	INSTALLS += target

	target.path = $${LIBDIR}
}

QT -= gui
QT += xml
