CXX = g++
CXXFLAGS = -Wall -g -I$(SYSTEMC_HOME)/include

TARGET = main
SRCS = $(shell find src -name '*.cpp')
OBJS = $(SRCS:.cpp=.o)

SYSTEMC_HOME ?= /usr/local/systemc
LDFLAGS  = -L$(SYSTEMC_HOME)/lib -Wl,-rpath,$(SYSTEMC_HOME)/lib
LDLIBS   = -lsystemc

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c  $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)