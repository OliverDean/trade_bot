CC = gcc
CFLAGS = -Wall -Wextra -Werror -O2

SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

SRCS = $(wildcard $(SRC_DIR)/*.c)
DEPS = $(wildcard $(INC_DIR)/*.h)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
TARGET = trading_algorithms

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	@mkdir -p $(OBJ_DIR)
	$(CC) -c -o $@ $< $(CFLAGS) -I $(INC_DIR)

$(BIN_DIR)/$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) -o $@ $^ $(CFLAGS)

all: $(BIN_DIR)/$(TARGET)

.PHONY: clean

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
