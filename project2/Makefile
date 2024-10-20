# Compiler settings - Can change to clang++ if preferred
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -std=c++11

# Build target executable:
TARGET = cache_sim

# List of source files
SRCS = main.cpp cache.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Add a rule for the object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(TARGET) $(OBJS)

# Phony targets
.PHONY: all clean