# 'main.c' file and all '*.c' files on the 'includes' folder
SOURCE := main.c $(wildcard includes/*.c)

# The above files but with a '.o' extension
OBJECTS := $(addsuffix .o,$(basename $(SOURCE)))

# Name of the final executables
NAME := Heartbound Save Editor

# Folder where the build will go
DIRECTORY := build/linux

# Files to dynamically build the user interface
STRUCT := places_list.tsv save_structure.tsv room_coordinates.tsv

# Files and folders necessary for compiling the program
DEPENDENCIES := structure assets $(OBJECTS)

# Flags to pass to the compiler
CFLAGS := $(shell pkg-config --cflags gtk+-3.0) $(shell pkg-config --cflags --libs gtk+-3.0) -Iincludes -fdiagnostics-color=always
# Note: Here we run two shell commands to get the flags to pass to the compiler, they return the folders of the GTK 3 headers and libraries.

# Target parameters that 'make' can be run with on the terminal
.PHONY: release debug clean analyze

# Subfolder where the release build will go
release: TARGET = release
# Activate compiler optimizations
release: CFLAGS += -O2
# Link together the compiled objects of the release build
release: $(DEPENDENCIES)
	@echo Linking release build...
	mkdir -p "$(DIRECTORY)/$(TARGET)/bin/"
	gcc $(OBJECTS) -o "$(DIRECTORY)/$(TARGET)/bin/$(NAME)" $(CFLAGS)
	@echo Release build saved to the folder: $(DIRECTORY)/$(TARGET)/ 

# Subfolder where the debug build will go
debug: TARGET = debug
# Add debug symbols to the executable and define the '_DEBUG' macro
debug: CFLAGS += -g2 -Og -D_DEBUG
# Link together the compiled objects of the debug build
debug: $(DEPENDENCIES)
	@echo Linking debug build...
	mkdir -p "$(DIRECTORY)/$(TARGET)/bin/"
	gcc $(OBJECTS) -o "$(DIRECTORY)/$(TARGET)/bin/$(NAME).exe" $(CFLAGS)
	@echo Debug build saved to the folder: $(DIRECTORY)/$(TARGET)/ 

# Compile 'main.c'
main.o: main.c
	@echo Compiling $(TARGET) build...
	gcc -c $< -o $@ $(CFLAGS)

# Compile the '*.c' files on the 'includes' folder
includes/%.o: includes/%.c
	gcc -c $< -o $@ $(CFLAGS)

# Copy the program's images to the build destination
assets:
	mkdir -p "$(DIRECTORY)/$(TARGET)/lib/"
	cp -u  "assets/icon.png" "$(DIRECTORY)/$(TARGET)/lib/"
	cp -u  "assets/icon.ico" "$(DIRECTORY)/$(TARGET)/lib/"
	cp -ru  "assets/textures" "$(DIRECTORY)/$(TARGET)/lib/textures/"
	
# Copy the structure files to the build destination
structure:
	mkdir -p "$(DIRECTORY)/$(TARGET)/lib/structure/"
	$(foreach file, $(STRUCT), cp -ru "structure/$(file)" "$(DIRECTORY)/$(TARGET)/lib/structure/" &)

# Perform some static code analysis
analyze:
	@gcc $(SOURCE) $(CFLAGS) -fanalyzer
	@gcc $(SOURCE) $(CFLAGS) -fanalyzer -fanalyzer-checker=taint
	@del a.exe

# Remove the temporary files created during compilation
clean:
	rm *.o
	rm includes/*.o