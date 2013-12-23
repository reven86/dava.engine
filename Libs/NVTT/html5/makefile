PROGRAM_NAME = libdxt_html5.a

SOURCE_DIRS := ../Sources \
				../Sources/nvcore \
				../Sources/nvcore/poshlib \
				../Sources/nvimage \
				../Sources/nvmath \
				../Sources/nvtt \
				../Sources/nvtt/cuda \
				../Sources/nvtt/squish \
				../Sources/nvtt/squish/extra \
				../Sources/nvtt/tools \
				../Sources/nvtt/tests

LOCAL_SRC_FILES :=  \
					../Sources/nvcore/Library.cpp \
					../Sources/nvcore/Memory.cpp \
					../Sources/nvcore/Radix.cpp \
					../Sources/nvcore/StrLib.cpp \
					../Sources/nvcore/TextReader.cpp \
					../Sources/nvcore/TextWriter.cpp \
					../Sources/nvcore/Tokenizer.cpp \
					../Sources/nvimage/BlockDXT.cpp \
					../Sources/nvimage/ColorBlock.cpp \
					../Sources/nvimage/DirectDrawSurface.cpp \
					../Sources/nvimage/Filter.cpp \
					../Sources/nvimage/FloatImage.cpp \
					../Sources/nvimage/HoleFilling.cpp \
					../Sources/nvimage/Image.cpp \
					../Sources/nvimage/ImageIO.cpp \
					../Sources/nvimage/NormalMap.cpp \
					../Sources/nvimage/NormalMipmap.cpp \
					../Sources/nvimage/Quantize.cpp \
					../Sources/nvmath/Basis.cpp \
					../Sources/nvmath/Montecarlo.cpp \
					../Sources/nvmath/Plane.cpp \
					../Sources/nvmath/Random.cpp \
					../Sources/nvmath/SphericalHarmonic.cpp \
					../Sources/nvmath/Triangle.cpp \
					../Sources/nvmath/TriBox.cpp \
					../Sources/nvtt/CompressDXT.cpp \
					../Sources/nvtt/CompressionOptions.cpp \
					../Sources/nvtt/Compressor.cpp \
					../Sources/nvtt/CompressRGB.cpp \
					../Sources/nvtt/cuda/CudaCompressDXT.cpp \
					../Sources/nvtt/cuda/CudaUtils.cpp \
					../Sources/nvtt/Decompressor.cpp \
					../Sources/nvtt/InputOptions.cpp \
					../Sources/nvtt/nvtt.cpp \
					../Sources/nvtt/nvtt_wrapper.cpp \
					../Sources/nvtt/OptimalCompressDXT.cpp \
					../Sources/nvtt/OutputOptions.cpp \
					../Sources/nvtt/QuickCompressDXT.cpp \
					../Sources/nvtt/squish/colourfit.cpp \
					../Sources/nvtt/squish/clusterfit.cpp \
					../Sources/nvtt/squish/colourblock.cpp \
					../Sources/nvtt/squish/colourset.cpp \
					../Sources/nvtt/squish/fastclusterfit.cpp \
					../Sources/nvtt/squish/maths.cpp \
					../Sources/nvtt/squish/weightedclusterfit.cpp \
					../Sources/nvtt/tests/filtertest.cpp \
					../Sources/nvcore/Debug.cpp
					

INCLUDE_DIRS := ../Sources

override COMPILE_FLAGS += -pipe -funroll-loops -fomit-frame-pointer -Werror -Wno-write-strings -Wno-trigraphs


$(PROGRAM_NAME) : $(notdir $(patsubst %.cpp, %.o, $(LOCAL_SRC_FILES)))
	python $(EMSCRIPTEN_ROOT)/emar rcs $@ $^
	python $(EMSCRIPTEN_ROOT)/emranlib $@
	cp $(PROGRAM_NAME) ../../libs

VPATH := $(SOURCE_DIRS)	

%.o : %.cpp
	python $(EMSCRIPTEN_ROOT)/emcc -O2 -c -w -MD $(addprefix -I, $(SOURCE_DIRS)) $(addprefix -I, $(INCLUDE_DIRS)) $(BUILD_FLAGS) $(COMPILE_FLAGS) $<

	
include $(wildcard *.d)

clean:
	rm -rf Debug Release