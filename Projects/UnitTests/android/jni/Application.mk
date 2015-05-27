APP_STL := gnustl_static
APP_GNUSTL_FORCE_CPP_FEATURES := rtti

#APP_CFLAGS = -marm -g

#debug
#APP_CFLAGS += -DNDK_DEBUG=1 -O0 -g
#APP_CFLAGS += -D__DAVAENGINE_DEBUG__
#APP_OPTIM := debug
#APP_CFLAGS += -DUSE_LOCAL_RESOURCES #use local resources

MEMORY_SANITIZE := true

ifeq ($(MEMORY_SANITIZE), true)
APP_CFLAGS   += -fsanitize=address -fno-omit-frame-pointer -fsanitize-memory-track-origins=2
APP_STL      := gnustl_shared
APP_LDFLAGS  := -fsanitize=address -v -fuse-ld=gold
endif

# release
# use -g for MEMORY_SANITIZER and -O1 (no inline)
APP_CFLAGS += -O1 -g
APP_OPTIM := release

APP_CFLAGS += -Qunused-arguments

APP_ABI := armeabi-v7a # x86
APP_PLATFORM := android-14

# we have to use last ndk10e with clang3.6, clang - point to clang3.6 in ndk10e
NDK_TOOLCHAIN_VERSION=clang
