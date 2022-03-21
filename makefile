SOURCE := Heartbound Save Editor.c
NAME := $(basename $(SOURCE))
DIRECTORY := build
DEPENDENCIES := gtk3 structure
CFLAGS := $(shell pkg-config --cflags gtk+-3.0) $(shell pkg-config --cflags --libs gtk+-3.0) -Iincludes -fdiagnostics-color=always

.PHONY: release debug

release: $(DEPENDENCIES)
	@gcc "$(SOURCE)" -o "$(DIRECTORY)\$@\bin\$(NAME).exe" $(CFLAGS) -O2 -mwindows

debug: $(DEPENDENCIES)
	@gcc "$(SOURCE)" -o "$(DIRECTORY)\$@\bin\$(NAME).exe" $(CFLAGS) -g3 -mconsole

gtk3:
	$(foreach target, $(MAKECMDGOALS), \
		xcopy "gtk3" "$(DIRECTORY)\$(target)" /S /E /D /Y /I & \
		glib-compile-schemas.exe $(DIRECTORY)\$(target)\share\glib-2.0\schemas\ &\
	)

structure: FILES = places_list.tsv save_structure.tsv room_coordinates.tsv
structure:
	$(foreach target, $(MAKECMDGOALS), \
		$(foreach file, $(FILES), xcopy "structure\$(file)" "$(DIRECTORY)\$(target)\lib\" /D /Y /I &) \
	)