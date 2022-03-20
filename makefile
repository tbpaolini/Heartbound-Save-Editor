SOURCE := Heartbound Save Editor.c
NAME := $(basename $(SOURCE))
DIRECTORY := release/bin
DEPENDENCIES := gtk3/% structure/%
CFLAGS := $(shell pkg-config --cflags gtk+-3.0) $(shell pkg-config --cflags --libs gtk+-3.0) -Iincludes -fdiagnostics-color=always

.PHONY: release debug

release: $(DEPENDENCIES)
	gcc "$(SOURCE)" -o "$(DIRECTORY)/$(NAME).exe" $(CFLAGS) -O2 -mwindows

debug: $(DEPENDENCIES)
	gcc "$(SOURCE)" -o "$(DIRECTORY)/$(NAME).exe" $(CFLAGS) -g3 -mconsole

gtk3/%:
	glib-compile-schemas.exe gtk3/share/glib-2.0/schemas/
	xcopy "gtk3" "release" /S /E /D /Y

structure/%: FILES = places_list.tsv save_structure.tsv room_coordinates.tsv
structure/%:
	$(foreach file, $(FILES), xcopy "$@/$(file)" "release/lib" /D &)