CONFIG += qt testcase c++11
CONFIG -= app_bundle

QT += core gui widgets printsupport opengl testlib

TARGET = fvd_regression_tests
TEMPLATE = app

OPENFVD_ROOT = $$clean_path($$PWD/..)

SOURCES += $$PWD/legacy_project_regression.cpp
include($$OPENFVD_ROOT/fvd_sources.pri)

INCLUDEPATH += \
    $$OPENFVD_ROOT \
    $$OPENFVD_ROOT/trackbuilder \
    $$OPENFVD_ROOT/ui \
    $$OPENFVD_ROOT/renderer \
    $$OPENFVD_ROOT/core

RESOURCES += $$OPENFVD_ROOT/resources.qrc

win32 {
    OPENFVD_DEPS_ROOT = $$clean_path($$(OPENFVD_DEPS_ROOT))
    isEmpty(OPENFVD_DEPS_ROOT) {
        error("Set OPENFVD_DEPS_ROOT to a dependency prefix containing include/, lib/, and bin/.")
    }

    INCLUDEPATH += $$OPENFVD_DEPS_ROOT/include
    LIBS += -L$$OPENFVD_DEPS_ROOT/lib -lglew32 -lOpenGL32 -lGlU32

    include($$OPENFVD_ROOT/third_party/lib3ds.pri)
}

unix:!macx {
    LIBS += -lGL -lGLU -lGLEW -lX11 -L/usr/local/lib/ -l3ds
}

macx {
    QMAKE_APPLE_DEVICE_ARCHS = arm64
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 12.0

    INCLUDEPATH += /opt/homebrew/include
    LIBS += -framework Foundation -framework Cocoa -framework OpenGL

    QMAKE_CFLAGS += -arch arm64
    QMAKE_CXXFLAGS += -arch arm64
    QMAKE_LFLAGS += -arch arm64

    include($$OPENFVD_ROOT/third_party/lib3ds.pri)
}
