LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libpng

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../tmp/libpng_source/

# set exported includes
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

# set source files
LOCAL_SRC_FILES := \
                  ../../tmp/libpng_source/png.c \
                  ../../tmp/libpng_source/pngerror.c \
                  ../../tmp/libpng_source/pngget.c \
                  ../../tmp/libpng_source/pngmem.c \
                  ../../tmp/libpng_source/pngpread.c \
                  ../../tmp/libpng_source/pngread.c \
                  ../../tmp/libpng_source/pngrio.c \
                  ../../tmp/libpng_source/pngrtran.c \
                  ../../tmp/libpng_source/pngrutil.c \
                  ../../tmp/libpng_source/pngset.c \
                  ../../tmp/libpng_source/pngtrans.c \
                  ../../tmp/libpng_source/pngwio.c \
                  ../../tmp/libpng_source/pngwrite.c \
                  ../../tmp/libpng_source/pngwtran.c \
                  ../../tmp/libpng_source/pngwutil.c \

LOCAL_CFLAGS := -O2

include $(BUILD_STATIC_LIBRARY)