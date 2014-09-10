APP_STL := gnustl_static
APP_GNUSTL_FORCE_CPP_FEATURES := rtti

APP_CFLAGS = -marm -g

#debug
#APP_CFLAGS += -DNDK_DEBUG=1 -O0
#APP_CFLAGS += -D__DAVAENGINE_DEBUG__
#APP_OPTIM := debug
#APP_CFLAGS += -DUSE_LOCAL_RESOURCES #use local resources

#release
APP_CFLAGS += -O2
APP_OPTIM := release

APP_CFLAGS += -Wno-invalid-offsetof

APP_ABI := armeabi-v7a