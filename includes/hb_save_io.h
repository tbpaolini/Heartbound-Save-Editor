/* Reading and writting of the Heartbound save file */

#ifndef _HB_SAVE_IO
#define _HB_SAVE_IO

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hb_save_struct.h>

#define LOCAL_APP_DATA getenv("LocalAppData")   // Path to the local application data
#define SAVE_FOLDER "Heartbound"                // Save folder on the local application data
#define SAVE_FNAME "heartbound_save8.thor"      // Name of the save file
#define PATH_BUFFER 512                         // Maximum number of characters of the absolute file path
#define SAVE_LINE_BUFFER 50                     // Maximum number of characters on each line of the save file

#define SAVE_FILE_IS_VALID 0
#define SAVE_FILE_NOT_VALID -1

extern char SAVE_PATH[PATH_BUFFER];    // Absolute path to the save file
extern char SAVE_ROOT[PATH_BUFFER];    // Absolute path to the folder where the save file is
extern char CURRENT_FILE[PATH_BUFFER]; // Absolute path to the save file that is currently opened

// Store the location of the save file on the SAVE_PATH variable
int hb_find_save();

// Get the contents of the save file and store them on memory
int hb_read_save(char *path);

// Validate if a file is a valid Heartbound save
int hb_validate_save(FILE *save_file);

// Save the contents back to the save file
int hb_write_save();

#endif