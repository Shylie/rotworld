RAYLIB_PATH   ?= $(LIBS_PATH)/raylib
CHIPMUNK_PATH ?= $(LIBS_PATH)/Chipmunk2D

CXX            = g++

HEADER_FILES   = drawing.h
SOURCE_FILES   = main.cpp drawing.cpp
OBJS           = $(patsubst %.cpp, %.o, $(SOURCE_FILES))

INCLUDE_PATHS  = -I$(RAYLIB_PATH)/src -I$(CHIPMUNK_PATH)/include/chipmunk
CXXFLAGS      := $(CXXFLAGS)
LDFLAGS       := -L$(RAYLIB_PATH)/src -L$(CHIPMUNK_PATH)/build/src/ -lraylib -lchipmunk -lwinmm -lgdi32 -static $(LDFLAGS)

build: $(OBJS) $(HEADER_FILES)
	$(CXX) $(OBJS) $(CXXFLAGS) $(LDFLAGS) -o rotworld.exe

run: build
	rotworld

%.o: %.cpp
	$(CXX) -c $(INCLUDE_PATHS) $(CXXFLAGS) $^ -o $@

clean:
	rm *.o
	rm rotworld.exe
