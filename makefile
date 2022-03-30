SOURCE := main.c $(wildcard includes/*.c)
OBJECTS := $(addsuffix .o,$(basename $(SOURCE)))
NAME := Heartbound Save Editor
DIRECTORY := build
ICON := icon.ico
STRUCT := places_list.tsv save_structure.tsv room_coordinates.tsv
DEPENDENCIES := gtk3 structure assets icon.o $(OBJECTS)
CFLAGS := $(shell pkg-config --cflags gtk+-3.0) $(shell pkg-config --cflags --libs gtk+-3.0) -Iincludes -fdiagnostics-color=always

.PHONY: release debug clean

release: TARGET = release
release: CFLAGS += -O2
release: $(DEPENDENCIES)
	@echo Linking release build...
	gcc $(OBJECTS) icon.o -o "$(DIRECTORY)\$(TARGET)\bin\$(NAME).exe" $(CFLAGS) -mwindows
	@echo Release build saved to the folder: $(DIRECTORY)\$(TARGET)\ 

debug: TARGET = debug
debug: CFLAGS += -g3 -D_DEBUG
debug: $(DEPENDENCIES)
	@echo Linking debug build...
	gcc $(OBJECTS) icon.o -o "$(DIRECTORY)\$(TARGET)\bin\$(NAME).exe" $(CFLAGS) -mconsole
	@echo Debug build saved to the folder: $(DIRECTORY)\$(TARGET)\ 

main.o: main.c
	@echo Compiling $(TARGET) build...
	gcc -c $< -o $@ $(CFLAGS)

includes/%.o: includes/%.c
	gcc -c $< -o $@ $(CFLAGS)

gtk3:
	xcopy "gtk3" "$(DIRECTORY)\$(TARGET)" /S /E /D /Y /I
	glib-compile-schemas.exe $(DIRECTORY)\$(TARGET)\share\glib-2.0\schemas
	
assets:
	xcopy "assets\icon.png" "$(DIRECTORY)\$(TARGET)\lib\" /D /Y /I
	xcopy "assets\icon.ico" "$(DIRECTORY)\$(TARGET)\lib\" /D /Y /I
	xcopy "assets\textures" "$(DIRECTORY)\$(TARGET)\lib\textures\" /D /Y /I
	
structure:
	$(foreach file, $(STRUCT), xcopy "structure\$(file)" "$(DIRECTORY)\$(TARGET)\lib\structure\" /D /Y /I &)

icon.o: assets\$(ICON)
	$(file > $*.rc,1 ICON assets\$(ICON))
	windres $*.rc $*.o

clean:
	del *.o
	del includes\*.o
	del icon.rc