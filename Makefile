CXX = d++
CXXFLAGS = -Wall -g

TARGET = main
TARGET_DEL = main
SRCS = src/*
OBJS = $(SRCS:.cpp=.o)

all: $(target) run

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c  $< -o $@

run: $(TARGET)
	$(TARGET)

clean:
	rm -f $(TARGET_DEL) $(OBJS)