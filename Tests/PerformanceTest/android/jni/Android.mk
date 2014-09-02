#------------------------
# ApplicationLib library

#set local path
LOCAL_PATH := $(call my-dir)

# clear all variables
include $(CLEAR_VARS)

# set module name
LOCAL_MODULE := PerformanceTestLib

# set path for includes
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Classes


# set exported includes
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

# set source files
LOCAL_SRC_FILES :=  \
                    AndroidLayer.cpp \
                    AndroidDelegate.cpp \
                    \
                    ../../Classes/FrameworkMain.cpp \
                    ../../Classes/GameCore.cpp \
                    ../../Classes/BaseScreen.cpp \
                    ../../Classes/SpriteTest.cpp \
                    ../../Classes/CacheTest.cpp \
                    ../../Classes/LandscapeTest.cpp \
                    ../../Classes/TestTemplate.cpp \

LOCAL_CFLAGS := -g -O2

LOCAL_LDLIBS := -lz -lOpenSLES -landroid

# set included libraries
LOCAL_STATIC_LIBRARIES := libInternal libbox2d

# build shared library
include $(BUILD_SHARED_LIBRARY)

# include modules
$(call import-module, Internal)