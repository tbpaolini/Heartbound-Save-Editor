/* Reading and writting of the Heartbound save file */

#ifndef _HB_SAVE_IO
#define _HB_SAVE_IO

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hb_save_struct.c>

#define LOCAL_APP_DATA getenv("LocalAppData")   // Path to the local application data
#define SAVE_FOLDER "Heartbound"                // Save folder on the local application data
#define SAVE_FNAME "heartbound_save8.thor"      // Name of the save file
#define PATH_BUFFER 512                         // Maximum number of characters of the absolute file path
#define SAVE_LINE_BUFFER 50                     // Maximum number of characters on each line of the save file

char SAVE_PATH[PATH_BUFFER];    // Absolute path to the save file

// Store the location of the save file on the SAVE_PATH variable
int find_save()
{
    snprintf(SAVE_PATH, sizeof(SAVE_PATH), "%s\\%s\\%s", LOCAL_APP_DATA, SAVE_FOLDER, SAVE_FNAME);
    return 0;
    // TO DO: Error handling
}

// Get the contents of the save file and store them on memory
int read_save()
{
    FILE *save_file = fopen(SAVE_PATH, "r");
    char *restrict line = malloc(SAVE_LINE_BUFFER);

    // Parse the player's attributes
    
    // Game seed
    fgets(line, SAVE_LINE_BUFFER, save_file);
    snprintf(game_seed, sizeof(game_seed), "%s", line);     // Store the seed as a string
    game_seed[strlen(game_seed) - 1] = '\0';                // Remove the line break at the end

    // Current room
    fgets(line, SAVE_LINE_BUFFER, save_file);
    snprintf(room_id, sizeof(room_id), "%s", line);     // Store the room name as a string
    room_id[strlen(room_id) - 1] = '\0';                // Remove the line break at the end

    // Coordinates on the room
    fgets(line, SAVE_LINE_BUFFER, save_file);
    x_axis = atof(line);
    fgets(line, SAVE_LINE_BUFFER, save_file);
    y_axis = atof(line);

    // Hit points
    fgets(line, SAVE_LINE_BUFFER, save_file);
    hp_current = atof(line);
    fgets(line, SAVE_LINE_BUFFER, save_file);
    hp_maximum = atof(line);

    // Known languages
    fgets(line, SAVE_LINE_BUFFER, save_file);
    known_glyphs = atof(line);

    // Parse the storyline variables
    size_t var = 1;
    while (!feof(save_file))    // Stop if we have arrived at the end of the file
    {
        fgets(line, SAVE_LINE_BUFFER, save_file);   // Read the storyline variable from the save file
        save_data[var++].value = atof(line);        // Store the current value and move to the next variable
        if (var >= NUM_STORY_VARS) break;           // Stop if we have arrived to the end of the storyline array
    }

    free(line);         // Delete the line buffer from memory
    fclose(save_file);  // Close the save file
    return 0;
    // TO DO: Error handling
}

// Save the contents back to the save file
int write_save()
{
    // Open save file for writting
    FILE *save_file = fopen(SAVE_PATH, "w");

    // Game seed
    fprintf_s(save_file, "%s \n", game_seed);

    // Current room
    fprintf_s(save_file, "%s \n", room_id);

    // Coordinates on the room
    fprintf_s(save_file, "%.0f \n", x_axis);
    fprintf_s(save_file, "%.0f \n", y_axis);

    // Hit points
    fprintf_s(save_file, "%.0f \n", hp_current);
    fprintf_s(save_file, "%.0f \n", hp_maximum);

    // Known languages
    fprintf_s(save_file, "%.0f \n", known_glyphs);

    // Storyline variables
    for (size_t i = 1; i < NUM_STORY_VARS; i++)
    {
        fprintf_s(save_file, "%.0f \n", save_data[i].value);
    }
    
    // Close the save file
    fclose(save_file);
    return 0;
    // TO DO: Error handling and back up of the original save file
}

#endif