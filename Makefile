TARGET=atra-engine

CC=gcc

SRC=src
OBJ=obj
EXEC_SRC=exec
BUILD_DIR=bin

SRCS=$(wildcard $(SRC)/*.c)
OBJS=$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))
EXEC_SRCS=$(wildcard $(EXEC_SRC)/*.c)

TEST=test
TESTS=$(wildcard $(TEST)/*.c)
TESTBINS=$(patsubst $(TEST)/%.c, $(TEST)/bin/%, $(TESTS))
TESTASMS=$(patsubst $(TEST)/bin/%, $(TEST)/asm/%.s, $(TESTBINS))

TARGET_PATH=$(BUILD_DIR)/$(TARGET)

LIBDIR=lib
LIB=$(LIBDIR)/libatra.a

INC_DIRS=$(shell find $(SRC) -type d)
INC_FLAGS=$(addprefix -I,$(INC_DIRS))

WARNFLAGS=-Wall -Wno-missing-braces -Wextra -Wno-missing-field-initializers -Wformat=2 \
		-Wpointer-arith -Wundef -Wnested-externs -Wshadow -Wunreachable-code \
		-Wlogical-op -Wstrict-aliasing=2 -Wredundant-decls
LDFLAGS=-lm -ldl -lSDL2 -lSDL2_image -lSDL2_net -lGL -lopenal
CFLAGS=$(INC_FLAGS) -MP -MMD -Og $(WARNFLAGS) -g -std=c99

$(TARGET_PATH): $(LIB) $(BUILD_DIR) $(EXEC_SRCS) $(TARGET_PATH_WIN)
	$(CC) $(CFLAGS) $(OBJS) $(EXEC_SRCS) $(LIB_FLAGS) -o $@ $(LDFLAGS)

.PHONY: all release test clean

all: $(LIB)

release: CFLAGS=-Wall -O2 -DNDEBUG
release: clean
release: $(LIB)

$(LIB): $(LIBDIR) $(OBJ) $(OBJS)
	$(RM) $(LIB)
	ar -cvrs $(LIB) $(OBJS)

$(OBJ)/%.o: $(SRC)/%.c $(SRC)/%.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST)/bin:
	mkdir $@

$(TEST)/asm:
	mkdir $@

$(TEST)/bin/%: $(TEST)/%.c
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LDFLAGS) -lcheck

$(TEST)/asm/%.s: $(TEST)/%.c
	$(CC) $(CFLAGS) -S $< $(OBJS) -o $@ $(LDFLAGS) -lcheck

$(OBJ):
	mkdir $@

$(LIBDIR):
	mkdir $@

$(BUILD_DIR):
	mkdir $@

$(EXEC_SRC):
	mkdir $@

test: clean $(LIB) $(TEST)/bin $(TEST)/asm $(TESTBINS) $(TESTASMS) 
	for test in $(TESTBINS) ; do ./$$test ; done

clean:
	$(RM) -r $(LIB) $(OBJ) $(TARGET_PATH) $(TEST)/bin $(TEST)/asm
