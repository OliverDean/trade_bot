# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -O3 -pthread -std=c11 -D_POSIX_C_SOURCE=200809L

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

# Libraries
LIBS = -lcurl -lcjson -lconfig -lm -lpthread
INCLUDES = -I $(INC_DIR) -I /usr/include/cjson -I /usr/include/libconfig

# Source Files
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Target
TARGET = tradbot

# Default Rule
.PHONY: all
all: $(BIN_DIR)/$(TARGET)

# Ensure the directories exist
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# Build Object Files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Link Object Files to Create Executable
$(BIN_DIR)/$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Clean Up
.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Additional Useful Targets
.PHONY: run
run: all
	./$(BIN_DIR)/$(TARGET)

.PHONY: debug
debug: CFLAGS += -g -DDEBUG
debug: clean all

.PHONY: release
release: CFLAGS += -DNDEBUG
release: clean all
