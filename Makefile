# Compiler and flags
CXX = g++
CXXFLAGS = -O2 -fPIC -Wall -std=c++11

# Directories
SRC_DIR = NTL_wrappers
BUILD_DIR = build

# Libraries to link
LIBS = -lntl -lgmp

# Output shared library
TARGET = libntl_wrappers.so

# Source and object files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# Default target
all: $(TARGET)

# Create build directory if needed
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link shared library
$(TARGET): $(OBJS)
	$(CXX) -shared $^ -o $@ $(LIBS)

# Clean
clean:
	rm -rf $(BUILD_DIR) *.so
