# synthv1_lv2ui.pro
#
NAME = synthv1

TARGET = $${NAME}_lv2ui
TEMPLATE = lib
CONFIG += shared plugin

include(src_lv2.pri)

HEADERS = \
	config.h \
	synthv1_ui.h \
	synthv1_config.h \
	synthv1_param.h \
	synthv1_programs.h \
	synthv1_controls.h \
	synthv1_lv2ui.h \
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
	synthv1widget_lv2.h

SOURCES = \
	synthv1_lv2ui.cpp \
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
	synthv1widget_lv2.cpp

FORMS = \
	synthv1widget.ui \
	synthv1widget_control.ui \
	synthv1widget_config.ui

RESOURCES += synthv1.qrc


unix {

	OBJECTS_DIR = .obj_lv2ui
	MOC_DIR     = .moc_lv2ui
	UI_DIR      = .ui_lv2ui

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

	contains(PREFIX, $$system(echo $HOME)) {
		LV2DIR = $${PREFIX}/.lv2
	} else {
		LV2DIR = $${LIBDIR}/lv2
	}

	TARGET_LV2UI = $${NAME}.lv2/$${NAME}_ui

	!exists($${TARGET_LV2UI}.so) {
		system(touch $${TARGET_LV2UI}.so)
	}

	!exists($${TARGET_LV2UI}.ttl) {
		system(touch $${TARGET_LV2UI}.ttl)
	}

	INSTALLS += target

	target.path  = $${LV2DIR}/$${NAME}.lv2
	target.files = $${TARGET_LV2UI}.so $${TARGET_LV2UI}.ttl

	QMAKE_POST_LINK += $${QMAKE_COPY} -vp $(TARGET) $${TARGET_LV2UI}.so

	greaterThan(QT_MAJOR_VERSION, 4) {
		QMAKE_POST_LINK += ;\
			$${QMAKE_COPY} -vp $${TARGET_LV2UI}-qt5.ttl $${TARGET_LV2UI}.ttl
	} else {
		QMAKE_POST_LINK += ;\
			$${QMAKE_COPY} -vp $${TARGET_LV2UI}-qt4.ttl $${TARGET_LV2UI}.ttl
	}

	QMAKE_CLEAN += $${TARGET_LV2UI}.so $${TARGET_LV2UI}.ttl

	LIBS += -L. -l$${NAME} -L$${NAME}.lv2 -Wl,-rpath,$${LIBDIR}:$${LV2DIR}/$${NAME}.lv2
}

QT += xml

# QT5 support
greaterThan(QT_MAJOR_VERSION, 4) {
	QT += widgets
}
