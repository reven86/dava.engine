#-----------------------------
# Framework lib 

# set local path for lib
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := iconv_android-prebuilt
LOCAL_SRC_FILES := ../../Libs/libs/android/$(TARGET_ARCH_ABI)/libiconv_android.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := fmodex-prebuild
LOCAL_SRC_FILES         := ../../Libs/fmod/lib/android/$(TARGET_ARCH_ABI)/libfmodex.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := fmodevent-prebuild
LOCAL_SRC_FILES         := ../../Libs/fmod/lib/android/$(TARGET_ARCH_ABI)/libfmodevent.so
include $(PREBUILT_SHARED_LIBRARY)


DAVA_ROOT := $(LOCAL_PATH)

# clear all variables
include $(CLEAR_VARS)

# set module name
LOCAL_MODULE := libInternal

# set path for includes
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/Platform/TemplateAndroid/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Libs/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Libs/fmod/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Libs/lua/include

# set exported includes
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

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
                     $(wildcard $(LOCAL_PATH)/Particles/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Platform/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Platform/TemplateAndroid/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Render/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Render/2D/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Render/3D/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Render/Effects/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Render/Highlevel/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Render/Highlevel/Vegetation/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Render/Material/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Scene2D/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Scene3D/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Scene3D/Components/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Scene3D/Converters/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Scene3D/SceneFile/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Scene3D/Systems/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Sound/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Thread/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/UI/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Utils/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Job/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/Render/Image/*.cpp) \
                     $(wildcard $(LOCAL_PATH)/DLC/*.cpp))

ifneq ($(filter $(TARGET_ARCH_ABI), armeabi-v7a armeabi-v7a-hard),)
ifndef USE_NEON
USE_NEON := true
endif
ifeq ($(USE_NEON), true)
LOCAL_ARM_NEON := true
LOCAL_ARM_MODE := arm
LOCAL_NEON_CFLAGS := -mfloat-abi=softfp -mfpu=neon -march=armv7
LOCAL_CFLAGS += -DUSE_NEON
endif
endif

# set build flags
LOCAL_CFLAGS += -frtti -DGL_GLEXT_PROTOTYPES=1 -Wno-psabi
LOCAL_CFLAGS += -Wno-invalid-offsetof
LOCAL_CFLAGS += -DDAVA_FMOD

# set exported build flags
LOCAL_EXPORT_CFLAGS := $(LOCAL_CFLAGS)

# set used libs

LIBS_PATH := $(call host-path,$(LOCAL_PATH)/../../Libs/libs)

LOCAL_LDLIBS := -lGLESv1_CM -llog -lEGL
LOCAL_LDLIBS += $(LIBS_PATH)/android/$(TARGET_ARCH_ABI)/libxml_android.a
LOCAL_LDLIBS += $(LIBS_PATH)/android/$(TARGET_ARCH_ABI)/libpng_android.a
LOCAL_LDLIBS += $(LIBS_PATH)/android/$(TARGET_ARCH_ABI)/libfreetype_android.a
LOCAL_LDLIBS += $(LIBS_PATH)/android/$(TARGET_ARCH_ABI)/libyaml_android.a
LOCAL_LDLIBS += $(LIBS_PATH)/android/$(TARGET_ARCH_ABI)/libmongodb_android.a
LOCAL_LDLIBS += $(LIBS_PATH)/android/$(TARGET_ARCH_ABI)/liblua_android.a
LOCAL_LDLIBS += $(LIBS_PATH)/android/$(TARGET_ARCH_ABI)/libdxt_android.a
LOCAL_LDLIBS += $(LIBS_PATH)/android/$(TARGET_ARCH_ABI)/libjpeg_android.a
LOCAL_LDLIBS += $(LIBS_PATH)/android/$(TARGET_ARCH_ABI)/libcurl_android.a
LOCAL_LDLIBS += $(LIBS_PATH)/android/$(TARGET_ARCH_ABI)/libssl_android.a
LOCAL_LDLIBS += $(LIBS_PATH)/android/$(TARGET_ARCH_ABI)/libcrypto_android.a

APP_PLATFORM_LEVEL := $(strip $(subst android-,,$(APP_PLATFORM)))
IS_GL2_PLATFORM := $(shell (if [ $(APP_PLATFORM_LEVEL) -lt 18 ]; then echo "GLES2"; else echo "GLES3"; fi))
ifeq ($(IS_GL2_PLATFORM), GLES2)
LOCAL_LDLIBS += -lGLESv2
else ifeq ($(IS_GL2_PLATFORM), GLES3)
LOCAL_LDLIBS += -lGLESv3
else
#$(warning UNKNOWN GL VERSION)
LOCAL_LDLIBS += -lGLESv2
endif

# set exported used libs
LOCAL_EXPORT_LDLIBS := $(LOCAL_LDLIBS)

# set included libraries
LOCAL_STATIC_LIBRARIES := libbox2d
LOCAL_SHARED_LIBRARIES += iconv_android-prebuilt
LOCAL_SHARED_LIBRARIES += fmodex-prebuild
LOCAL_SHARED_LIBRARIES += fmodevent-prebuild

include $(BUILD_STATIC_LIBRARY)

# include modules
$(call import-add-path,$(DAVA_ROOT)/..)
$(call import-add-path,$(DAVA_ROOT)/../External)
$(call import-add-path,$(DAVA_ROOT)/../External/Box2D)
$(call import-add-path,$(DAVA_ROOT))

$(call import-module,box2d)