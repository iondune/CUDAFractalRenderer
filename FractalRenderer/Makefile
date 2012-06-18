SOURCES=$(wildcard *.cpp) $(wildcard CabbageFramework/*.cpp) $(wildcard CabbageScene/*.cpp) $(wildcard CabbageScene/ImageLoaders/*.cpp)

OBJECTS=$(SOURCES:%.cpp=%.o)

CC=g++ -g

COMPILER_VERSION=-std=c++0x
INCLUDE=-ICabbageCore -ICabbageScene -ICabbageFramework -I/home/idunn01/csc572/glew/include/ `sdl-config --cflags` `freetype-config --cflags`



all: fractal

.cpp.o:
	$(CC) -c $(INCLUDE) -DGL_GLEXT_PROTOTYPES $< -o $@

fractal: ${OBJECTS}
	$(CC) ${COMPILER_VERSION} ${OBJECTS} -DGL_GLEXT_PROTOTYPES -lGL -lGLU -lSDL -L/home/idunn01/csc572/glew/lib/ -lGLEW `sdl-config --cflags --libs` `freetype-config --libs` -o $@

clean:
	rm -rf $(OBJECTS) fractal 2>/dev/null

remake: clean all


