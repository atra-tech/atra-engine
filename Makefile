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

LIBDIR=lib
LIB=$(LIBDIR)/atra.a

INC_DIRS=$(shell find $(SRC) -type d)
INC_FLAGS=$(addprefix -I,$(INC_DIRS))

LDFLAGS=-lm -ldl -lSDL2 -lSDL2_image -lGL
CFLAGS=$(INC_FLAGS) -MP -MMD -O0 -Wall -Wextra -Wpedantic -g -std=c99

$(BUILD_DIR)/$(TARGET): $(LIB) $(BUILD_DIR) $(EXEC_SRCS)
	$(CC) $(CFLAGS) $(SRCS) $(EXEC_SRCS) -o $@ $(LDFLAGS)

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
	$(RM) -r $(LIB) $(OBJ) $(BUILD_DIR) $(TEST)/bin $(TEST)/asm
