#------------------------
# ApplicationLib library

#set local path
LOCAL_PATH := $(call my-dir)

MY_PROJECT_ROOT := $(LOCAL_PATH)/../..
DAVA_ROOT := $(MY_PROJECT_ROOT)/../..

# clear all variables
include $(CLEAR_VARS)

# set module name
LOCAL_MODULE := LevelPerformanceTest

# set path for includes
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Classes

# set exported includes
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

LOCAL_SRC_FILES := \
                   $(subst $(LOCAL_PATH)/,, \
                   $(wildcard $(LOCAL_PATH)/../../Classes/*.cpp) \
                   $(wildcard $(LOCAL_PATH)/../../../../Sources/Internal/Platform/TemplateAndroid/ExternC/*.cpp) )

LOCAL_LDLIBS := -lz -lOpenSLES -landroid -fuse-ld=gold -fno-exceptions

# set included libraries
LOCAL_STATIC_LIBRARIES := libInternal

# build shared library
include $(BUILD_SHARED_LIBRARY)

# include modules
$(call import-add-path,$(DAVA_ROOT)/Sources)
$(call import-module,Internal)