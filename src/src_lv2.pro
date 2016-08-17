# synthv1_lv2.pro
#
NAME = synthv1

TARGET = $${NAME}_lv2
TEMPLATE = lib
CONFIG += shared plugin

include(src_lv2.pri)

HEADERS = \
	config.h \
	synthv1.h \
	synthv1_lv2.h

SOURCES = \
	synthv1_lv2.cpp \


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

	TARGET_LV2 = $${NAME}.lv2/$${NAME}

	!exists($${TARGET_LV2}.so) {
		system(touch $${TARGET_LV2}.so)
	}

	TARGET_LIB = $${NAME}.lv2/lib$${NAME}.a

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

	LIBS += -L. -l$${NAME}
}

QT -= gui
QT += xml
