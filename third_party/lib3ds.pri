# Build the vendored lib3ds sources directly into FVD++. This keeps packaged
# builds self-contained and avoids platform-specific lib3ds DLL/dylib paths.

LIB3DS_ROOT = $$OPENFVD_ROOT/third_party/lib3ds

INCLUDEPATH += $$LIB3DS_ROOT/src

SOURCES += \
    $$LIB3DS_ROOT/src/lib3ds_atmosphere.c \
    $$LIB3DS_ROOT/src/lib3ds_background.c \
    $$LIB3DS_ROOT/src/lib3ds_camera.c \
    $$LIB3DS_ROOT/src/lib3ds_chunk.c \
    $$LIB3DS_ROOT/src/lib3ds_chunktable.c \
    $$LIB3DS_ROOT/src/lib3ds_file.c \
    $$LIB3DS_ROOT/src/lib3ds_io.c \
    $$LIB3DS_ROOT/src/lib3ds_light.c \
    $$LIB3DS_ROOT/src/lib3ds_material.c \
    $$LIB3DS_ROOT/src/lib3ds_math.c \
    $$LIB3DS_ROOT/src/lib3ds_matrix.c \
    $$LIB3DS_ROOT/src/lib3ds_mesh.c \
    $$LIB3DS_ROOT/src/lib3ds_node.c \
    $$LIB3DS_ROOT/src/lib3ds_quat.c \
    $$LIB3DS_ROOT/src/lib3ds_shadow.c \
    $$LIB3DS_ROOT/src/lib3ds_track.c \
    $$LIB3DS_ROOT/src/lib3ds_util.c \
    $$LIB3DS_ROOT/src/lib3ds_vector.c \
    $$LIB3DS_ROOT/src/lib3ds_viewport.c
