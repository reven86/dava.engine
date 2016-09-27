#-----------------------------
# Yaml lib

# set local path for lib
LOCAL_PATH := $(call my-dir)

# clear all variables
include $(CLEAR_VARS)

# set module name
LOCAL_MODULE    := libdxt

# set path for includes
LOCAL_C_INCLUDES := $(LOCAL_PATH)

# set exported includes
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Sources
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../libpng/include/libpng
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Sources/nvtt/squish

# set source files
LOCAL_SRC_FILES :=  \
					../../Sources/nvcore/Library.cpp \
					../../Sources/nvcore/Memory.cpp \
					../../Sources/nvcore/Radix.cpp \
					../../Sources/nvcore/StrLib.cpp \
					../../Sources/nvcore/TextReader.cpp \
					../../Sources/nvcore/TextWriter.cpp \
					../../Sources/nvcore/Tokenizer.cpp \
					../../Sources/nvimage/BlockDXT.cpp \
					../../Sources/nvimage/ColorBlock.cpp \
					../../Sources/nvimage/DirectDrawSurface.cpp \
					../../Sources/nvimage/Filter.cpp \
					../../Sources/nvimage/FloatImage.cpp \
					../../Sources/nvimage/HoleFilling.cpp \
					../../Sources/nvimage/Image.cpp \
					../../Sources/nvimage/ImageIO.cpp \
					../../Sources/nvimage/NormalMap.cpp \
					../../Sources/nvimage/NormalMipmap.cpp \
					../../Sources/nvimage/Quantize.cpp \
					../../Sources/nvmath/Basis.cpp \
					../../Sources/nvmath/Montecarlo.cpp \
					../../Sources/nvmath/Plane.cpp \
					../../Sources/nvmath/Random.cpp \
					../../Sources/nvmath/SphericalHarmonic.cpp \
					../../Sources/nvmath/Triangle.cpp \
					../../Sources/nvmath/TriBox.cpp \
					../../Sources/nvtt/CompressDXT.cpp \
					../../Sources/nvtt/CompressionOptions.cpp \
					../../Sources/nvtt/Compressor.cpp \
					../../Sources/nvtt/CompressRGB.cpp \
					../../Sources/nvtt/cuda/CudaCompressDXT.cpp \
					../../Sources/nvtt/cuda/CudaUtils.cpp \
					../../Sources/nvtt/Decompressor.cpp \
					../../Sources/nvtt/InputOptions.cpp \
					../../Sources/nvtt/nvtt.cpp \
					../../Sources/nvtt/nvtt_wrapper.cpp \
					../../Sources/nvtt/OptimalCompressDXT.cpp \
					../../Sources/nvtt/OutputOptions.cpp \
					../../Sources/nvtt/QuickCompressDXT.cpp \
					../../Sources/nvtt/squish/colourfit.cpp \
					../../Sources/nvtt/squish/clusterfit.cpp \
					../../Sources/nvtt/squish/colourblock.cpp \
					../../Sources/nvtt/squish/colourset.cpp \
					../../Sources/nvtt/squish/fastclusterfit.cpp \
					../../Sources/nvtt/squish/maths.cpp \
					../../Sources/nvtt/squish/weightedclusterfit.cpp \
					../../Sources/nvtt/tests/filtertest.cpp \
					../../Sources/nvcore/Debug.cpp
					
#../../Sources/nvcore/Debug.cpp \
#../../Sources/nvimage/ConeMap.cpp \
#../../Sources/nvtt/squish/alpha.cpp \
					
					
					

LOCAL_CFLAGS := -O2

# build static library
include $(BUILD_STATIC_LIBRARY)
