.PHONY: clean 

PROGRAM_NAME = libFramework.a

DIR_SOURCES_MAIN := ../../Sources/Internal

INCLUDE_SYS_TYPES_H := 1

SOURCE_DIRS := $(DIR_SOURCES_MAIN) \
 				$(DIR_SOURCES_MAIN)/Animation \
 				$(DIR_SOURCES_MAIN)/Asset \
 				$(DIR_SOURCES_MAIN)/Autotesting \
                $(DIR_SOURCES_MAIN)/Base \
                $(DIR_SOURCES_MAIN)/Base/Radix \
                $(DIR_SOURCES_MAIN)/Collision \
                $(DIR_SOURCES_MAIN)/Core \
                $(DIR_SOURCES_MAIN)/Database \
                $(DIR_SOURCES_MAIN)/Debug \
                $(DIR_SOURCES_MAIN)/DLC \
                $(DIR_SOURCES_MAIN)/Entity \
				$(DIR_SOURCES_MAIN)/FileSystem \
                $(DIR_SOURCES_MAIN)/Input \
                $(DIR_SOURCES_MAIN)/Job \
                $(DIR_SOURCES_MAIN)/Math \
                $(DIR_SOURCES_MAIN)/Math/Neon \
                $(DIR_SOURCES_MAIN)/Network \
                $(DIR_SOURCES_MAIN)/Particles \
                $(DIR_SOURCES_MAIN)/Platform \
                $(DIR_SOURCES_MAIN)/Platform/TemplateHtml5 \
                $(DIR_SOURCES_MAIN)/Render \
                $(DIR_SOURCES_MAIN)/Render/2D \
                $(DIR_SOURCES_MAIN)/Render/3D \
                $(DIR_SOURCES_MAIN)/Render/Effects \
                $(DIR_SOURCES_MAIN)/Render/Highlevel \
                $(DIR_SOURCES_MAIN)/Render/Material \
                $(DIR_SOURCES_MAIN)/Scene2D \
                $(DIR_SOURCES_MAIN)/Scene3D \
                $(DIR_SOURCES_MAIN)/Scene3D/Components \
                $(DIR_SOURCES_MAIN)/Scene3D/Converters \
                $(DIR_SOURCES_MAIN)/Scene3D/SceneFile \
                $(DIR_SOURCES_MAIN)/Scene3D/Systems \
                $(DIR_SOURCES_MAIN)/Sound \
                $(DIR_SOURCES_MAIN)/UI \
                $(DIR_SOURCES_MAIN)/Utils

SOURCE_DIRS      := $(addprefix ../,$(SOURCE_DIRS))
override BUILD_FLAGS += -D__DAVAENGINE_HTML5__
INCLUDE_DIRS = ../../../Sources/External \
				../../../Sources/External/Box2D \
				../../../Sources/Internal \
				../../../Libs/Include \
				../inc \
				../../../Libs/fmod/include \
				../../../Libs/ 

SEARCH_WILDCARDS := $(addsuffix /*.cpp,$(SOURCE_DIRS)) 
override COMPILE_FLAGS += -pipe

$(PROGRAM_NAME) : $(notdir $(patsubst %.cpp, %.o, $(wildcard $(SEARCH_WILDCARDS))))
	python $(EMSCRIPTEN_ROOT)/emar rcs $@ $^
	python $(EMSCRIPTEN_ROOT)/emranlib $@

VPATH := $(SOURCE_DIRS)

%.o : %.cpp
	python $(EMSCRIPTEN_ROOT)/em++ -O2 -c -w $(addprefix -I, $(SOURCE_DIRS)) $(addprefix -I, $(INCLUDE_DIRS)) $(BUILD_FLAGS) $(COMPILE_FLAGS) $<
	
include $(wildcard *.d)

clean:
	rm -rf Debug Release