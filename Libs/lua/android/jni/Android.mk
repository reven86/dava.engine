#-----------------------------
# lua lib

# set local path for lib
LOCAL_PATH := $(call my-dir)

# clear all variables
include $(CLEAR_VARS)

# set module name
LOCAL_MODULE := liblua_android

# set path for includes
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../include

# set exported includes
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

# set source files
LOCAL_SRC_FILES :=  \
                  ../../src/lapi.c \
                  ../../src/lzio.c \
                  ../../src/lauxlib.c \
                  ../../src/lbaselib.c \
                  ../../src/lcode.c \
                  ../../src/ldblib.c \
                  ../../src/ldebug.c \
                  ../../src/ldo.c \
                  ../../src/ldump.c \
                  ../../src/lfunc.c \
                  ../../src/lgc.c \
                  ../../src/linit.c \
                  ../../src/liolib.c \
                  ../../src/llex.c \
                  ../../src/lmathlib.c \
                  ../../src/lmem.c \
                  ../../src/loadlib.c \
                  ../../src/lobject.c \
                  ../../src/lopcodes.c \
                  ../../src/loslib.c \
                  ../../src/lparser.c \
                  ../../src/lstate.c \
                  ../../src/lstring.c \
                  ../../src/lstrlib.c \
                  ../../src/ltable.c \
                  ../../src/ltablib.c \
                  ../../src/ltm.c \
                  ../../src/lundump.c \
                  ../../src/lvm.c \

# set build flags
LOCAL_CFLAGS := -O2

#set exported build flags
LOCAL_EXPORT_CFLAGS := $(LOCAL_CFLAGS) 
                    
# build static library
include $(BUILD_STATIC_LIBRARY)
