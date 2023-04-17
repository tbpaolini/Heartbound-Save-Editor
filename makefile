# On Windows, set the default shell to CMD
ifeq ($(OS),Windows_NT)
    SHELL := cmd.exe
endif

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
DIRECTORY := build\windows

# Icon and metadata for the program executable
RESOURCES := resources.rc
RESOURCES_O := $(addsuffix .o,$(basename $(RESOURCES)))
ICON := icon.ico

# Names of the GTK icons used inside the program (comma-separated)
GTK_ICONS := document-save,document-open,dialog-error,dialog-warning,dialog-information,image-loading,image-missing,pan-down-symbolic,pan-up-symbolic

# Files to dynamically build the user interface
STRUCT := places_list.tsv save_structure.tsv room_coordinates.tsv turtlefarm_crops.tsv turtlefarm_crops.css

# Files and folders necessary for compiling the program
DEPENDENCIES := gtk3 structure assets $(RESOURCES_O) $(OBJECTS) $(LOADER_O)

# Flags to pass to the compiler
CFLAGS := $(shell pkg-config --cflags --libs gtk+-3.0) -Iincludes -fdiagnostics-color=always
# Note: Here we run a shell command to get the flags to pass to the compiler, they return the folders of the GTK 3 headers and libraries.

# Target parameters that 'make' can be run with on the terminal
.PHONY: release debug zip clean analyze

# Subfolder where the release build will go
release: TARGET = release
# Activate compiler optimizations
release: CFLAGS += -O3
# Link together the compiled objects of the release build
release: $(DEPENDENCIES)
	@echo Linking release build...
	gcc $(OBJECTS) $(RESOURCES_O) -o "$(DIRECTORY)\$(TARGET)\bin\$(NAME).exe" $(CFLAGS) -mwindows
	@echo Release build saved to the folder: $(DIRECTORY)\$(TARGET)\ 

# Subfolder where the debug build will go
debug: TARGET = debug
# Add debug symbols to the executable and define the '_DEBUG' macro
debug: CFLAGS += -g2 -Og -D_DEBUG
# Add icons used by the GTK Inspector
debug: GTK_ICONS += ,find-location-symbolic,view-list-symbolic,media-record,media-playback-pause,object-select-symbolic,list-add-symbolic,list-remove-symbolic,window-close,window-close-symbolic,window-maximize-symbolic,window-minimize-symbolic,window-restore-symbolic
# Link together the compiled objects of the debug build
debug: $(DEPENDENCIES)
	@echo Linking debug build...
	gcc $(OBJECTS) $(RESOURCES_O) -o "$(DIRECTORY)\$(TARGET)\bin\$(NAME).exe" $(CFLAGS) -mconsole
	@echo Debug build saved to the folder: $(DIRECTORY)\$(TARGET)\ 

# Compile 'main.c'
main.o: main.c
	@echo Compiling $(TARGET) build...
	gcc -c $< -o $@ $(CFLAGS)

# Compile the '*.c' files on the 'includes' folder
includes/%.o: includes/%.c
	gcc -c $< -o $@ $(CFLAGS)

# Compile the loader
$(LOADER_O): $(LOADER_C) $(RESOURCES_O)
	gcc -c $< -o $@ -mwindows -Os
	gcc $(LOADER_O) $(RESOURCES_O) -o "$(DIRECTORY)\$(TARGET)\$(NAME).exe" -mwindows -Os

# Copy the GTK 3 files to the build destination
gtk3:
	@echo Copying GTK files into the build...
	python utils\gtk_copy.py src="$@" dst="$(DIRECTORY)\$(TARGET)" icons="$(GTK_ICONS)"
	glib-compile-schemas.exe $(DIRECTORY)\$(TARGET)\share\glib-2.0\schemas
	
# Copy the program's images to the build destination
assets:
	xcopy "assets\icon.png" "$(DIRECTORY)\$(TARGET)\lib\" /D /Y /I
	xcopy "assets\icon.ico" "$(DIRECTORY)\$(TARGET)\lib\" /D /Y /I
	xcopy "assets\textures" "$(DIRECTORY)\$(TARGET)\lib\textures\" /D /Y /I
	
# Copy the structure files to the build destination
structure:
	$(foreach file, $(STRUCT), xcopy "structure\$(file)" "$(DIRECTORY)\$(TARGET)\lib\structure\" /D /Y /I &)

# Compile the executable icon and metadata
$(RESOURCES_O): assets\$(ICON) $(RESOURCES)
	windres $*.rc $*.o

# Create a zip file with the release build
zip: TARGET = release
zip:
	move "$(DIRECTORY)\$(TARGET)" "$(DIRECTORY)\$(NAME)"
	-cd "$(DIRECTORY)" && ..\..\utils\7z.exe a -tzip -mx9 "$(NAME).zip" "$(NAME)" && cd ..\ 
	move "$(DIRECTORY)\$(NAME)" "$(DIRECTORY)\$(TARGET)"

# Perform some static code analysis
analyze:
	@gcc $(SOURCE) $(CFLAGS) -fanalyzer
	@gcc $(SOURCE) $(CFLAGS) -fanalyzer -fanalyzer-checker=taint
	@del a.exe

# Remove the temporary files created during compilation
clean:
	del *.o
	del includes\*.o