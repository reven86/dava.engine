#-----------------------------
# Framework lib 

# set local path for lib
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := iconv_android-prebuilt
LOCAL_SRC_FILES := ../../Libs/libs/android/$(TARGET_ARCH_ABI)/libiconv_android.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := fmodex-prebuild
LOCAL_SRC_FILES := ../../Libs/fmod/lib/android/$(TARGET_ARCH_ABI)/libfmodex.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := fmodevent-prebuild
LOCAL_SRC_FILES := ../../Libs/fmod/lib/android/$(TARGET_ARCH_ABI)/libfmodevent.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := xml_android
LOCAL_SRC_FILES := ../../Libs/libs/android/$(TARGET_ARCH_ABI)/libxml_android.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := png_android
LOCAL_SRC_FILES := ../../Libs/libs/android/$(TARGET_ARCH_ABI)/libpng_android.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := freetype_android
LOCAL_SRC_FILES := ../../Libs/libs/android/$(TARGET_ARCH_ABI)/libfreetype_android.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := yaml_android
LOCAL_SRC_FILES := ../../Libs/libs/android/$(TARGET_ARCH_ABI)/libyaml_android.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := mongodb_android
LOCAL_SRC_FILES := ../../Libs/libs/android/$(TARGET_ARCH_ABI)/libmongodb_android.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := lua_android
LOCAL_SRC_FILES := ../../Libs/libs/android/$(TARGET_ARCH_ABI)/liblua_android.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := dxt_android
LOCAL_SRC_FILES := ../../Libs/libs/android/$(TARGET_ARCH_ABI)/libdxt_android.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := jpeg_android
LOCAL_SRC_FILES := ../../Libs/libs/android/$(TARGET_ARCH_ABI)/libjpeg_android.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := curl_android
LOCAL_SRC_FILES := ../../Libs/libs/android/$(TARGET_ARCH_ABI)/libcurl_android.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := ssl_android
LOCAL_SRC_FILES := ../../Libs/libs/android/$(TARGET_ARCH_ABI)/libssl_android.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := crypto_android
LOCAL_SRC_FILES := ../../Libs/libs/android/$(TARGET_ARCH_ABI)/libcrypto_android.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := zip_android
LOCAL_SRC_FILES := ../../Libs/libs/android/$(TARGET_ARCH_ABI)/libzip_android.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := fribidi_android
LOCAL_SRC_FILES := ../../Libs/libs/android/$(TARGET_ARCH_ABI)/libfribidi_android.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := unibreak_android
LOCAL_SRC_FILES := ../../Libs/libs/android/$(TARGET_ARCH_ABI)/libunibreak_android.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := uv_android
LOCAL_SRC_FILES := ../../Libs/libs/android/$(TARGET_ARCH_ABI)/libuv_android.a
include $(PREBUILT_STATIC_LIBRARY)


DAVA_ROOT := $(LOCAL_PATH)

# set path for includes
MY_LOCAL_C_INCLUDES := $(LOCAL_PATH)
MY_LOCAL_C_INCLUDES += $(LOCAL_PATH)/Platform/TemplateAndroid/
MY_LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Libs/include
MY_LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Libs/fmod/include
MY_LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Libs/lua/include

# set exported includes
MY_LOCAL_EXPORT_C_INCLUDES := $(MY_LOCAL_C_INCLUDES)

ifneq ($(filter $(TARGET_ARCH_ABI), armeabi-v7a armeabi-v7a-hard),)
ifndef USE_NEON
MY_USE_NEON := true
endif
ifeq ($(USE_NEON), true)
MY_LOCAL_ARM_NEON := true
MY_LOCAL_ARM_MODE := arm
MY_LOCAL_NEON_CFLAGS := -mfloat-abi=softfp -mfpu=neon -march=armv7
MY_LOCAL_CFLAGS += -DUSE_NEON
endif
endif

# set build flags
MY_LOCAL_CPPFLAGS += -frtti -DGL_GLEXT_PROTOTYPES=1
MY_LOCAL_CPPFLAGS += -Wno-invalid-offsetof
MY_LOCAL_CFLAGS += -DDAVA_FMOD
MY_LOCAL_CPPFLAGS += -std=c++1y
MY_LOCAL_CFLAGS += -Qunused-arguments
# temporal fix to turn off warning in release
ifeq ($(APP_OPTIM), debug)
MY_LOCAL_CFLAGS += -Werror
endif
MY_LOCAL_CFLAGS += -Wno-error=deprecated-register

MY_LOCAL_CPP_FEATURES += exceptions

ifeq ($(DAVA_PROFILE), true)
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
$(info ==============)
$(info profiling enabled!)
$(info ==============)

MY_LOCAL_CFLAGS += -pg
MY_LOCAL_CFLAGS += -D__DAVAENGINE_PROFILE__
endif
endif

# set exported build flags
MY_LOCAL_EXPORT_CFLAGS := $(LOCAL_CFLAGS)

# set exported used libs
MY_LOCAL_EXPORT_LDLIBS := $(LOCAL_LDLIBS)

# set included libraries
MY_LOCAL_STATIC_LIBRARIES := libbox2d

ifeq ($(DAVA_PROFILE), true)
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
MY_LOCAL_STATIC_LIBRARIES += android-ndk-profiler
endif
endif


MY_LOCAL_SHARED_LIBRARIES += iconv_android-prebuilt
MY_LOCAL_SHARED_LIBRARIES += fmodex-prebuild
MY_LOCAL_SHARED_LIBRARIES += fmodevent-prebuild

MY_LOCAL_STATIC_LIBRARIES += xml_android
MY_LOCAL_STATIC_LIBRARIES += png_android
MY_LOCAL_STATIC_LIBRARIES += freetype_android
MY_LOCAL_STATIC_LIBRARIES += yaml_android
MY_LOCAL_STATIC_LIBRARIES += mongodb_android
MY_LOCAL_STATIC_LIBRARIES += lua_android
MY_LOCAL_STATIC_LIBRARIES += dxt_android
MY_LOCAL_STATIC_LIBRARIES += jpeg_android
MY_LOCAL_STATIC_LIBRARIES += curl_android
MY_LOCAL_STATIC_LIBRARIES += ssl_android
MY_LOCAL_STATIC_LIBRARIES += crypto_android
MY_LOCAL_STATIC_LIBRARIES += zip_android
MY_LOCAL_STATIC_LIBRARIES += fribidi_android
MY_LOCAL_STATIC_LIBRARIES += unibreak_android
MY_LOCAL_STATIC_LIBRARIES += uv_android

MY_LOCAL_EXPORT_LDLIBS := -lGLESv1_CM -llog -lEGL

ifeq ($(APP_PLATFORM), android-14)
	MY_LOCAL_EXPORT_LDLIBS += -lGLESv2
else 
ifeq ($(APP_PLATFORM), android-15)
	MY_LOCAL_EXPORT_LDLIBS += -lGLESv2
else 
ifeq ($(APP_PLATFORM), android-16)
	MY_LOCAL_EXPORT_LDLIBS += -lGLESv2
else 
ifeq ($(APP_PLATFORM), android-17)
	MY_LOCAL_EXPORT_LDLIBS += -lGLESv2
else
	MY_LOCAL_EXPORT_LDLIBS += -lGLESv3
endif
endif
endif
endif

# clear all variables
include $(CLEAR_VARS)

# set module name
LOCAL_MODULE := libInternalPart1

LOCAL_C_INCLUDES := $(MY_LOCAL_C_INCLUDES)
LOCAL_EXPORT_C_INCLUDES := $(MY_LOCAL_EXPORT_C_INCLUDES)
LOCAL_ARM_NEON := $(MY_LOCAL_ARM_NEON)
LOCAL_ARM_MODE := $(MY_LOCAL_ARM_MODE)
LOCAL_NEON_CFLAGS := $(MY_LOCAL_NEON_CFLAGS)
LOCAL_CPPFLAGS := $(MY_LOCAL_CPPFLAGS)
LOCAL_CFLAGS := $(MY_LOCAL_CFLAGS)
LOCAL_CPP_FEATURES := $(MY_LOCAL_CPP_FEATURES)
LOCAL_EXPORT_CFLAGS := $(MY_LOCAL_EXPORT_CFLAGS)
LOCAL_EXPORT_LDLIBS := $(MY_LOCAL_EXPORT_LDLIBS)
LOCAL_STATIC_LIBRARIES := $(MY_LOCAL_STATIC_LIBRARIES)
LOCAL_SHARED_LIBRARIES := $(MY_LOCAL_SHARED_LIBRARIES)
LOCAL_STATIC_LIBRARIES := $(MY_LOCAL_STATIC_LIBRARIES)

USE_NEON := $(MY_USE_NEON)

# set source files 
LOCAL_SRC_FILES := \
                     $(subst $(LOCAL_PATH)/,, \
                     $(wildcard $(LOCAL_PATH)/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Animation/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Autotesting/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Base/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Collision/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Core/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Database/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Debug/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Entity/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/FileSystem/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Input/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Math/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Math/Neon/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Network/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Network/Base/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Network/Services/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Network/Private/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Particles/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Platform/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Platform/TemplateAndroid/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Platform/TemplateAndroid/BacktraceAndroid/*.cpp))
                     
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

# set module name
LOCAL_MODULE := libInternal

LOCAL_C_INCLUDES := $(MY_LOCAL_C_INCLUDES)
LOCAL_EXPORT_C_INCLUDES := $(MY_LOCAL_EXPORT_C_INCLUDES)
LOCAL_ARM_NEON := $(MY_LOCAL_ARM_NEON)
LOCAL_ARM_MODE := $(MY_LOCAL_ARM_MODE)
LOCAL_NEON_CFLAGS := $(MY_LOCAL_NEON_CFLAGS)
LOCAL_CPPFLAGS := $(MY_LOCAL_CPPFLAGS)
LOCAL_CFLAGS := $(MY_LOCAL_CFLAGS)
LOCAL_CPP_FEATURES := $(MY_LOCAL_CPP_FEATURES)
LOCAL_EXPORT_CFLAGS := $(MY_LOCAL_EXPORT_CFLAGS)
LOCAL_EXPORT_LDLIBS := $(MY_LOCAL_EXPORT_LDLIBS)
LOCAL_STATIC_LIBRARIES := $(MY_LOCAL_STATIC_LIBRARIES)
LOCAL_SHARED_LIBRARIES := $(MY_LOCAL_SHARED_LIBRARIES)
LOCAL_STATIC_LIBRARIES := $(MY_LOCAL_STATIC_LIBRARIES)

LOCAL_WHOLE_STATIC_LIBRARIES := libInternalPart1

USE_NEON := $(MY_USE_NEON)

LOCAL_SRC_FILES := \
                     $(subst $(LOCAL_PATH)/,, \
                     $(wildcard $(LOCAL_PATH)/Render/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Render/2D/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Render/2D/Systems/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Render/3D/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Render/Effects/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Render/Highlevel/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Render/Highlevel/Vegetation/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Render/Material/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Scene2D/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Scene3D/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Scene3D/Components/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Scene3D/Components/Controller/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Scene3D/Components/Waypoint/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Scene3D/Converters/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Scene3D/SceneFile/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Scene3D/Systems/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Scene3D/Systems/Controller/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Sound/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Thread/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/UI/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/UI/Components/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Utils/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Job/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Render/Image/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/DLC/Downloader/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/DLC/Patcher/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/DLC/Patcher/bsdiff/*.c) \
                     $(wildcard $(LOCAL_PATH)/DLC/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/DataStorage/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/DataStorage/Android/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Notification/*.cpp))

include $(BUILD_STATIC_LIBRARY)

# include modules
$(call import-add-path,$(DAVA_ROOT)/..)
$(call import-add-path,$(DAVA_ROOT)/../External)
$(call import-add-path,$(DAVA_ROOT)/../External/Box2D)
$(call import-add-path,$(DAVA_ROOT))

ifeq ($(DAVA_PROFILE), true)
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
$(call import-add-path,$(DAVA_ROOT)/../../Libs)
$(call import-module,android-ndk-profiler)
endif
endif

$(call import-module,box2d)