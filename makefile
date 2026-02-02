CC = gcc
CFLAGS = -Wall -Wextra -MMD -MP -Isrc -O3

BUILD_DIR = bin
OBJ_DIR = obj
SRC_DIR = src

SOURCES = $(SRC_DIR)/Arena.c

OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS = $(OBJECTS:.o=.d)

TARGET = arena.exe

.PHONY: clean all run

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR) $(OBJ_DIR):
	mkdir -p $@

# Linking the final executable
$(BUILD_DIR)/$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(OBJECTS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(OBJ_DIR) $(BUILD_DIR)

run: $(BUILD_DIR)/$(TARGET)
	./$(BUILD_DIR)/$(TARGET)
