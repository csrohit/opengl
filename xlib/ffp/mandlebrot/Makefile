target		= mandelbrot

BUILD_DIR 	= build
SRC_DIRS	= src
INC_DIRS 	= include

SRCS = $(shell find $(SRC_DIRS) -name '*.cpp' -o -name '*.cu')
OBJS = $(SRCS:%.cpp=$(BUILD_DIR)/%.o)

INC_FLAGS := $(addprefix -I,$(INC_DIRS))
LD_FLAGS  = -lX11 -lGL -lGLU -lm
CPP_FLAGS = -DXK_MISCELLANY $(INC_FLAGS) -g3

all: execute

execute: build
	./$(target)

build: $(target)

$(target): $(OBJS)
	g++ -o $@ $^ $(LD_FLAGS) $(CPP_FLAGS) $(CXXFLAGS)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	g++ $(CPP_FLAGS) $(CXXFLAGS) -o $@ -c $<
$(BUILD_DIR)/%.o: %.cu
	@mkdir -p $(dir $@)
	nvcc $(CPP_FLAGS) $(CXXFLAGS) -o $@ -c $<


clean:
	rm $(OBJS) $(target)

