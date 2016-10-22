#-----------------------------
# XmlLib lib

# set local path for lib
LOCAL_PATH := $(call my-dir)

# clear all variables
include $(CLEAR_VARS)

# set module name
LOCAL_MODULE := libmongodb

# set path for includes
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../include

# set exported includes
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

# set source files
LOCAL_SRC_FILES :=  \
                  ../../src/bson.c \
                  ../../src/encoding.c \
                  ../../src/env_posix.c \
                  ../../src/env_standard.c \
                  ../../src/gridfs.c \
                  ../../src/md5.c \
                  ../../src/mongo.c \
                  ../../src/numbers.c \

# set build flags
LOCAL_CFLAGS := -O2

#set exported build flags
LOCAL_EXPORT_CFLAGS := $(LOCAL_CFLAGS) 
                    
# build static library
include $(BUILD_STATIC_LIBRARY)
