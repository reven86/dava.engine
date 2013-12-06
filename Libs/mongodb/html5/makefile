PROGRAM_NAME = libmongodb_html5.a

SOURCE_DIRS := ../src \
				
override COMPILE_FLAGS += -pipe -funroll-loops -fomit-frame-pointer -Werror -Wno-write-strings -Wno-trigraphs

INCLUDE_DIRS := ../include

SOURCE_FILES := \
                  ../src/mongo.c \
                  ../src/bson.c \
                  ../src/md5.c \
                  ../src/numbers.c \
                  ../src/encoding.c \
                  ../src/env_standard.c \
				  ../src/gridfs.c



SEARCH_WILDCARDS := $(addsuffix /*.c,$(SOURCE_DIRS))
override BUILD_FLAGS += 


$(PROGRAM_NAME) : $(notdir $(patsubst %.c, %.o, $(SOURCE_FILES)))
	python $(EMSCRIPTEN_ROOT)/emar rcs $@ $^ 
	python $(EMSCRIPTEN_ROOT)/emranlib $@
	cp $(PROGRAM_NAME) ../../libs

VPATH := $(SOURCE_DIRS)	

%.o : %.c
	python $(EMSCRIPTEN_ROOT)/emcc -O2 -c -MD $(addprefix -I, $(SOURCE_DIRS)) $(addprefix -I, $(INCLUDE_DIRS)) $(BUILD_FLAGS) $(COMPILE_FLAGS) $<
	
include $(wildcard *.d)

clean:
	rm -rf Debug Release