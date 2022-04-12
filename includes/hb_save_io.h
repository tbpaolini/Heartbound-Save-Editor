/* Reading and writting of the Heartbound save file */

#ifndef _HB_SAVE_IO
#define _HB_SAVE_IO

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <hb_save_struct.h>

// Save file properties
#define LOCAL_APP_DATA getenv("LocalAppData")   // Path to the local application data
#define SAVE_FOLDER "Heartbound"                // Save folder on the local application data
#define SAVE_FNAME "heartbound_save8.thor"      // Name of the save file
#define PATH_BUFFER 512                         // Maximum number of characters of the absolute file path
#define SAVE_LINE_BUFFER 100                    // Maximum number of characters on each line of the save file

// Status codes for when the file is loaded or saved
#define SAVE_FILE_IS_VALID 0
#define SAVE_FILE_NOT_VALID -1
#define FILE_SAVING_SUCCESS 0
#define FILE_SAVING_FAILURE -1

// File system paths for the save file
extern char SAVE_PATH[PATH_BUFFER];    // Absolute path to the save file
extern char SAVE_ROOT[PATH_BUFFER];    // Absolute path to the folder where the save file is
extern char CURRENT_FILE[PATH_BUFFER]; // Absolute path to the save file that is currently opened

// Whether a save file is currently open
extern bool hb_save_is_open;

// Store the location of the save file on the SAVE_PATH variable
int hb_find_save();

// Get the contents of the save file and store them on memory
int hb_read_save(char *path);

// Read until the end of the line of a file stream, and store the read data on the 'destination' buffer.
// (newline characters not included on the output)
// Note: Unicode characters are not supported in the file's contents. But that should be fine, since the
//       save file never includes any.
gssize hb_read_line(GInputStream *save_file, char *destination, size_t max_size);

// Validate if a file is a valid Heartbound save
int hb_validate_save(GInputStream *save_file);

// Save the contents back to the save file
int hb_write_save();

#endif