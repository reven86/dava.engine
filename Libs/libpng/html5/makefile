PROGRAM_NAME = libpng_html5.a

SOURCE_DIRS := ../src
INCLUDE_DIRS := ../include/libpng

override COMPILE_FLAGS += -pipe -funroll-loops -fomit-frame-pointer -Werror -Wno-write-strings -Wno-trigraphs

SEARCH_WILDCARDS := $(addsuffix /*.c,$(SOURCE_DIRS))



$(PROGRAM_NAME) : $(notdir $(patsubst %.c, %.o, $(wildcard $(SEARCH_WILDCARDS))))
	python $(EMSCRIPTEN_ROOT)/emar rcs $@ $^
	python $(EMSCRIPTEN_ROOT)/emranlib $@
	cp $(PROGRAM_NAME) ../../libs

VPATH := $(SOURCE_DIRS)	

%.o : %.c
	python $(EMSCRIPTEN_ROOT)/emcc -O2 -c -w -MD $(addprefix -I, $(SOURCE_DIRS)) $(addprefix -I, $(INCLUDE_DIRS)) $(BUILD_FLAGS) $(COMPILE_FLAGS) $<
	
include $(wildcard *.d)

clean:
	rm -rf Debug Release