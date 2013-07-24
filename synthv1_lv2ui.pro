# synthv1_lv2ui.pro
#
QMAKEVERSION = $$[QMAKE_VERSION]
ISQT4 = $$find(QMAKEVERSION, ^[2-9])
isEmpty( ISQT4 ) {
	error("Use the qmake include with Qt4.4 or greater, on Debian that is qmake-qt4");
}

TEMPLATE = subdirs
SUBDIRS = src
src.file = src/src_lv2ui.pro
