CC = gcc

CFLAGS = -Wall -Wextra -MMD -MP

BUILD_DIR=bin
OBJ_DIR= obj
SOURCES = arena.c
OBJECTS = $(patsubst %.c,$(OBJDIR)/%.o,$(SOURCES))
DEPS = $(OBJECTS:.o=.d)
VPATH = src

TARGET = arena

.PHONY: clean all run

all: $(BUILD_DIR)/$(TARGET)


$(BUILD_DIR) $(OBJ_DIR):
	mkdir -p $@


$(BUILD_DIR)/$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	$(CC) $^ -o $@


$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)


clean:
	rm -rf $(OBJ_DIR) $(BUILD_DIR)

run: $(BUILD_DIR)/$(TARGET)
	./$(BUILD_DIR)/$(TARGET)
	
