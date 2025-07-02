# Diretórios
SRC    := src
BUILD  := build
BIN    := huffbin
CC     := gcc
CFLAGS := -std=c11 -Wall -Wextra -O2

DEBUG  ?= 0
ifeq ($(DEBUG),1)
    CFLAGS += -g -DDEBUG
endif

LOG ?= 0
ifeq ($(LOG),1)
    CFLAGS += -DFGK_DEBUG_LOG
endif

SRCS := $(wildcard $(SRC)/*.c)
OBJS := $(patsubst $(SRC)/%.c, $(BUILD)/%.o, $(SRCS))

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Regra para todos objetos EXCETO main.o depende de .h correspondente
$(BUILD)/%.o: $(SRC)/%.c $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $@

clean:
	rm -rf $(BUILD) $(BIN) fgk_debug.log

.PHONY: all clean
