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

!unix:!macx {
    INCLUDEPATH += "C:\Development\Libraries\glew-1.12.0\include"
    INCLUDEPATH += "C:\Development\Libraries\glm"
    INCLUDEPATH += "C:\Development\Libraries\lib3ds-20080909\src"

    LIBS += -lOpenGL32 -lGlU32
    LIBS += "C:\Development\Libraries\glew-1.12.0\lib\Release\x64\glew32.lib"
    LIBS += "C:\Development\Libraries\lib3ds-20080909\build-lib3ds-64Bit-Release\release\lib3ds.dll"
}

unix:!macx {
    LIBS += -lGL -lGLU -lGLEW -lX11 -L/usr/local/lib/ -l3ds
}

macx {
    QMAKE_APPLE_DEVICE_ARCHS = arm64
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 12.0

    INCLUDEPATH += /opt/homebrew/include $$OPENFVD_ROOT/third_party/lib3ds/src
    LIBS += -framework Foundation -framework Cocoa -framework OpenGL

    QMAKE_CFLAGS += -arch arm64
    QMAKE_CXXFLAGS += -arch arm64
    QMAKE_LFLAGS += -arch arm64

    SOURCES += \
        $$OPENFVD_ROOT/third_party/lib3ds/src/lib3ds_atmosphere.c \
        $$OPENFVD_ROOT/third_party/lib3ds/src/lib3ds_background.c \
        $$OPENFVD_ROOT/third_party/lib3ds/src/lib3ds_camera.c \
        $$OPENFVD_ROOT/third_party/lib3ds/src/lib3ds_chunk.c \
        $$OPENFVD_ROOT/third_party/lib3ds/src/lib3ds_chunktable.c \
        $$OPENFVD_ROOT/third_party/lib3ds/src/lib3ds_file.c \
        $$OPENFVD_ROOT/third_party/lib3ds/src/lib3ds_io.c \
        $$OPENFVD_ROOT/third_party/lib3ds/src/lib3ds_light.c \
        $$OPENFVD_ROOT/third_party/lib3ds/src/lib3ds_material.c \
        $$OPENFVD_ROOT/third_party/lib3ds/src/lib3ds_math.c \
        $$OPENFVD_ROOT/third_party/lib3ds/src/lib3ds_matrix.c \
        $$OPENFVD_ROOT/third_party/lib3ds/src/lib3ds_mesh.c \
        $$OPENFVD_ROOT/third_party/lib3ds/src/lib3ds_node.c \
        $$OPENFVD_ROOT/third_party/lib3ds/src/lib3ds_quat.c \
        $$OPENFVD_ROOT/third_party/lib3ds/src/lib3ds_shadow.c \
        $$OPENFVD_ROOT/third_party/lib3ds/src/lib3ds_track.c \
        $$OPENFVD_ROOT/third_party/lib3ds/src/lib3ds_util.c \
        $$OPENFVD_ROOT/third_party/lib3ds/src/lib3ds_vector.c \
        $$OPENFVD_ROOT/third_party/lib3ds/src/lib3ds_viewport.c
}
