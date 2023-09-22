
mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(dir $(mkfile_path))
DIRS=obj bin
MAKE?=make

RAYLIB_SRC_PATH=$(mkfile_dir)/deps/raylib
RAYGUI_SRC_PATH=$(mkfile_dir)/deps/raygui

DEPS_PATH=$(mkfile_dir)/bin
DEPS=$(DEPS_PATH)/libraylib.a $(DEPS_PATH)/raygui.o
LIBS=$(DEPS) -lopengl32 -lgdi32 -lwinmm

CC=gcc
CCF=-D_DEBUG -Og -g -std=c99 -I$(RAYLIB_SRC_PATH) -I$(RAYGUI_SRC_PATH) -Wall -Wpedantic -Wextra 
LDF=-L$(DEPS_PATH)


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

$(DEPS_PATH)/libraylib.a:
	$(MAKE) \
		RAYLIB_SRC_PATH=$(RAYLIB_SRC_PATH)\
		RAYLIB_RELEASE_PATH=$(DEPS_PATH)\
		-C $(RAYLIB_SRC_PATH)

$(RAYGUI_SRC_PATH)/raygui.o:$(RAYGUI_SRC_PATH)/raygui.c
	$(CC) $(CCF) -c $^ -o $@

$(DEPS_PATH)/raygui.o:$(RAYGUI_SRC_PATH)/raygui.o
	cp $^ $(DEPS_PATH)




obj/%.o:src/%.c
	$(CC) $(CCF) -c $^ -o $@

clean:
	rm -f bin/*
	rm -f obj/*
	rmdir $(DIRS)
