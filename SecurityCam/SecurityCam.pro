
TEMPLATE = app
DESTDIR = ..
TARGET = SecurityCam.App
QT += core gui widgets multimedia multimediawidgets
DEFINES += CFGFILE_QT_SUPPORT ARGS_QSTRING_BUILD
CONFIG += c++14

TO_GENERATE = cfg.qtconf

GENERATED =

QMAKE_EXTRA_COMPILERS += generate_cfg
generate_cfg.name = CONF_GEN
generate_cfg.input = TO_GENERATE
generate_cfg.output = ${QMAKE_FILE_BASE}.hpp
generate_cfg.CONFIG = no_link
generate_cfg.variable_out = GENERATED

generate_cfg.commands = $$shell_path( $$absolute_path( $${OUT_PWD}/../3rdparty/cfgfile/cfgfile.generator ) ) \
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
    view.hpp \
    resolution.hpp

SOURCES = main.cpp \
	mainwindow.cpp \
    options.cpp \
    frames.cpp \
    view.cpp \
    resolution.cpp
	

INCLUDEPATH += $$PWD/../3rdparty/cfgfile
DEPENDPATH += $$PWD/../3rdparty/cfgfile

include( ../3rdparty/Args/Args/Args.pri )

unix: LIBS += -lopencv_core

win32: LIBS += -lopencv_core2413

FORMS += \
    options.ui \
    resolution.ui
