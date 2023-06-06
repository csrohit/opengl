
TARGET = window

SRC_DIRS = ./

SRCS 	= $(shell find $(SRC_DIRS) -name '*.cpp')
OBJS	= $(SRCS:%.cpp=%.o)

CPPFLAGS 	= -Iinclude -I/usr/local/include
CXXFLAGS	= 
LDFLAGS 	= -lglut -lGL -lGLU

all: $(TARGET)


$(TARGET): $(OBJS)
	gcc $(CPPFLAGS) $^ $(LDFLAGS) -o $(TARGET)

%.o: %.c
	gcc -c $(CPPFLAGS) -o $@ $<

clean: 
	rm $(TARGET) $(OBJS)

