cmake_minimum_required(VERSION 3.6) 

project(dxt)
set (CMAKE_CXX_STANDARD 11)
include_directories("${PROJECT_SOURCE_DIR}/Sources")
include_directories("${PROJECT_SOURCE_DIR}/Sources/nvtt/squish")
include_directories("${PROJECT_SOURCE_DIR}/../include/libpng")
add_definitions(-DNV_NO_DEBUG)

set (SOURCE_FILES
    Sources/nvcore/Debug.cpp
    Sources/nvcore/Library.cpp
    Sources/nvcore/Memory.cpp
    Sources/nvcore/Radix.cpp
    Sources/nvcore/StrLib.cpp
    Sources/nvcore/TextReader.cpp
    Sources/nvcore/TextWriter.cpp
    Sources/nvcore/Tokenizer.cpp
    Sources/nvcore/poshlib/posh.cpp
    Sources/nvimage/BlockDXT.cpp
    Sources/nvimage/ColorBlock.cpp
    Sources/nvimage/DirectDrawSurface.cpp
    Sources/nvimage/Filter.cpp
    Sources/nvimage/FloatImage.cpp
    Sources/nvimage/HoleFilling.cpp
    Sources/nvimage/Image.cpp
    Sources/nvimage/ImageIO.cpp
    Sources/nvimage/NormalMap.cpp
    Sources/nvimage/NormalMipmap.cpp
    Sources/nvimage/Quantize.cpp
    Sources/nvmath/Basis.cpp
    Sources/nvmath/Montecarlo.cpp
    Sources/nvmath/Plane.cpp
    Sources/nvmath/Random.cpp
    Sources/nvmath/SphericalHarmonic.cpp
    Sources/nvmath/Triangle.cpp
    Sources/nvmath/TriBox.cpp
    Sources/nvtt/CompressDXT.cpp
    Sources/nvtt/CompressionOptions.cpp
    Sources/nvtt/Compressor.cpp
    Sources/nvtt/CompressRGB.cpp
    Sources/nvtt/Decompressor.cpp
    Sources/nvtt/InputOptions.cpp
    Sources/nvtt/nvtt.cpp
    Sources/nvtt/nvtt_wrapper.cpp
    Sources/nvtt/OptimalCompressDXT.cpp
    Sources/nvtt/OutputOptions.cpp
    Sources/nvtt/QuickCompressDXT.cpp
    Sources/nvtt/cuda/CudaCompressDXT.cpp
    Sources/nvtt/cuda/CudaUtils.cpp
    Sources/nvtt/squish/clusterfit.cpp
    Sources/nvtt/squish/colourblock.cpp
    Sources/nvtt/squish/colourfit.cpp
    Sources/nvtt/squish/colourset.cpp
    Sources/nvtt/squish/fastclusterfit.cpp
    Sources/nvtt/squish/maths.cpp
    Sources/nvtt/squish/weightedclusterfit.cpp
)

add_library("${PROJECT_NAME}" STATIC ${SOURCE_FILES} ) 
