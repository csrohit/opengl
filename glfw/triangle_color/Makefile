TARGET = ogl

SRCS 	= $(shell find $(SRC_DIRS) -name '*.cpp')
OBJS	= $(SRCS:%.cpp=%.o)

CC = g++

CPPFLAGS 	= -Iinclude -I -DGLEW_STATIC
CXXFLAGS	= 
LDFLAGS 	=  -L/usr/local/lib -L/usr/lib64 -lglfw3 -lGL -lm -lX11 -lGLEW -lGLU -lOpenGL
all: $(TARGET)


$(TARGET): $(OBJS)
	$(CC) $(CPPFLAGS) $^ $(LDFLAGS) -o $(TARGET)

%.o: %.c
	$(CC) -c $(CPPFLAGS) -o $@ $<

clean: 
	rm $(TARGET) $(OBJS)

