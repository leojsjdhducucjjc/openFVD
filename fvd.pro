#-------------------------------------------------
#
#    FVD++, an advanced coaster design tool for NoLimits
#    Copyright (C) 2012-2015, Stephan "Lenny" Alt <alt.stephan@web.de>
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program. If not, see <http://www.gnu.org/licenses/>.
#
#-------------------------------------------------


# DEPENDENCIES
# 
# QT (tested with 5.4.0)
# glew (for Windows and Linux builds only)
# glm (tested with 0.9.5.1-1)
# lib3ds

CONFIG	+= qt c++11
QT       += core gui widgets printsupport opengl

#CONFIG += exceptions \
#          rtti

TARGET = FVD
TEMPLATE = app

OPENFVD_ROOT = $$PWD
INCLUDEPATH += $$OPENFVD_ROOT/trackbuilder
SOURCES += $$OPENFVD_ROOT/main.cpp
include($$OPENFVD_ROOT/fvd_sources.pri)

!unix:!macx {
    INCLUDEPATH += "./ui/"
    INCLUDEPATH += "./renderer/"
    INCLUDEPATH += "./core/"

	INCLUDEPATH += "C:\Development\Libraries\glew-1.12.0\include" # path-to-glew/include
	INCLUDEPATH += "C:\Development\Libraries\glm" #path-to-glm"
	INCLUDEPATH += "C:\Development\Libraries\lib3ds-20080909\src" #path-to-lib3ds

    RC_FILE = winicon.rc

    LIBS += -lOpenGL32
    LIBS += -lGlU32
	LIBS += "C:\Development\Libraries\glew-1.12.0\lib\Release\x64\glew32.lib" #path-to-glew\lib\Release\Win32\glew32.lib
	LIBS += "C:\Development\Libraries\glew-1.12.0\bin\Release\x64\glew32.dll" #path-to-glew\bin\Release\Win32\glew32.dll
	LIBS += "C:\Development\Libraries\lib3ds-20080909\build-lib3ds-64Bit-Release\release\lib3ds.dll" #path-to-glew\bin\Release\Win32\glew32.dll
}

unix:!macx {
    INCLUDEPATH += "./ui/"
    INCLUDEPATH += "./renderer/"
    INCLUDEPATH += "./core/"

    LIBS += -lGL
    LIBS += -lGLU
    LIBS += -lGLEW

    LIBS += -lX11
    LIBS += -L /usr/local/lib/
    LIBS += -l3ds
}

macx {
    ICON = fvd.icns
    QMAKE_INFO_PLIST = ./osx/resources/Info.plist

    # Homebrew's Qt 5 bottle is native on Apple Silicon. Keep the target
    # explicit so qmake can never silently produce a Rosetta-only build.
    QMAKE_APPLE_DEVICE_ARCHS = arm64
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 12.0

    INCLUDEPATH += "./ui/"
    INCLUDEPATH += "./renderer/"
    INCLUDEPATH += "./core/"
    INCLUDEPATH += "/opt/homebrew/include/"
    INCLUDEPATH += "./third_party/lib3ds/src/"

    LIBS += -framework Foundation -framework Cocoa -framework OpenGL

    QMAKE_CFLAGS += -arch arm64
    QMAKE_CXXFLAGS += -arch arm64
    QMAKE_LFLAGS += -arch arm64
    QMAKE_CXXFLAGS_RELEASE += -O3

    # Build lib3ds into FVD itself. The old app bundled an x86_64-only dylib,
    # which forced the entire process through Rosetta.
    SOURCES += \
        third_party/lib3ds/src/lib3ds_atmosphere.c \
        third_party/lib3ds/src/lib3ds_background.c \
        third_party/lib3ds/src/lib3ds_camera.c \
        third_party/lib3ds/src/lib3ds_chunk.c \
        third_party/lib3ds/src/lib3ds_chunktable.c \
        third_party/lib3ds/src/lib3ds_file.c \
        third_party/lib3ds/src/lib3ds_io.c \
        third_party/lib3ds/src/lib3ds_light.c \
        third_party/lib3ds/src/lib3ds_material.c \
        third_party/lib3ds/src/lib3ds_math.c \
        third_party/lib3ds/src/lib3ds_matrix.c \
        third_party/lib3ds/src/lib3ds_mesh.c \
        third_party/lib3ds/src/lib3ds_node.c \
        third_party/lib3ds/src/lib3ds_quat.c \
        third_party/lib3ds/src/lib3ds_shadow.c \
        third_party/lib3ds/src/lib3ds_track.c \
        third_party/lib3ds/src/lib3ds_util.c \
        third_party/lib3ds/src/lib3ds_vector.c \
        third_party/lib3ds/src/lib3ds_viewport.c
}

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    winicon.rc \
    background.png \
    metal.png \
    shaders/normals.vert \
    shaders/track.vert \
    shaders/track.frag \
    shaders/sky.vert \
    shaders/sky.frag \
    shaders/simpleSM.vert \
    shaders/simpleSM.frag \
    shaders/shadowVolume.vert \
    shaders/shadowVolume.frag \
    shaders/oculus.vert \
    shaders/oculus.frag \
    shaders/occlusion.vert \
    shaders/occlusion.frag \
    shaders/normals.frag \
    shaders/metal.dat \
    shaders/floor.vert \
    shaders/floor.frag \
    shaders/ghost.vert \
    shaders/ghost.frag \
    shaders/debug.vert \
    shaders/debug.frag \
    metalnormals.png \
    readme.txt \
    sky/negx.jpg \
    sky/negy.jpg \
    sky/negz.jpg \
    sky/posx.jpg \
    sky/posy.jpg \
    sky/posz.jpg
