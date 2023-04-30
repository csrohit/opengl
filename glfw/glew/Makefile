TARGET = ogl

SRCS 	= $(shell find $(SRC_DIRS) -name '*.cpp')
OBJS	= $(SRCS:%.cpp=%.o)

CPPFLAGS 	= -Iinclude -I /usr/local/bin/
CXXFLAGS	= 
LDFLAGS 	=  -L/usr/local/lib -L/usr/lib64 -lglfw3 -lGL -lm -lX11 -lGLEW -lGLU -lOpenGL
all: $(TARGET)


$(TARGET): $(OBJS)
	gcc $(CPPFLAGS) $^ $(LDFLAGS) -o $(TARGET)

%.o: %.c
	gcc -c $(CPPFLAGS) -o $@ $<

clean: 
	rm $(TARGET) $(OBJS)

