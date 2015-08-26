LOCAL_PATH := $(call my-dir)

WEBP_CFLAGS := -Wall -DANDROID -DHAVE_MALLOC_H -DHAVE_PTHREAD -DWEBP_USE_THREAD

ifeq ($(APP_OPTIM),release)
  WEBP_CFLAGS += -finline-functions -ffast-math \
                 -ffunction-sections -fdata-sections
  ifeq ($(findstring clang,$(NDK_TOOLCHAIN_VERSION)),)
    WEBP_CFLAGS += -frename-registers -s
  endif
endif

ifneq ($(findstring armeabi-v7a, $(TARGET_ARCH_ABI)),)
  # Setting LOCAL_ARM_NEON will enable -mfpu=neon which may cause illegal
  # instructions to be generated for armv7a code. Instead target the neon code
  # specifically.
  NEON := c.neon
else
  NEON := c
endif

dec_srcs := \
    ../libwebp-0.4.3/src/dec/alpha.c \
    ../libwebp-0.4.3/src/dec/buffer.c \
    ../libwebp-0.4.3/src/dec/frame.c \
    ../libwebp-0.4.3/src/dec/idec.c \
    ../libwebp-0.4.3/src/dec/io.c \
    ../libwebp-0.4.3/src/dec/quant.c \
    ../libwebp-0.4.3/src/dec/tree.c \
    ../libwebp-0.4.3/src/dec/vp8.c \
    ../libwebp-0.4.3/src/dec/vp8l.c \
    ../libwebp-0.4.3/src/dec/webp.c \

demux_srcs := \
    ../libwebp-0.4.3/src/demux/demux.c \

dsp_dec_srcs := \
    ../libwebp-0.4.3/src/dsp/alpha_processing.c \
    ../libwebp-0.4.3/src/dsp/alpha_processing_sse2.c \
    ../libwebp-0.4.3/src/dsp/cpu.c \
    ../libwebp-0.4.3/src/dsp/dec.c \
    ../libwebp-0.4.3/src/dsp/dec_clip_tables.c \
    ../libwebp-0.4.3/src/dsp/dec_mips32.c \
    ../libwebp-0.4.3/src/dsp/dec_neon.$(NEON) \
    ../libwebp-0.4.3/src/dsp/dec_sse2.c \
    ../libwebp-0.4.3/src/dsp/lossless.c \
    ../libwebp-0.4.3/src/dsp/lossless_mips32.c \
    ../libwebp-0.4.3/src/dsp/lossless_neon.$(NEON) \
    ../libwebp-0.4.3/src/dsp/lossless_sse2.c \
    ../libwebp-0.4.3/src/dsp/upsampling.c \
    ../libwebp-0.4.3/src/dsp/upsampling_neon.$(NEON) \
    ../libwebp-0.4.3/src/dsp/upsampling_sse2.c \
    ../libwebp-0.4.3/src/dsp/yuv.c \
    ../libwebp-0.4.3/src/dsp/yuv_mips32.c \
    ../libwebp-0.4.3/src/dsp/yuv_sse2.c \

dsp_enc_srcs := \
    ../libwebp-0.4.3/src/dsp/enc.c \
    ../libwebp-0.4.3/src/dsp/enc_avx2.c \
    ../libwebp-0.4.3/src/dsp/enc_mips32.c \
    ../libwebp-0.4.3/src/dsp/enc_neon.$(NEON) \
    ../libwebp-0.4.3/src/dsp/enc_sse2.c \

enc_srcs := \
    ../libwebp-0.4.3/src/enc/alpha.c \
    ../libwebp-0.4.3/src/enc/analysis.c \
    ../libwebp-0.4.3/src/enc/backward_references.c \
    ../libwebp-0.4.3/src/enc/config.c \
    ../libwebp-0.4.3/src/enc/cost.c \
    ../libwebp-0.4.3/src/enc/filter.c \
    ../libwebp-0.4.3/src/enc/frame.c \
    ../libwebp-0.4.3/src/enc/histogram.c \
    ../libwebp-0.4.3/src/enc/iterator.c \
    ../libwebp-0.4.3/src/enc/picture.c \
    ../libwebp-0.4.3/src/enc/picture_csp.c \
    ../libwebp-0.4.3/src/enc/picture_psnr.c \
    ../libwebp-0.4.3/src/enc/picture_rescale.c \
    ../libwebp-0.4.3/src/enc/picture_tools.c \
    ../libwebp-0.4.3/src/enc/quant.c \
    ../libwebp-0.4.3/src/enc/syntax.c \
    ../libwebp-0.4.3/src/enc/token.c \
    ../libwebp-0.4.3/src/enc/tree.c \
    ../libwebp-0.4.3/src/enc/vp8l.c \
    ../libwebp-0.4.3/src/enc/webpenc.c \

mux_srcs := \
    ../libwebp-0.4.3/src/mux/muxedit.c \
    ../libwebp-0.4.3/src/mux/muxinternal.c \
    ../libwebp-0.4.3/src/mux/muxread.c \

utils_dec_srcs := \
    ../libwebp-0.4.3/src/utils/bit_reader.c \
    ../libwebp-0.4.3/src/utils/color_cache.c \
    ../libwebp-0.4.3/src/utils/filters.c \
    ../libwebp-0.4.3/src/utils/huffman.c \
    ../libwebp-0.4.3/src/utils/quant_levels_dec.c \
    ../libwebp-0.4.3/src/utils/random.c \
    ../libwebp-0.4.3/src/utils/rescaler.c \
    ../libwebp-0.4.3/src/utils/thread.c \
    ../libwebp-0.4.3/src/utils/utils.c \

utils_enc_srcs := \
    ../libwebp-0.4.3/src/utils/bit_writer.c \
    ../libwebp-0.4.3/src/utils/huffman_encode.c \
    ../libwebp-0.4.3/src/utils/quant_levels.c \

################################################################################
# libwebp

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    $(dec_srcs) \
    $(dsp_dec_srcs) \
    $(utils_dec_srcs) \
    $(enc_srcs) \
    $(dsp_enc_srcs) \
    $(utils_enc_srcs) \

LOCAL_CFLAGS := $(WEBP_CFLAGS)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src

LOCAL_STATIC_LIBRARIES := cpufeatures

# prefer arm over thumb mode for performance gains
LOCAL_ARM_MODE := arm

LOCAL_MODULE := libwebp_android

include $(BUILD_STATIC_LIBRARY)

################################################################################

$(call import-module,android/cpufeatures)
