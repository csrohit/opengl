TARGET = vao

BUILD_DIR = build
SRCS 	= $(shell find $(SRC_DIRS) -name '*.cpp')
OBJS	= $(SRCS:%.cpp=$(BUILD_DIR)/%.o)

CPPFLAGS 	= -Iinclude
CXXFLAGS	= 
LDFLAGS 	=  -L/usr/local/lib -lglfw3 -lGL -lX11 -lGLEW 
all: execute

execute: $(TARGET)
	./$<

$(TARGET): $(OBJS)
	g++ $(CPPFLAGS) $^ $(LDFLAGS) -o $(TARGET)

%.o: %.c
	g++ -c $(CPPFLAGS) -o $@ $<

clean: 
	rm $(TARGET) $(OBJS)

