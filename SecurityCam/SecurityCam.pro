
TEMPLATE = app
DESTDIR = ..
TARGET = SecurityCam.App
QT += core gui widgets multimedia xml multimediawidgets

TO_GENERATE = cfg.qtconf

GENERATED =

QMAKE_EXTRA_COMPILERS += generate_cfg
generate_cfg.name = CONF_GEN
generate_cfg.input = TO_GENERATE
generate_cfg.output = ${QMAKE_FILE_BASE}.hpp
generate_cfg.CONFIG = no_link
generate_cfg.variable_out = GENERATED

generate_cfg.commands = $$shell_path( $$absolute_path( $${OUT_PWD}/../3rdparty/QtConfFile/qtconffile.generator ) ) \
-i ${QMAKE_FILE_IN} \
-o $${OUT_PWD}/${QMAKE_FILE_BASE}.hpp

PRE_TARGETDEPS += compiler_generate_cfg_make_all

RESOURCES = resources.qrc

win32 {
    RC_FILE = SecurityCam.rc
}

HEADERS = mainwindow.hpp \
	$$GENERATED \
    options.hpp \
    frames.hpp \
    view.hpp

SOURCES = main.cpp \
	mainwindow.cpp \
    options.cpp \
    frames.cpp \
    view.cpp
	

unix|win32: LIBS += -L$$OUT_PWD/../3rdparty/QtConfFile/lib/ -lQtConfFile

INCLUDEPATH += $$PWD/../3rdparty/QtConfFile
DEPENDPATH += $$PWD/../3rdparty/QtConfFile

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../3rdparty/QtConfFile/lib/QtConfFile.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../3rdparty/QtConfFile/lib/libQtConfFile.a

include( ../3rdparty/QtArg/QtArg/qtarg.pri )

unix: LIBS += -lopencv_core

win32: LIBS += -lopencv_core2413

FORMS += \
    options.ui
