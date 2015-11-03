#-----------------------------
# Framework lib 

# set local path for lib
LOCAL_PATH := $(call my-dir)

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

include $(CLEAR_VARS)
LOCAL_MODULE := webp_android
LOCAL_SRC_FILES := ../../Libs/libs/android/$(TARGET_ARCH_ABI)/libwebp_android.a
include $(PREBUILT_STATIC_LIBRARY)

DAVA_ROOT := $(LOCAL_PATH)

# set path for includes
DV_LOCAL_C_INCLUDES := $(LOCAL_PATH)
DV_LOCAL_C_INCLUDES += $(LOCAL_PATH)/../Tools/
DV_LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Libs/include
DV_LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Libs/fmod/include
DV_LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Libs/lua/include

# set exported includes
DV_LOCAL_EXPORT_C_INCLUDES := $(DV_LOCAL_C_INCLUDES)

# starting from ndk10b x86 support NEON too! Add latter
ifeq ($(TARGET_ARCH_ABI), $(filter $(TARGET_ARCH_ABI), armeabi-v7a))
DV_LOCAL_ARM_NEON := true
DV_LOCAL_ARM_MODE := arm
DV_LOCAL_NEON_CFLAGS := -mfloat-abi=softfp -mfpu=neon -march=armv7
DV_LOCAL_CFLAGS += -DUSE_NEON
else
DV_LOCAL_ARM_NEON := false
endif

# set build flags
DV_LOCAL_CPPFLAGS += -frtti -DGL_GLEXT_PROTOTYPES=1
DV_LOCAL_CPPFLAGS += -std=c++1y

DV_LOCAL_CFLAGS += -DDAVA_FMOD

# remove warnings about unused arguments to compiler
DV_LOCAL_CFLAGS += -Qunused-arguments
# enable ALL warnings
# we write C++ so check warnings only inside our C++ code not C libraries
DV_LOCAL_CPPFLAGS += -Weverything
# treat warnings as errors
DV_LOCAL_CPPFLAGS += -Werror
# disable common simple warnings
# read about any clang warning messages http://fuckingclangwarnings.com/
DV_LOCAL_CPPFLAGS += -Wno-c++98-compat-pedantic
DV_LOCAL_CPPFLAGS += -Wno-newline-eof
DV_LOCAL_CPPFLAGS += -Wno-gnu-anonymous-struct
DV_LOCAL_CPPFLAGS += -Wno-nested-anon-types
DV_LOCAL_CPPFLAGS += -Wno-float-equal
DV_LOCAL_CPPFLAGS += -Wno-extra-semi
DV_LOCAL_CPPFLAGS += -Wno-unused-parameter
DV_LOCAL_CPPFLAGS += -Wno-shadow
DV_LOCAL_CPPFLAGS += -Wno-exit-time-destructors
DV_LOCAL_CPPFLAGS += -Wno-documentation
DV_LOCAL_CPPFLAGS += -Wno-global-constructors
DV_LOCAL_CPPFLAGS += -Wno-padded
DV_LOCAL_CPPFLAGS += -Wno-weak-vtables
DV_LOCAL_CPPFLAGS += -Wno-variadic-macros
DV_LOCAL_CPPFLAGS += -Wno-deprecated-register
DV_LOCAL_CPPFLAGS += -Wno-sign-conversion
DV_LOCAL_CPPFLAGS += -Wno-sign-compare
DV_LOCAL_CPPFLAGS += -Wno-format-nonliteral

# TODO fix next warnings first
DV_LOCAL_CPPFLAGS += -Wno-cast-align
DV_LOCAL_CPPFLAGS += -Wno-conversion
DV_LOCAL_CPPFLAGS += -Wno-unreachable-code
DV_LOCAL_CPPFLAGS += -Wno-zero-length-array
DV_LOCAL_CPPFLAGS += -Wno-switch-enum
DV_LOCAL_CPPFLAGS += -Wno-c99-extensions
DV_LOCAL_CPPFLAGS += -Wno-missing-prototypes
DV_LOCAL_CPPFLAGS += -Wno-missing-field-initializers
DV_LOCAL_CPPFLAGS += -Wno-conditional-uninitialized
DV_LOCAL_CPPFLAGS += -Wno-covered-switch-default
DV_LOCAL_CPPFLAGS += -Wno-deprecated
DV_LOCAL_CPPFLAGS += -Wno-unused-macros
DV_LOCAL_CPPFLAGS += -Wno-disabled-macro-expansion
DV_LOCAL_CPPFLAGS += -Wno-undef
DV_LOCAL_CPPFLAGS += -Wno-non-virtual-dtor
DV_LOCAL_CPPFLAGS += -Wno-char-subscripts
DV_LOCAL_CPPFLAGS += -Wno-unneeded-internal-declaration
DV_LOCAL_CPPFLAGS += -Wno-unused-variable
DV_LOCAL_CPPFLAGS += -Wno-used-but-marked-unused
DV_LOCAL_CPPFLAGS += -Wno-missing-variable-declarations
DV_LOCAL_CPPFLAGS += -Wno-gnu-statement-expression
DV_LOCAL_CPPFLAGS += -Wno-missing-braces
DV_LOCAL_CPPFLAGS += -Wno-reorder
DV_LOCAL_CPPFLAGS += -Wno-implicit-fallthrough
DV_LOCAL_CPPFLAGS += -Wno-ignored-qualifiers
DV_LOCAL_CPPFLAGS += -Wno-shift-sign-overflow
DV_LOCAL_CPPFLAGS += -Wno-mismatched-tags
DV_LOCAL_CPPFLAGS += -Wno-missing-noreturn
DV_LOCAL_CPPFLAGS += -Wno-consumed
DV_LOCAL_CPPFLAGS += -Wno-sometimes-uninitialized
DV_LOCAL_CPPFLAGS += -Wno-reserved-id-macro
DV_LOCAL_CPPFLAGS += -Wno-old-style-cast
# we have to do it because clang3.6 bug http://bugs.mitk.org/show_bug.cgi?id=18883
DV_LOCAL_CPPFLAGS += -Wno-error=inconsistent-missing-override
DV_LOCAL_CPPFLAGS += -Wno-inconsistent-missing-override
DV_LOCAL_CPPFLAGS += -Wno-null-conversion
DV_LOCAL_CPPFLAGS += -Wno-unused-local-typedef
DV_LOCAL_CPPFLAGS += -Wno-unreachable-code-return
DV_LOCAL_CPPFLAGS += -Wno-unreachable-code-break
DV_LOCAL_CPPFLAGS += -Wno-unknown-warning-option
DV_LOCAL_CPPFLAGS += -Wno-pedantic
DV_LOCAL_CPPFLAGS += -Wno-extern-c-compat
DV_LOCAL_CPPFLAGS += -Wno-unknown-pragmas
DV_LOCAL_CPPFLAGS += -Wno-unused-private-field
DV_LOCAL_CPPFLAGS += -Wno-unused-label
DV_LOCAL_CPPFLAGS += -Wno-unused-function
DV_LOCAL_CPPFLAGS += -Wno-unused-value
DV_LOCAL_CPPFLAGS += -Wno-self-assign-field
DV_LOCAL_CPPFLAGS += -Wno-undefined-reinterpret-cast

DV_LOCAL_CPP_FEATURES += exceptions

ifeq ($(DAVA_PROFILE), true)
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
$(info ==============)
$(info profiling enabled!)
$(info ==============)

DV_LOCAL_CFLAGS += -pg
DV_LOCAL_CFLAGS += -D__DAVAENGINE_PROFILE__
endif
endif

# set exported build flags
DV_LOCAL_EXPORT_CFLAGS := $(LOCAL_CFLAGS)

# set exported used libs
DV_LOCAL_EXPORT_LDLIBS := $(LOCAL_LDLIBS)

# set included libraries
DV_LOCAL_STATIC_LIBRARIES := fmodex-prebuild

ifeq ($(DAVA_PROFILE), true)
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
DV_LOCAL_STATIC_LIBRARIES += android-ndk-profiler
endif
endif

DV_LOCAL_SHARED_LIBRARIES += fmodevent-prebuild

DV_LOCAL_STATIC_LIBRARIES += xml_android
DV_LOCAL_STATIC_LIBRARIES += png_android
DV_LOCAL_STATIC_LIBRARIES += freetype_android
DV_LOCAL_STATIC_LIBRARIES += yaml_android
DV_LOCAL_STATIC_LIBRARIES += mongodb_android
DV_LOCAL_STATIC_LIBRARIES += lua_android
DV_LOCAL_STATIC_LIBRARIES += dxt_android
DV_LOCAL_STATIC_LIBRARIES += jpeg_android
DV_LOCAL_STATIC_LIBRARIES += curl_android
DV_LOCAL_STATIC_LIBRARIES += ssl_android
DV_LOCAL_STATIC_LIBRARIES += crypto_android
DV_LOCAL_STATIC_LIBRARIES += zip_android
DV_LOCAL_STATIC_LIBRARIES += fribidi_android
DV_LOCAL_STATIC_LIBRARIES += unibreak_android
DV_LOCAL_STATIC_LIBRARIES += uv_android
DV_LOCAL_STATIC_LIBRARIES += webp_android
DV_LOCAL_STATIC_LIBRARIES += cpufeatures

DV_LOCAL_EXPORT_LDLIBS := -lGLESv1_CM -llog -lEGL -latomic

ifeq ($(APP_PLATFORM), android-14)
	DV_LOCAL_EXPORT_LDLIBS += -lGLESv2
else 
ifeq ($(APP_PLATFORM), android-15)
	DV_LOCAL_EXPORT_LDLIBS += -lGLESv2
else 
ifeq ($(APP_PLATFORM), android-16)
	DV_LOCAL_EXPORT_LDLIBS += -lGLESv2
else 
ifeq ($(APP_PLATFORM), android-17)
	DV_LOCAL_EXPORT_LDLIBS += -lGLESv2
else
	DV_LOCAL_EXPORT_LDLIBS += -lGLESv3
endif
endif
endif
endif

# clear all variables
include $(CLEAR_VARS)

# set module name
LOCAL_MODULE := libInternalPart1

# On arm architectures add sysroot option to be able to use 
# _Unwind_Backtrace and _Unwind_GetIP for collecting backtraces
# TODO: review checking arm arch and $(ANDROID_NDK_ROOT) 
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
       DV_LOCAL_CFLAGS += --sysroot=$(ANDROID_NDK_ROOT)/platforms/$(APP_PLATFORM)/arch-arm
endif

LOCAL_C_INCLUDES := $(DV_LOCAL_C_INCLUDES)
LOCAL_EXPORT_C_INCLUDES := $(DV_LOCAL_EXPORT_C_INCLUDES)
LOCAL_ARM_NEON := $(DV_LOCAL_ARM_NEON)
LOCAL_ARM_MODE := $(DV_LOCAL_ARM_MODE)
LOCAL_NEON_CFLAGS := $(DV_LOCAL_NEON_CFLAGS)
LOCAL_CPPFLAGS := $(DV_LOCAL_CPPFLAGS)
LOCAL_CFLAGS := $(DV_LOCAL_CFLAGS)
LOCAL_CPP_FEATURES := $(DV_LOCAL_CPP_FEATURES)
LOCAL_EXPORT_CFLAGS := $(DV_LOCAL_EXPORT_CFLAGS)
LOCAL_EXPORT_LDLIBS := $(DV_LOCAL_EXPORT_LDLIBS)
LOCAL_STATIC_LIBRARIES := $(DV_LOCAL_STATIC_LIBRARIES)
LOCAL_SHARED_LIBRARIES := $(DV_LOCAL_SHARED_LIBRARIES)
LOCAL_STATIC_LIBRARIES := $(DV_LOCAL_STATIC_LIBRARIES)

# set source files 
LOCAL_SRC_FILES := \
                     $(subst $(LOCAL_PATH)/,, \
                     $(wildcard $(LOCAL_PATH)/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Animation/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Autotesting/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Autotesting/*.c) \
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
                     $(wildcard $(LOCAL_PATH)/MemoryManager/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Network/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Network/Base/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Network/Services/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Network/Services/MMNet/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Network/Private/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Particles/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Platform/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Platform/TemplateAndroid/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Platform/TemplateAndroid/BacktraceAndroid/*.cpp) \
	                 $(wildcard $(LOCAL_PATH)/Platform/TemplateAndroid/ExternC/*.cpp))
                     
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

# set module name
LOCAL_MODULE := libInternal

LOCAL_C_INCLUDES := $(DV_LOCAL_C_INCLUDES)
LOCAL_EXPORT_C_INCLUDES := $(DV_LOCAL_EXPORT_C_INCLUDES)
LOCAL_ARM_NEON := $(DV_LOCAL_ARM_NEON)
LOCAL_ARM_MODE := $(DV_LOCAL_ARM_MODE)
LOCAL_NEON_CFLAGS := $(DV_LOCAL_NEON_CFLAGS)
LOCAL_CPPFLAGS := $(DV_LOCAL_CPPFLAGS)
LOCAL_CFLAGS := $(DV_LOCAL_CFLAGS)
LOCAL_CPP_FEATURES := $(DV_LOCAL_CPP_FEATURES)
LOCAL_EXPORT_CFLAGS := $(DV_LOCAL_EXPORT_CFLAGS)
LOCAL_EXPORT_LDLIBS := $(DV_LOCAL_EXPORT_LDLIBS)
LOCAL_STATIC_LIBRARIES := $(DV_LOCAL_STATIC_LIBRARIES)
LOCAL_SHARED_LIBRARIES := $(DV_LOCAL_SHARED_LIBRARIES)
LOCAL_STATIC_LIBRARIES := $(DV_LOCAL_STATIC_LIBRARIES)

LOCAL_WHOLE_STATIC_LIBRARIES := libInternalPart1

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
                     $(wildcard $(LOCAL_PATH)/Render/RHI/Common/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Render/RHI/Common/MCPP/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Render/RHI/GLES2/*.cpp) \
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
                     $(wildcard $(LOCAL_PATH)/Concurrency/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/UI/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/UI/Components/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/UI/Styles/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/UI/Layouts/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/UnitTests/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Utils/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Job/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Render/Image/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/DLC/Downloader/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/DLC/Patcher/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/DLC/Patcher/bsdiff/*.c) \
                     $(wildcard $(LOCAL_PATH)/DLC/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/DataStorage/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/DataStorage/Android/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Timer/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Notification/*.cpp))

include $(BUILD_STATIC_LIBRARY)

# include modules
$(call import-add-path,$(DAVA_ROOT)/..)
$(call import-add-path,$(DAVA_ROOT)/../External)
$(call import-add-path,$(DAVA_ROOT))

ifeq ($(DAVA_PROFILE), true)
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
$(call import-add-path,$(DAVA_ROOT)/../../Libs)
$(call import-module,android-ndk-profiler)
endif
endif

$(call import-module,android/cpufeatures)
