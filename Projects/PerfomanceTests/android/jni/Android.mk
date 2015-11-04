#------------------------
# ApplicationLib library

#set local path
LOCAL_PATH := $(call my-dir)

MY_PROJECT_ROOT := $(LOCAL_PATH)/../..
DAVA_ROOT := $(MY_PROJECT_ROOT)/../..

# clear all variables
include $(CLEAR_VARS)

# set module name
LOCAL_MODULE := PerformanceTests

# set path for includes
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(MY_PROJECT_ROOT)/Classes
LOCAL_C_INCLUDES += $(MY_PROJECT_ROOT)/Classes/OldTests
LOCAL_C_INCLUDES += $(MY_PROJECT_ROOT)/Classes/Infrastructure
LOCAL_C_INCLUDES += $(DAVA_ROOT)/Sources/Tools

# set exported includes
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

# set source files
LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(MY_PROJECT_ROOT)/Classes/*.cpp) \
	$(wildcard $(MY_PROJECT_ROOT)/Classes/Infrastructure/*.cpp) \
	$(wildcard $(MY_PROJECT_ROOT)/Classes/Infrastructure/Screen/*.cpp) \
	$(wildcard $(MY_PROJECT_ROOT)/Classes/Infrastructure/Controller/*.cpp) \
	$(wildcard $(MY_PROJECT_ROOT)/Classes/Infrastructure/Settings/*.cpp) \
	$(wildcard $(MY_PROJECT_ROOT)/Classes/Infrastructure/Utils/*.cpp) \
	$(wildcard $(MY_PROJECT_ROOT)/Classes/Tests/*.cpp) \
	$(wildcard $(MY_PROJECT_ROOT)/Classes/Tests/Controller/*.cpp) \
	$(wildcard $(DAVA_ROOT)/Sources/Tools/TeamcityOutput/*.cpp) \
	$(wildcard $(DAVA_ROOT)/Sources/Tools/CommandLine/CommandLineParser.cpp) \
	$(wildcard $(DAVA_ROOT)/Sources/Internal/Platform/TemplateAndroid/ExternC/*.cpp) )

LOCAL_LDLIBS := -lz -lOpenSLES -landroid

ifneq ($(filter $(TARGET_ARCH_ABI), armeabi-v7a armeabi-v7a-hard),)
LOCAL_ARM_NEON := true
LOCAL_NEON_CFLAGS := -mfloat-abi=softfp -mfpu=neon -march=armv7
endif
LOCAL_CPPFLAGS += -std=c++1y

# set included libraries
LOCAL_STATIC_LIBRARIES := libInternal

# build shared library
include $(BUILD_SHARED_LIBRARY)

# include modules
$(call import-add-path,$(DAVA_ROOT)/Sources)
$(call import-module,Internal)