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

win32 {
    INCLUDEPATH += "./ui/"
    INCLUDEPATH += "./renderer/"
    INCLUDEPATH += "./core/"

    OPENFVD_DEPS_ROOT = $$clean_path($$(OPENFVD_DEPS_ROOT))
    isEmpty(OPENFVD_DEPS_ROOT) {
        error("Set OPENFVD_DEPS_ROOT to a dependency prefix containing include/, lib/, and bin/ (the Windows build script configures this automatically).")
    }

    RC_FILE = winicon.rc

    INCLUDEPATH += $$OPENFVD_DEPS_ROOT/include
    LIBS += -L$$OPENFVD_DEPS_ROOT/lib -lglew32 -lOpenGL32 -lGlU32

    include($$OPENFVD_ROOT/third_party/lib3ds.pri)
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

    LIBS += -framework Foundation -framework Cocoa -framework OpenGL

    QMAKE_CFLAGS += -arch arm64
    QMAKE_CXXFLAGS += -arch arm64
    QMAKE_LFLAGS += -arch arm64
    QMAKE_CXXFLAGS_RELEASE += -O3

    # The old app bundled an x86_64-only dylib, which forced the entire
    # process through Rosetta. Compile the vendored source into the app.
    include($$OPENFVD_ROOT/third_party/lib3ds.pri)
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
