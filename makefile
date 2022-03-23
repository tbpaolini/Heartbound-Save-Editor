SOURCE := main.c
NAME := Heartbound Save Editor
DIRECTORY := build
ICON := icon.ico
STRUCT := places_list.tsv save_structure.tsv room_coordinates.tsv
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
	xcopy "gtk3" "$(DIRECTORY)\$@" /S /E /D /Y /I
	glib-compile-schemas.exe $(DIRECTORY)\$@\share\glib-2.0\schemas
	
assets:
	xcopy "assets\icon.png" "$(DIRECTORY)\$@\lib\" /D /Y /I
	xcopy "assets\icon.ico" "$(DIRECTORY)\$@\lib\" /D /Y /I
	
structure:
	$(foreach file, $(STRUCT), xcopy "structure\$(file)" "$(DIRECTORY)\$@\lib\" /D /Y /I &)

icon.o: assets\$(ICON)
	$(file > $*.rc,1 ICON assets\$(ICON))
	windres $*.rc $*.o

clean:
	del icon.o
	del icon.rc