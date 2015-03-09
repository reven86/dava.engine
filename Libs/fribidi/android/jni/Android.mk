#-----------------------------
# PngLib lib

# set local path for lib
LOCAL_PATH := $(call my-dir)

# clear all variables
include $(CLEAR_VARS)

# set module name
LOCAL_MODULE := libfribidi_android

# set path for includes
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../lib
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../charset

# set exported includes
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

# set source files
LOCAL_SRC_FILES :=  \
                  ../../lib/fribidi.c \
                  ../../lib/fribidi-arabic.c \
                  ../../lib/fribidi-bidi.c \
                  ../../lib/fribidi-bidi-types.c \
                  ../../lib/fribidi-deprecated.c \
                  ../../lib/fribidi-joining.c \
                  ../../lib/fribidi-joining-types.c \
                  ../../lib/fribidi-mem.c \
                  ../../lib/fribidi-mirroring.c \
                  ../../lib/fribidi-run.c \
                  ../../lib/fribidi-shape.c \
                  ../../charset/fribidi-char-sets.c \
                  ../../charset/fribidi-char-sets-cap-rtl.c \
                  ../../charset/fribidi-char-sets-cp1255.c \
                  ../../charset/fribidi-char-sets-cp1256.c \
                  ../../charset/fribidi-char-sets-iso8859-6.c \
                  ../../charset/fribidi-char-sets-iso8859-8.c \
                  ../../charset/fribidi-char-sets-utf8.c \

LOCAL_CFLAGS := -O2 -DHAVE_CONFIG_H=1

# build static library
include $(BUILD_STATIC_LIBRARY)