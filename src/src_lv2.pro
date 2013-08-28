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
	synthv1_lv2.h \
	synthv1_config.h \
	synthv1_wave.h \
	synthv1_ramp.h \
	synthv1_list.h \
	synthv1_fx.h \
	synthv1_param.h

SOURCES = \
	synthv1.cpp \
	synthv1_lv2.cpp \
	synthv1_param.cpp


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
		ARCH = $$system(uname -m)
		contains(ARCH, x86_64) {
			LV2DIR = $${PREFIX}/lib64/lv2
		} else {
			LV2DIR = $${PREFIX}/lib/lv2
		}
	}

	TARGET_LV2 = $${NAME}.lv2/$${TARGET}.so

	!exists($${TARGET_LV2}) {
		system(touch $${TARGET_LV2})
	}

	TARGET_LIB = $${NAME}.lv2/lib$${TARGET}.a

	!exists($${TARGET_LIB}) {
		system(touch $${TARGET_LIB})
	}

	QMAKE_POST_LINK += $${QMAKE_COPY} -vp $(TARGET) $${TARGET_LV2};\
		rm -vf $${TARGET_LIB}; ar -r $${TARGET_LIB} $${TARGET_LV2}

	INSTALLS += target

	target.path  = $${LV2DIR}/$${NAME}.lv2
	target.files = $${TARGET_LV2} \
		$${NAME}.lv2/$${NAME}.ttl \
		$${NAME}.lv2/manifest.ttl

	QMAKE_CLEAN += $${TARGET_LV2} $${TARGET_LIB}
}

QT -= gui
QT += xml
