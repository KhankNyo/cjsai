
mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(dir $(mkfile_path))
DIRS=obj bin
MAKE?=make

RAYLIB_SRC_PATH=$(mkfile_dir)/deps/raylib
RAYLIB_RELEASE_PATH=$(mkfile_dir)/bin

DEPS=$(RAYLIB_RELEASE_PATH)/libraylib.a
LIBS=$(DEPS)



CC=gcc
CCF=-D_DEBUG -Og -g -Wall -Wpedantic -Wextra 
LDF=


EXEC_FMT=
ifeq ($(OS),Windows_NT)
	EXEC_FMT=.exe
endif


SRC=$(wildcard src/*.c)
OBJ=$(patsubst src/%.c,obj/%.o,$(SRC))
OUTPUT=bin/ai$(EXEC_FMT)


.PHONY:all clean


all:$(DIRS) $(OUTPUT)

$(DIRS):
	mkdir $@

$(OUTPUT):$(DIRS) $(OBJ) $(DEPS)
	$(CC) $(LDF) -o $@ $(OBJ) $(LIBS)

$(RAYLIB_RELEASE_PATH)/libraylib.a:
	$(MAKE) \
		RAYLIB_SRC_PATH=$(RAYLIB_SRC_PATH)\
		RAYLIB_RELEASE_PATH=$(RAYLIB_RELEASE_PATH)\
		-C $(RAYLIB_SRC_PATH)




obj/%.o:src/%.c
	$(CC) $(CCF) -c $^ -o $@

clean:
	rm -f bin/*
	rm -f obj/*
	rmdir $(DIRS)
