CXX      := g++
# -O3 for speed, -march=native to use AVX hardware
CXXFLAGS := -O3 -march=native -fPIC -Wall -std=c++11 -MMD -MP
LDFLAGS  := -shared
LDLIBS   := -lntl -lgmp -lm

SRC_DIR   := NTL_wrappers
BUILD_DIR := build
TARGET    := libntl_wrappers.so
SRCS      := $(wildcard $(SRC_DIR)/*.cpp)
OBJS      := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS      := $(OBJS:.o=.d)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(DEPS)