/* Reading and writting of the Heartbound save file */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
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

// Whether a save file is currently open
bool hb_save_is_open = false;

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
    // Open the file
    FILE *save_file = fopen( (path != NULL ? path : SAVE_PATH), "r" );
    if (save_file == NULL) return SAVE_FILE_NOT_VALID;
    
    // Check if the file is valid
    int status = hb_validate_save(save_file);
    if (status != SAVE_FILE_IS_VALID)
    {
        fclose(save_file);
        return status;
    }

    // Buffer for reading the file's lines
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

    hb_save_is_open = true;
    return SAVE_FILE_IS_VALID;
}

// Validate if a file is a valid Heartbound save
int hb_validate_save(FILE *save_file)
{
    // Remember the current read position on the file,
    // so the position can be reset to it when the function returns.
    fpos_t my_position;
    fgetpos(save_file, &my_position);
    rewind(save_file);

    // Keep track of the characters and the line count
    char current_character;
    size_t line_count = 0;      // How many lines of text the file has
    bool line_is_empty = true;  // If the line has not any characters (besides spaces or newlines)
    size_t target_line_count = NUM_STORY_VARS + ROW_OFFSET - 1;  // How many lines the file should have
    /* Note:
        The default save file has 1006 lines: 7 player attributes followed by 999 storyline variables.
        Technically there are 1000 storyline variables, however the DEBUG VARIABLE (at index 0) is not
        included on the save file.
    */

    // Loop through all characters in the file
    while (!feof(save_file))
    {
        current_character = fgetc(save_file);
        
        if (current_character == EOF) break;     // End of file
        if (current_character == ' ') continue;  // Blank space
        
        if (current_character == '\n')  // Line break
        {
            // Increase the line count
            line_count++;

            // Check if the file went over the expected number of lines,
            // and if the line has at least one character.
            if (line_count > target_line_count || line_is_empty)
            {
                fsetpos(save_file, &my_position);
                return SAVE_FILE_NOT_VALID;
            }

            // Move to the next line
            line_is_empty = true;
            continue;
        }

        if (line_count != 1)
        {
            // On all lines, except for the second, the characters can only be decimal digits
            if ( !isdigit(current_character) )
            {
                fsetpos(save_file, &my_position);
                return SAVE_FILE_NOT_VALID;
            }
        }
        else
        {
            // We are on the second line, which has the room's name.
            // The characters must be decimal digits, English letters, or underscores.
            if ( !isalnum(current_character) && current_character != '_' )
            {
                fsetpos(save_file, &my_position);
                return SAVE_FILE_NOT_VALID;
            }
        }

        // When the character is not a blank space or a newline,
        // the line is flagged as being 'not empty'
        line_is_empty = false;
    }
    
    // Check if the file has the expected number of lines
    // (The program forgives a lack of newline at the end of the file)
    if (line_count == target_line_count || line_count == target_line_count-1)
    {
        // If the program has the expected number of lines,
        // and all characters have passed the test,
        // then return that the save file is valid.
        fsetpos(save_file, &my_position);
        return SAVE_FILE_IS_VALID;
    }
    else
    {
        // The file is invalid if it has less lines than expected
        fsetpos(save_file, &my_position);
        return SAVE_FILE_NOT_VALID;
    }
}

// Save the contents back to the save file
int hb_write_save()
{
    // Open save file for writting
    FILE *save_file = fopen(CURRENT_FILE, "w");

    // Return if the file could not be opened for writing
    if (save_file == NULL) return FILE_SAVING_FAILURE;

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
    return FILE_SAVING_SUCCESS;
    // TO DO: Backup of the original save file
}