all: compile

compile:
	@mkdir -p lib
	
	@echo "Compiling socketwrapper.cpp"
	python $(EMSCRIPTEN_ROOT)/em++ -O2 -c -I. -D__DAVAENGINE_HTML5__ -DDEFINE_TYPES socketwrapper.cpp
	python $(EMSCRIPTEN_ROOT)/emar crus lib/libsocketwrapper_html5.a socketwrapper.o
	cp lib/libsocketwrapper_html5.a ../libs


