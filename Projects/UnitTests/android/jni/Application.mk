# next line needed for teamcity special build agent to test ASAN
MEMORY_SANITIZE := false

APP_STL := gnustl_shared
APP_CPPFLAGS := -frtti -fexceptions

#APP_CFLAGS = -marm -g

#debug
#APP_CFLAGS += -DNDK_DEBUG=1 -O0 -g
#APP_CFLAGS += -D__DAVAENGINE_DEBUG__
#APP_OPTIM := debug
#APP_CFLAGS += -DUSE_LOCAL_RESOURCES #use local resources

# release
APP_OPTIM := release
APP_CFLAGS += -Qunused-arguments
APP_CFLAGS += -O2

ifeq ($(MEMORY_SANITIZE), true)
APP_OPTIM := debug
# https://code.google.com/p/address-sanitizer/wiki/Android
APP_CFLAGS   := -fsanitize=address -fno-omit-frame-pointer
# use -g for MEMORY_SANITIZER and -O1 (no inline)
APP_CFLAGS   += -O1 -g # Warnign! -O0 - will crush clang++ with memory sanitizer
APP_STL      := gnustl_shared # work with gnustl_shared or c++_shared, better with gnustl_shared
                           # but because http://stackoverflow.com/questions/29018310/android-ndk-clang-compiler-cant-find-stdmake-unique
                           # we have to stick with c++_shared
APP_LDFLAGS  := -fsanitize=address
#LIBCXX_FORCE_REBUILD := true # if you want to see bug in stl line code
endif

APP_ABI := armeabi-v7a x86
APP_PLATFORM := android-14

# we have to use last ndk10e with clang3.6, clang - point to clang3.6 in ndk10e
NDK_TOOLCHAIN_VERSION=clang
