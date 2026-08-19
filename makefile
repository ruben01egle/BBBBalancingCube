APP_NAME = balancing_cube_app
# Default to native build
TARGET_PLATFORM ?= bbb

# Compiler
ifeq ($(TARGET_PLATFORM), native)
    CXX = g++
else ifeq ($(TARGET_PLATFORM), bbb)
    CXX = arm-linux-gnueabihf-g++
else
    $(error Unknown platform: $(TARGET_PLATFORM))
endif

# Build
BUILD ?= Release

# Include directories
INCLUDE_DIRS = $(shell find lib -type d)

# Flags
CXX_GENERAL_FLAGS = -Wall -Wextra -Wno-psabi $(addprefix -I,$(INCLUDE_DIRS)) -Iinclude
ifeq ($(BUILD), Debug)
	CXXFLAGS = $(CXX_GENERAL_FLAGS) -O0 -g
else ifeq ($(BUILD), Release)
	CXXFLAGS = $(CXX_GENERAL_FLAGS) -O0 -DNDEBUG
else
	$(error Unknown build type: $(BUILD))
endif
LDFLAGS = -static

# Directories
SRC_DIR = src
LIB_DIR = lib
BUILD_DIR_DEBUG = debug
BUILD_DIR_RELEASE = release
BUILD_DIR_NATIVE = build_native
BUILD_DIR_BBB = build_BBB

ifeq ($(TARGET_PLATFORM), native)
	BUILD_DIR_PLATFORM = $(BUILD_DIR_NATIVE)
else ifeq ($(TARGET_PLATFORM), bbb)
	BUILD_DIR_PLATFORM = $(BUILD_DIR_BBB)
else
    $(error Unknown platform: $(TARGET_PLATFORM))
endif

ifeq ($(BUILD), Debug)
	BUILD_DIR = $(BUILD_DIR_PLATFORM)/$(BUILD_DIR_DEBUG)
else ifeq ($(BUILD), Release)
	BUILD_DIR = $(BUILD_DIR_PLATFORM)/$(BUILD_DIR_RELEASE)
else
	$(error Unknown build type: $(BUILD))
endif

# Files
SRCS = $(shell find $(SRC_DIR) -name '*.cpp') $(shell find $(LIB_DIR) -name '*.cpp')
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
OBJS := $(OBJS:$(LIB_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d) # Dependency files
TARGET = $(BUILD_DIR)/$(APP_NAME)

# Debugging: Ausgabe von SRCS und OBJS
#$(info Source files: $(SRCS))
#$(info Object files: $(OBJS))

# Default rule
all: $(TARGET)

# Linking the binary (use only .o files)
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Compiling .cpp to .o with dependency generation
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(dir $@) # Create necessary directory
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.o: $(LIB_DIR)/%.cpp
	mkdir -p $(dir $@) # Create necessary directory
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Include dependency files
-include $(DEPS)

ifeq ($(MAKECMDGOALS), all)
    $(info build with: $(CXX) $(CXX_GENERAL_FLAGS) $(CXXFLAGS))
endif

# Clean rule
clean:
	rm -rf $(BUILD_DIR_NATIVE)
	rm -rf $(BUILD_DIR_BBB)

# Phony targets
.PHONY: all clean