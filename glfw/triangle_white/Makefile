TARGET = ogl

SRCS 	= $(shell find $(SRC_DIRS) -name '*.cpp')
OBJS	= $(SRCS:%.cpp=%.o)

CPPFLAGS 	= -Iinclude -I /usr/local/bin/
CXXFLAGS	= 
LDFLAGS 	=  -L/usr/local/lib -lglfw3 -lGL -lrt -lm -ldl -lX11 -lpthread -lxcb -lXau -lXdmcp
all: $(TARGET)


$(TARGET): $(OBJS)
	g++ $(CPPFLAGS) $^ $(LDFLAGS) -o $(TARGET)

%.o: %.c
	g++ -c $(CPPFLAGS) -o $@ $<

clean: 
	rm $(TARGET) $(OBJS)

