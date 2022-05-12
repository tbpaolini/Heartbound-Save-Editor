# 'main.c' file and all '*.c' files on the 'includes' folder
SOURCE := main.c $(wildcard includes/*.c)

# The above files but with a '.o' extension
OBJECTS := $(addsuffix .o,$(basename $(SOURCE)))

# Name of the main executable
NAME := heartbound-save-editor

# Folder where the build will go
DIRECTORY := build/linux

# Files to dynamically build the user interface
STRUCT := places_list.tsv save_structure.tsv room_coordinates.tsv

# Files and folders necessary for compiling the program
DEPENDENCIES := structure assets $(OBJECTS)

# Flags to pass to the compiler
CFLAGS := $(shell pkg-config --cflags --libs gtk+-3.0) -lm -Iincludes -fdiagnostics-color=always
# Note: Here we run two shell commands to get the flags to pass to the compiler, they return the folders of the GTK 3 headers and libraries.

# Target parameters that 'make' can be run with on the terminal
.PHONY: release debug tar clean analyze

# Creation of a Debian package for the program
define deb-package
	cp -ru packaging/DEBIAN/ $(DIRECTORY)/$(TARGET)
	mkdir -p $(DIRECTORY)/$(TARGET)/usr/share/applications/
	cp -u packaging/shortcut-$(TARGET).desktop $(DIRECTORY)/$(TARGET)/usr/share/applications/$(NAME).desktop
	-dpkg-deb --build $(DIRECTORY)/$(TARGET) $(DIRECTORY)/$(NAME)_$(TARGET).deb
	rm -r $(DIRECTORY)/$(TARGET)/DEBIAN
	cp packaging/run.sh "$(DIRECTORY)/$(TARGET)/Run Heartbound Save Editor.sh"
	chmod +x "$(DIRECTORY)/$(TARGET)/Run Heartbound Save Editor.sh"
endef

# Subfolder where the release build will go
release: TARGET = release
# Activate compiler optimizations
release: CFLAGS += -O2
# Link together the compiled objects of the release build
release: $(DEPENDENCIES)
	@echo Linking release build...
	mkdir -p "$(DIRECTORY)/$(TARGET)/usr/bin/"
	gcc $(OBJECTS) -o "$(DIRECTORY)/$(TARGET)/usr/bin/$(NAME)" $(CFLAGS)
	$(deb-package)
	@echo Release build saved to the folder: $(DIRECTORY)/$(TARGET)/ 

# Subfolder where the debug build will go
debug: TARGET = debug
# Add debug symbols to the executable and define the '_DEBUG' macro
debug: CFLAGS += -g2 -D_DEBUG
# Link together the compiled objects of the debug build
debug: $(DEPENDENCIES)
	@echo Linking debug build...
	mkdir -p "$(DIRECTORY)/$(TARGET)/usr/bin/"
	gcc $(OBJECTS) -o "$(DIRECTORY)/$(TARGET)/usr/bin/$(NAME)" $(CFLAGS)
	$(deb-package)
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
	mkdir -p "$(DIRECTORY)/$(TARGET)/usr/lib/$(NAME)/"
	mkdir -p "$(DIRECTORY)/$(TARGET)/usr/share/pixmaps/"
	cp -u  "assets/icon.png" "$(DIRECTORY)/$(TARGET)/usr/share/pixmaps/$(NAME).png"
	cp -u  "assets/icon.ico" "$(DIRECTORY)/$(TARGET)/usr/share/pixmaps/$(NAME).png"
	cp -ru  "assets/textures" "$(DIRECTORY)/$(TARGET)/usr/lib/$(NAME)/textures/"
	
# Copy the structure files to the build destination
structure:
	mkdir -p "$(DIRECTORY)/$(TARGET)/usr/lib/$(NAME)/structure/"
	$(foreach file, $(STRUCT), cp -ru "structure/$(file)" "$(DIRECTORY)/$(TARGET)/usr/lib/$(NAME)/structure/" &)

# Create a TAR file
tar: TARGET = release
tar:
	mv $(DIRECTORY)/$(TARGET) $(DIRECTORY)/$(NAME)
	-cp -v packaging/tar-readme.txt $(DIRECTORY)/$(NAME)/readme.txt
	-tar --directory=$(DIRECTORY) -cvzf $(DIRECTORY)/$(NAME).tar.xz $(NAME)
	-rm -v $(DIRECTORY)/$(NAME)/readme.txt
	mv $(DIRECTORY)/$(NAME) $(DIRECTORY)/$(TARGET)

# Perform some static code analysis
analyze:
	@gcc $(SOURCE) $(CFLAGS) -fanalyzer
	@gcc $(SOURCE) $(CFLAGS) -fanalyzer -fanalyzer-checker=taint
	@rm a.out

# Remove the temporary files created during compilation
clean:
	rm *.o
	rm includes/*.o