# 'main.c' file and all '*.c' files on the 'includes' folder
SOURCE := main.c $(wildcard includes/*.c)

# The above files but with a '.o' extension
OBJECTS := $(addsuffix .o,$(basename $(SOURCE)))

# Source code of the program's loader
LOADER_C := loader.c
LOADER_O := $(addsuffix .o,$(basename $(LOADER_C)))

# Name of the final executables
NAME := Heartbound Save Editor

# Folder where the build will go
DIRECTORY := build

# Icon for the program executable
ICON := icon.ico

# Files to dinamically build the user interface
STRUCT := places_list.tsv save_structure.tsv room_coordinates.tsv

# Files and foldes necessary for compiling the program
DEPENDENCIES := gtk3 structure assets icon.o $(OBJECTS) $(LOADER_O)

# Flags to pass to the compiler
CFLAGS := $(shell pkg-config --cflags gtk+-3.0) $(shell pkg-config --cflags --libs gtk+-3.0) -Iincludes -fdiagnostics-color=always
# Note: Here we run two shell commands to get the flags to pass to the compiler, they return the folders of the GTK 3 headers and libraries.

# Target parameters that 'make' can be run with on the tarminal
.PHONY: release debug clean analyze

# Subfolder where the release build will go
release: TARGET = release
# Activate compiler optimizations
release: CFLAGS += -O2
# Link together the compiled objects of the release build
release: $(DEPENDENCIES)
	@echo Linking release build...
	gcc $(OBJECTS) icon.o -o "$(DIRECTORY)\$(TARGET)\bin\$(NAME).exe" $(CFLAGS) -mwindows
	@echo Release build saved to the folder: $(DIRECTORY)\$(TARGET)\ 

# Subfolder where the debug build will go
debug: TARGET = debug
# Add debug symbols to the executable and define the '_DEBUG' macro
debug: CFLAGS += -g2 -Og -D_DEBUG
# Link together the compiled objects of the debug build
debug: $(DEPENDENCIES)
	@echo Linking debug build...
	gcc $(OBJECTS) icon.o -o "$(DIRECTORY)\$(TARGET)\bin\$(NAME).exe" $(CFLAGS) -mconsole
	@echo Debug build saved to the folder: $(DIRECTORY)\$(TARGET)\ 

# Compile 'main.c'
main.o: main.c
	@echo Compiling $(TARGET) build...
	gcc -c $< -o $@ $(CFLAGS)

# Compile the '*.c' files on the 'includes' folder
includes/%.o: includes/%.c
	gcc -c $< -o $@ $(CFLAGS)

# Compile the loader
$(LOADER_O): $(LOADER_C)
	gcc -c $< -o $@ -mconsole -Os
	gcc $(LOADER_O) icon.o -o "$(DIRECTORY)\$(TARGET)\$(NAME).exe" -mwindows -Os

# Copy the GTK 3 files to the build destination
gtk3:
	xcopy "gtk3" "$(DIRECTORY)\$(TARGET)" /S /E /D /Y /I
	glib-compile-schemas.exe $(DIRECTORY)\$(TARGET)\share\glib-2.0\schemas
	
# Copy the program's images to the build destination
assets:
	xcopy "assets\icon.png" "$(DIRECTORY)\$(TARGET)\lib\" /D /Y /I
	xcopy "assets\icon.ico" "$(DIRECTORY)\$(TARGET)\lib\" /D /Y /I
	xcopy "assets\textures" "$(DIRECTORY)\$(TARGET)\lib\textures\" /D /Y /I
	
# Copy the structure files to the build destination
structure:
	$(foreach file, $(STRUCT), xcopy "structure\$(file)" "$(DIRECTORY)\$(TARGET)\lib\structure\" /D /Y /I &)

# Compile the executable icon
icon.o: assets\$(ICON)
	$(file > $*.rc,1 ICON assets\$(ICON))
	windres $*.rc $*.o

# Perform some static code analysis
analyze:
	@gcc $(SOURCE) $(CFLAGS) -fanalyzer
	@gcc $(SOURCE) $(CFLAGS) -fanalyzer -fanalyzer-checker=taint
	@del a.exe

# Remove the temporary files created during compilation
clean:
	del *.o
	del includes\*.o
	del icon.rc