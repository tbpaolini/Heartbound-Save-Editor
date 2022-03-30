SOURCE := main.c $(wildcard includes/*.c)			# 'main.c' file and all '*.c' files on the 'includes' folder
OBJECTS := $(addsuffix .o,$(basename $(SOURCE)))	# The above files but with a '.o' extension
NAME := Heartbound Save Editor						# Name of the final executable
DIRECTORY := build									# Folder where the build will go
ICON := icon.ico									# Icon for the program executable
STRUCT := places_list.tsv save_structure.tsv room_coordinates.tsv	# Files to dinamically build the user interface
DEPENDENCIES := gtk3 structure assets icon.o $(OBJECTS)				# Files and foldes necessary for compiling the program
CFLAGS := $(shell pkg-config --cflags gtk+-3.0) $(shell pkg-config --cflags --libs gtk+-3.0) -Iincludes -fdiagnostics-color=always
# Note: CFLAGS runs two shell commands to get the flags to pass to the compiler, they return the folders of the GTK 3 headers and libraries.

.PHONY: release debug clean		# Target parameters that 'make' can be run with on the tarminal

# Link together the compiled objects of the release build
release: TARGET = release	# Subfolder where the release build will go
release: CFLAGS += -O2		# Activate compiler optimizations
release: $(DEPENDENCIES)
	@echo Linking release build...
	gcc $(OBJECTS) icon.o -o "$(DIRECTORY)\$(TARGET)\bin\$(NAME).exe" $(CFLAGS) -mwindows
	@echo Release build saved to the folder: $(DIRECTORY)\$(TARGET)\ 

# Link together the compiled objects of the release build
debug: TARGET = debug			# Subfolder where the debug build will go
debug: CFLAGS += -g3 -D_DEBUG	# Add debug symbols to the executable and define the '_DEBUG' macro
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

# Remove the temporary files created during compilation
clean:
	del *.o
	del includes\*.o
	del icon.rc