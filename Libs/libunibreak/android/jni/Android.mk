#-----------------------------
# libunibrake lib

# set local path for lib
LOCAL_PATH := $(call my-dir)

# clear all variables
include $(CLEAR_VARS)

# set module name
LOCAL_MODULE := libunibreak_android

# set path for includes
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../src

# set exported includes
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

# set source files
LOCAL_SRC_FILES :=  \
                  ../../src/linebreak.c \
                  ../../src/linebreakdata.c \
                  ../../src/linebreakdef.c \
                  ../../src/wordbreak.c \
                  ../../src/wordbreakdata.c \
                  
LOCAL_CFLAGS := -O2

# build static library
include $(BUILD_STATIC_LIBRARY)