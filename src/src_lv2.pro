# synthv1_lv2.pro
#
NAME = synthv1

TARGET = $${NAME}
TEMPLATE = lib
CONFIG += shared plugin

include(src_lv2.pri)

HEADERS = \
	config.h \
	synthv1.h \
	synthv1_ui.h \
	synthv1_lv2.h \
	synthv1_config.h \
	synthv1_wave.h \
	synthv1_ramp.h \
	synthv1_list.h \
	synthv1_fx.h \
	synthv1_reverb.h \
	synthv1_param.h \
	synthv1_sched.h \
	synthv1_programs.h \
	synthv1_control.h

SOURCES = \
	synthv1.cpp \
	synthv1_ui.cpp \
	synthv1_lv2.cpp \
	synthv1_config.cpp \
	synthv1_wave.cpp \
	synthv1_param.cpp \
	synthv1_sched.cpp \
	synthv1_programs.cpp \
	synthv1_control.cpp


unix {

	OBJECTS_DIR = .obj_lv2
	MOC_DIR     = .moc_lv2
	UI_DIR      = .ui_lv2

	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}

	contains(PREFIX, $$system(echo $HOME)) {
		LV2DIR = $${PREFIX}/.lv2
	} else {
		isEmpty(LIBDIR) {
			LV2DIR = $${PREFIX}/lib/lv2
		} else {
			LV2DIR = $${LIBDIR}/lv2
		}
	}

	TARGET_LV2 = $${NAME}.lv2/$${TARGET}

	!exists($${TARGET_LV2}.so) {
		system(touch $${TARGET_LV2}.so)
	}

	TARGET_LIB = $${NAME}.lv2/lib$${TARGET}.a

	!exists($${TARGET_LIB}) {
		system(touch $${TARGET_LIB})
	}

	QMAKE_POST_LINK += $${QMAKE_COPY} -vp $(TARGET) $${TARGET_LV2}.so;\
		$${QMAKE_DEL_FILE} -vf $${TARGET_LIB};\
		ar -r $${TARGET_LIB} $${TARGET_LV2}.so

	INSTALLS += target

	target.path  = $${LV2DIR}/$${NAME}.lv2
	target.files = $${TARGET_LV2}.so \
		$${TARGET_LV2}.ttl \
		$${NAME}.lv2/manifest.ttl

	QMAKE_CLEAN += $${TARGET_LV2}.so $${TARGET_LIB}
}

QT -= gui
QT += xml
