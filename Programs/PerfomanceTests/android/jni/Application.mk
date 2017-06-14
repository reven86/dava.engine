APP_STL := c++_shared
APP_CPPFLAGS := -frtti -fexceptions

#debug
# not forget to build: "ndk-build NDK_DEBUG=1"
APP_CFLAGS += -DNDK_DEBUG=1 -O0
APP_CFLAGS += -D__DAVAENGINE_DEBUG__
# http://en.cppreference.com/w/cpp/numeric/fenv
#APP_CFLAGS += -fnon-call-exceptions
APP_OPTIM := debug
#APP_CFLAGS += -DUSE_LOCAL_RESOURCES #use local resources

#release
#APP_CFLAGS += -O2
#APP_OPTIM := release

APP_CFLAGS += -Qunused-arguments

APP_CFLAGS += -Wno-invalid-offsetof
APP_LDLIBS := -fuse-ld=gold -fno-exceptions

APP_ABI += armeabi-v7a
APP_ABI += x86
APP_PLATFORM := android-14

NDK_TOOLCHAIN_VERSION=clang
