/* Reading and writting of the Heartbound save file */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hb_save_io.h>

char SAVE_PATH[PATH_BUFFER];
char SAVE_ROOT[PATH_BUFFER];
char CURRENT_FILE[PATH_BUFFER];

// Player attributes
char hb_game_seed[SEED_SIZE];                       // Game seed (10 decimal characters long)
char hb_room_id[ROOM_NAME_SIZE];                    // The ID (as a string) of the room the player is
double hb_x_axis, hb_y_axis;                        // Coordinates of the player in the room
double hb_hitpoints_current, hb_hitpoints_maximum;  // Current and maximum hit points of the player

// Which glyph sets the player know:
// 0 = None; 1 = Lightbringer; 2 = Lightbringer and Darksider
double hb_known_glyphs;

// Store the location of the save file on the SAVE_PATH variable
int hb_find_save()
{
    snprintf(SAVE_PATH, sizeof(SAVE_PATH), "%s\\%s\\%s", LOCAL_APP_DATA, SAVE_FOLDER, SAVE_FNAME);
    snprintf(SAVE_ROOT, sizeof(SAVE_ROOT), "%s\\%s", LOCAL_APP_DATA, SAVE_FOLDER);
    return 0;
    // TO DO: Error handling
}

// Get the contents of the save file and store them on memory
int hb_read_save(char *path)
{
    FILE *save_file = fopen( (path != NULL ? path : SAVE_PATH), "r" );
    char *restrict line = malloc(SAVE_LINE_BUFFER);

    // Parse the player's attributes
    
    // Game seed
    fgets(line, SAVE_LINE_BUFFER, save_file);
    snprintf(hb_game_seed, sizeof(hb_game_seed), "%s", line);     // Store the seed as a string
    hb_game_seed[strlen(hb_game_seed) - 1] = '\0';                // Remove the line break at the end

    // Current room
    fgets(line, SAVE_LINE_BUFFER, save_file);
    snprintf(hb_room_id, sizeof(hb_room_id), "%s", line);     // Store the room name as a string
    hb_room_id[strlen(hb_room_id) - 1] = '\0';                // Remove the line break at the end

    // Coordinates on the room
    fgets(line, SAVE_LINE_BUFFER, save_file);
    hb_x_axis = atof(line);
    fgets(line, SAVE_LINE_BUFFER, save_file);
    hb_y_axis = atof(line);

    // Hit points
    fgets(line, SAVE_LINE_BUFFER, save_file);
    hb_hitpoints_current = atof(line);
    fgets(line, SAVE_LINE_BUFFER, save_file);
    hb_hitpoints_maximum = atof(line);

    // Known languages
    fgets(line, SAVE_LINE_BUFFER, save_file);
    hb_known_glyphs = atof(line);

    // Parse the storyline variables
    size_t var = 1;
    while (!feof(save_file))    // Stop if we have arrived at the end of the file
    {
        fgets(line, SAVE_LINE_BUFFER, save_file);   // Read the storyline variable from the save file
        hb_save_data[var++].value = atof(line);        // Store the current value and move to the next variable
        if (var >= NUM_STORY_VARS) break;           // Stop if we have arrived to the end of the storyline array
    }

    free(line);         // Delete the line buffer from memory
    fclose(save_file);  // Close the save file

    // Store the current open file
    snprintf(
        CURRENT_FILE,
        sizeof(CURRENT_FILE),
        (path != NULL ? path : SAVE_PATH)
    );

    return 0;
    // TO DO: Error handling
}

// Save the contents back to the save file
int hb_write_save()
{
    // Open save file for writting
    FILE *save_file = fopen(SAVE_PATH, "w");

    // Game seed
    fprintf_s(save_file, "%s\n", hb_game_seed);

    // Current room
    fprintf_s(save_file, "%s\n", hb_room_id);

    // Coordinates on the room
    fprintf_s(save_file, "%.0f \n", hb_x_axis);
    fprintf_s(save_file, "%.0f \n", hb_y_axis);

    // Hit points
    fprintf_s(save_file, "%.0f \n", hb_hitpoints_current);
    fprintf_s(save_file, "%.0f \n", hb_hitpoints_maximum);

    // Known languages
    fprintf_s(save_file, "%.0f \n", hb_known_glyphs);

    // Storyline variables
    for (size_t i = 1; i < NUM_STORY_VARS; i++)
    {
        fprintf_s(save_file, "%.0f \n", hb_save_data[i].value);
    }
    
    // Close the save file
    fclose(save_file);
    return 0;
    // TO DO: Error handling and back up of the original save file
}