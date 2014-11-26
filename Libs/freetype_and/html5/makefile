PROGRAM_NAME = libfreetype_html5.a

override COMPILE_FLAGS += -pipe -funroll-loops -fomit-frame-pointer -Werror -Wno-write-strings -Wno-trigraphs

SOURCE_DIRS := ../src \
				../src/base \
				../src/cff \
				../src/pshinter \
				../src/psnames \
				../src/raster \
				../src/sfnt \
				../src/smooth \
				../src/truetype \
				

INCLUDE_DIRS := ../include

SOURCE_FILES := \
                  ../src/base/basepic.c \
                  ../src/base/ftapi.c \
                  ../src/base/ftbase.c \
                  ../src/base/ftbbox.c \
                  ../src/base/ftbitmap.c \
                  ../src/base/ftdbgmem.c \
                  ../src/base/ftdebug.c \
                  ../src/base/ftglyph.c \
                  ../src/base/ftinit.c \
                  ../src/base/ftpic.c \
                  ../src/base/ftstroke.c \
                  ../src/base/ftsynth.c \
                  ../src/base/ftsystem.c \
                  \
                  ../src/cff/cff.c \
                  \
                  ../src/pshinter/pshinter.c \
                  \
                  ../src/psnames/psnames.c \
                  \
                  ../src/raster/raster.c \
                  \
                  ../src/sfnt/sfnt.c \
                  \
                  ../src/smooth/smooth.c \
                  \
                  ../src/truetype/truetype.c




SEARCH_WILDCARDS := $(addsuffix /*.c,$(SOURCE_DIRS))
override BUILD_FLAGS += -DFT2_BUILD_LIBRARY



$(PROGRAM_NAME) : $(notdir $(patsubst %.c, %.o, $(SOURCE_FILES)))
	python $(EMSCRIPTEN_ROOT)/emar rcs $@ $^
	python $(EMSCRIPTEN_ROOT)/emranlib $@
	cp $(PROGRAM_NAME) ../../libs
	rm -rf *.o *.a

VPATH := $(SOURCE_DIRS)	

%.o : %.c
	python $(EMSCRIPTEN_ROOT)/emcc -O2 -c $(addprefix -I, $(SOURCE_DIRS)) $(addprefix -I, $(INCLUDE_DIRS)) $(BUILD_FLAGS) $(COMPILE_FLAGS) $<
	
include $(wildcard *.d)

clean:
	rm -rf Debug Release