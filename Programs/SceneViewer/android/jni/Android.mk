#------------------------
# ApplicationLib library

#set local path
LOCAL_PATH := $(call my-dir)

DV_PROJECT_ROOT := $(LOCAL_PATH)/../..
DAVA_ROOT := $(DV_PROJECT_ROOT)/../..

# clear all variables
include $(CLEAR_VARS)

# set module name
LOCAL_MODULE := SceneViewer

# set path for includes
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(DV_PROJECT_ROOT)/Classes
LOCAL_C_INCLUDES += $(DAVA_ROOT)/Modules/ScenePerformanceTests/Sources

# set exported includes
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

# set source files
LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(DV_PROJECT_ROOT)/Classes/*.cpp) \
	$(wildcard $(DV_PROJECT_ROOT)/Classes/UIControls/*.cpp) \
	$(wildcard $(DV_PROJECT_ROOT)/Classes/UIScreens/*.cpp) \
	$(wildcard $(DV_PROJECT_ROOT)/Classes/Quality/*.cpp) \
	$(wildcard $(DAVA_ROOT)/Sources/Tools/TeamcityOutput/*.cpp) \
	$(wildcard $(DAVA_ROOT)/Modules/ScenePerformanceTests/Sources/Private/*.cpp))

LOCAL_CPPFLAGS += -std=c++14

# set included libraries
LOCAL_STATIC_LIBRARIES := libInternal

# build shared library
include $(BUILD_SHARED_LIBRARY)

# include modules
$(call import-add-path,$(DAVA_ROOT)/Sources)
$(call import-module,Internal)