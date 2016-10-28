# set local path for lib
LOCAL_PATH := $(call my-dir)

# clear all variables
include $(CLEAR_VARS)

# set module name
LOCAL_MODULE := libsqlite3

# set path for includes
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../

# set exported includes
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

LOCAL_SRC_FILES :=  ../../sqlite3.c

# set build flags
LOCAL_CFLAGS := -O2

#set exported build flags
LOCAL_EXPORT_CFLAGS := $(LOCAL_CFLAGS)

# build static library
include $(BUILD_STATIC_LIBRARY)
