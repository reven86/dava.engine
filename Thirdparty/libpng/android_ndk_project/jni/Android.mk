LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libpng

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../tmp/libpng_source/

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

# SRC_PATH should be specified via command line arg, to avoid hardcoding pathes to downloaded sources
LOCAL_SRC_FILES := \
                  $(SRC_PATH)/png.c \
                  $(SRC_PATH)/pngerror.c \
                  $(SRC_PATH)/pngget.c \
                  $(SRC_PATH)/pngmem.c \
                  $(SRC_PATH)/pngpread.c \
                  $(SRC_PATH)/pngread.c \
                  $(SRC_PATH)/pngrio.c \
                  $(SRC_PATH)/pngrtran.c \
                  $(SRC_PATH)/pngrutil.c \
                  $(SRC_PATH)/pngset.c \
                  $(SRC_PATH)/pngtrans.c \
                  $(SRC_PATH)/pngwio.c \
                  $(SRC_PATH)/pngwrite.c \
                  $(SRC_PATH)/pngwtran.c \
                  $(SRC_PATH)/pngwutil.c \

LOCAL_CFLAGS := -O2

include $(BUILD_STATIC_LIBRARY)