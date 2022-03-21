SOURCE := Heartbound Save Editor.c
NAME := $(basename $(SOURCE))
DIRECTORY := build
ICON = icon.ico
DEPENDENCIES := gtk3 structure assets icon.o
CFLAGS := $(shell pkg-config --cflags gtk+-3.0) $(shell pkg-config --cflags --libs gtk+-3.0) -Iincludes -fdiagnostics-color=always

.PHONY: release debug clean

release: $(DEPENDENCIES)
	@echo Compiling release build...
	gcc "$(SOURCE)" icon.o -o "$(DIRECTORY)\$@\bin\$(NAME).exe" $(CFLAGS) -O2 -mwindows
	@echo Release build saved to the folder: $(DIRECTORY)\$@\

debug: $(DEPENDENCIES)
	@echo Compiling debug build...
	gcc "$(SOURCE)" icon.o -o "$(DIRECTORY)\$@\bin\$(NAME).exe" $(CFLAGS) -g3 -mconsole
	@echo Debug build saved to the folder: $(DIRECTORY)\$@\

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

assets:
	$(foreach target, $(MAKECMDGOALS), \
		xcopy "assets\icon.png" "$(DIRECTORY)\$(target)\lib\" /D /Y /I & \
	)

icon.o: assets\$(ICON)
	$(file > $*.rc,1 ICON assets\$(ICON))
	windres $*.rc $*.o

clean:
	del *.o
	del *.rc