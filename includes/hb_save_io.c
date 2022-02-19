/* Reading and writting of the Heartbound save file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOCAL_APP_DATA getenv("LocalAppData")   // Path to the local application data
#define SAVE_FOLDER "Heartbound"                // Save folder on the local application data
#define SAVE_FNAME "heartbound_save8.thor"      // Name of the save file
#define PATH_BUFFER 512                         // Maximum number of characters of the absolute file path
#define SAVE_LINE_BUFFER 50                     // Maximum number of characters on each line of the save file

extern char SAVE_PATH[PATH_BUFFER];    // Absolute path to the save file

// Store the location of the save file on the SAVE_PATH variable
int find_save()
{
    snprintf(SAVE_PATH, sizeof(SAVE_PATH), "%s\\%s\\%s", LOCAL_APP_DATA, SAVE_FOLDER, SAVE_FNAME);
    return 0;
    // TO DO: Error handling
}

// Get the contents of the save file and store them on memory
void read_save()
{
    find_save();
    FILE *save_file = fopen(SAVE_PATH, "r");
    char line[SAVE_LINE_BUFFER];
    while (!feof(save_file))
    {
        fgets(line, sizeof(line), save_file);
        /* Do stuff with each line... */
    }

    return 0;
    // TO DO: Error handling
}