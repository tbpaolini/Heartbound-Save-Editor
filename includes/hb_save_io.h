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

extern char SAVE_PATH[PATH_BUFFER];    // Absolute path to the save file
extern char SAVE_ROOT[PATH_BUFFER];    // Absolute path to the folder where the save file is

// Store the location of the save file on the SAVE_PATH variable
int hb_find_save();

// Get the contents of the save file and store them on memory
int hb_read_save();

// Save the contents back to the save file
int hb_write_save();

#endif