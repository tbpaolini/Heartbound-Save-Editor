SOURCE := Heartbound Save Editor.c
NAME := $(basename $(SOURCE))
DIR := release/bin
CFLAGS := $(shell pkg-config --cflags gtk+-3.0) $(shell pkg-config --cflags --libs gtk+-3.0) -Iincludes -fdiagnostics-color=always

.PHONY: release debug

release:
	gcc "$(SOURCE)" -o "$(DIR)/$(NAME).exe" $(CFLAGS) -O2 -mwindows

debug:
	gcc "$(SOURCE)" -o "$(DIR)/$(NAME).exe" $(CFLAGS) -g3 -mconsole