#-----------------------------
# PngLib lib

# set local path for lib
LOCAL_PATH := $(call my-dir)

# clear all variables
include $(CLEAR_VARS)

# set module name
LOCAL_MODULE := libpng

# set path for includes
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../include/libpng

# set exported includes
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

# set source files
LOCAL_SRC_FILES :=  \
                  ../../src/png.c \
                  ../../src/pngerror.c \
                  ../../src/pngget.c \
                  ../../src/pngmem.c \
                  ../../src/pngpread.c \
                  ../../src/pngread.c \
                  ../../src/pngrio.c \
                  ../../src/pngrtran.c \
                  ../../src/pngrutil.c \
                  ../../src/pngset.c \
                  ../../src/pngtrans.c \
                  ../../src/pngwio.c \
                  ../../src/pngwrite.c \
                  ../../src/pngwtran.c \
                  ../../src/pngwutil.c \

LOCAL_CFLAGS := -O2

# build static library
include $(BUILD_STATIC_LIBRARY)