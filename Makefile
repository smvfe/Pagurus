BUILD_DIR  := build
BUILD_TYPE ?= Release

.PHONY: all build run clean

all: build

$(BUILD_DIR)/CMakeCache.txt:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

build: $(BUILD_DIR)/CMakeCache.txt
	cmake --build $(BUILD_DIR)

run: build
	./$(BUILD_DIR)/pagurus

clean:
	rm -rf $(BUILD_DIR)
