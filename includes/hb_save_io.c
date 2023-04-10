/* Reading and writting of the Heartbound save file */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <hb_save_io.h>
#include <hb_game_options.h>
#include "../config.h"

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

// Whether the currently open file has been changed by another program
gint64 hb_save_modification_time = 0;

// Store the location of the save file on the SAVE_PATH variable
int hb_find_save()
{
    // Whether the editor opens the save location used by Steam's Proton compatibility engine
    // Note: That engine runs the Windows version of the game on Linux, inside an emulator.
    //       However the save location is different than the native Linux version.
    bool proton_compatibility = false;

    // The Heartbound's manifest file on Steam
    // (it is where is the compatibility mode setting)
    char heartbound_appmanifest_path[PATH_BUFFER];

    // Build the path to the manifest file
    snprintf(
        heartbound_appmanifest_path,
        PATH_BUFFER,
        "%s/%s/%s",
        LOCAL_APP_DATA,
        ".local/share/Steam/steamapps",
        "appmanifest_567380.acf"
    );

    // Open the manifest file for reading
    FILE *appmanifest = fopen(heartbound_appmanifest_path, "r");
    
    if (appmanifest != NULL)
    {
        // Get the size of the file
        fseek(appmanifest, 0, SEEK_END);
        long file_size = ftell(appmanifest);
        rewind(appmanifest);

        // Sanity check for not reading a huge file
        // The actual file is less than 1 kB, our size limit is much higher (10 MB)
        if (file_size < 10000000)
        {
            // Read the contents of the file into memory
            char *appmanifest_buffer = calloc(file_size + 1, sizeof(char));
            if (appmanifest_buffer != NULL)
            {
                // Read the file, then close it
                size_t read_count = fread(appmanifest_buffer, file_size, 1, appmanifest);
                fclose(appmanifest);
                appmanifest = NULL;

                // Check if the compatibility setting is present and enabled
                if (read_count > 0 && strstr(appmanifest_buffer, "\"platform_override_source\"\t\t\"windows\"") != NULL)
                {
                    // If yes, then flag it as enabled
                    proton_compatibility = true;
                }

                // Remove the file contents from memory
                free(appmanifest_buffer);
            }
        }

        // Close the manifest file
        if (appmanifest != NULL) fclose(appmanifest);
    }

    // Which save file location to use (Proton's or Linux native's)
    char *my_save_folder = proton_compatibility ? PROTON_FOLDER : SAVE_FOLDER;

    // Build the path to the save file
    snprintf(SAVE_PATH, sizeof(SAVE_PATH), "%s/%s/%s", LOCAL_APP_DATA, my_save_folder, SAVE_FNAME);
    snprintf(SAVE_ROOT, sizeof(SAVE_ROOT), "%s/%s", LOCAL_APP_DATA, my_save_folder);
    
    return 0;
    // TO DO: Error handling
}

// Get the contents of the save file and store them on memory
int hb_read_save(char *path)
{
    // Open the file
    int chdir_status;
    GFile *save_file = g_file_new_for_path( (path != NULL ? path : SAVE_PATH) );

    /* Note:
        We are switching between directories in order to take the loader into consideration,
        because it is on the directory above of the main executable. Relative paths opened
        through the loader are relative to the loader itself.
    */
    
    GInputStream *input = G_INPUT_STREAM(g_file_read(save_file, NULL, NULL));
    if (input == NULL)
    {
        g_object_unref(save_file);
        return SAVE_FILE_NOT_VALID;
    }

    /* Note:
        We are not using the standard "fopen()" function because it does not
        support the Unicode characters that originate from the GTK functions.
        So we are using the GIO functions that come from the GTK headers.
        This way a file with non-English characters on its path is displayed
        and works properly.
    */
    
    // Check if the file is valid
    int status = hb_validate_save(input);
    if (status != SAVE_FILE_IS_VALID)
    {
        g_object_unref(save_file);
        return status;
    }

    // Buffer for reading the file's lines
    char *restrict line = malloc(SAVE_LINE_BUFFER * sizeof(char));

    // Parse the player's attributes
    
    // Game seed
    hb_read_line(input, line, SAVE_LINE_BUFFER);
    snprintf(hb_game_seed, sizeof(hb_game_seed), "%s", line);     // Store the seed as a string

    // Current room
    hb_read_line(input, line, SAVE_LINE_BUFFER);
    snprintf(hb_room_id, sizeof(hb_room_id), "%s", line);     // Store the room name as a string

    // Coordinates on the room
    hb_read_line(input, line, SAVE_LINE_BUFFER);
    hb_x_axis = atof(line);
    hb_read_line(input, line, SAVE_LINE_BUFFER);
    hb_y_axis = atof(line);

    // Hit points
    hb_read_line(input, line, SAVE_LINE_BUFFER);
    hb_hitpoints_current = atof(line);
    hb_read_line(input, line, SAVE_LINE_BUFFER);
    hb_hitpoints_maximum = atof(line);

    // Known languages
    hb_read_line(input, line, SAVE_LINE_BUFFER);
    hb_known_glyphs = atof(line);

    // Parse the storyline variables
    size_t var = 1;
    gssize line_status = 1;
    while (true)
    {
        line_status = hb_read_line(input, line, SAVE_LINE_BUFFER);   // Read the storyline variable from the save file
        if (line_status <= 0) break;    // Stop if we have arrived at the end of the file
        
        hb_save_data[var++].value = atof(line);        // Store the current value and move to the next variable
        if (var >= NUM_STORY_VARS) break;           // Stop if we have arrived to the end of the storyline array
    }

    free(line);         // Delete the line buffer from memory

    // Close the save file
    g_input_stream_close(input, NULL, NULL);
    g_object_unref(input);
    g_object_unref(save_file);

    // Store the current open file
    // Note: On Linux, the first character of the string got replaced by a null terminator,
    //       if the source and destination strings pointed to the same adress on memory.
    //       Windows just handled it better by keeping the the text the same.
    if (path != CURRENT_FILE)
    {
        snprintf(
            CURRENT_FILE,
            sizeof(CURRENT_FILE),
            "%s",
            (path != NULL ? path : SAVE_PATH)
        );
    }

    hb_save_get_modified_time();
    hb_save_is_open = true;
    hb_read_game_options();
    return SAVE_FILE_IS_VALID;
}

// Validate if a file is a valid Heartbound save
int hb_validate_save(GInputStream *save_file)
{
    // Remember the current read position on the file,
    // so the position can be reset to it when the function returns.
    goffset my_position;
    my_position = g_seekable_tell(G_SEEKABLE(save_file));

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
    gssize status;
    while (true)
    {
        status = g_input_stream_read(save_file, &current_character, sizeof(char), NULL, NULL);
        
        if (status <= 0) break;                  // End of file
        if (current_character == '\r') continue; // Carriage return
        if (current_character == ' ') continue;  // Blank space
        
        if (current_character == '\n')  // Line break
        {
            // Increase the line count
            line_count++;

            // Check if the file went over the expected number of lines,
            // and if the line has at least one character.
            if (line_count > target_line_count || line_is_empty)
            {
                g_seekable_seek(G_SEEKABLE(save_file), my_position, G_SEEK_SET, NULL, NULL);
                return SAVE_FILE_NOT_VALID;
            }

            // Move to the next line
            line_is_empty = true;
            continue;
        }

        switch (line_count)
        {
            case 1:
                // We are on the second line, which has the room's name.
                // The characters must be decimal digits, English letters, or underscores.
                if ( !isalnum(current_character) && current_character != '_' )
                {
                    g_seekable_seek(G_SEEKABLE(save_file), my_position, G_SEEK_SET, NULL, NULL);
                    return SAVE_FILE_NOT_VALID;
                }
                break;
            
            case 2:
            case 3:
                // Third and fourth lines, the X and Y coordinates
                // The characters must me decimal digits, or positive/negative signs, or a periods
                if ( !isdigit(current_character) && current_character != '+' && current_character != '-' && current_character != '.')
                {
                    g_seekable_seek(G_SEEKABLE(save_file), my_position, G_SEEK_SET, NULL, NULL);
                    return SAVE_FILE_NOT_VALID;
                }
                break;
            
            case 406:
            case 407:
            case 408:
            case 409:
            case 410:
            case 411:
            case 412:
            case 413:
                // Lines 407 to 414, the Turtle Farm's crops
                // Thanks to the ARG (Alternate Reality Game), these values can be set to very high numbers,
                // which would make the game to save them in scientific notation, for example 2.00913e+31
                if ( !isdigit(current_character) && current_character != '+' && current_character != 'e' && current_character != '.')
                {
                    g_seekable_seek(G_SEEKABLE(save_file), my_position, G_SEEK_SET, NULL, NULL);
                    return SAVE_FILE_NOT_VALID;
                }
                break;
            
            default:
                // On all lines, except for lines 2 to 4, the characters can only be decimal digits
                if ( !isdigit(current_character) )
                {
                    g_seekable_seek(G_SEEKABLE(save_file), my_position, G_SEEK_SET, NULL, NULL);
                    return SAVE_FILE_NOT_VALID;
                }
                break;
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
        g_seekable_seek(G_SEEKABLE(save_file), my_position, G_SEEK_SET, NULL, NULL);
        return SAVE_FILE_IS_VALID;
    }
    else
    {
        // The file is invalid if it has less lines than expected
        g_seekable_seek(G_SEEKABLE(save_file), my_position, G_SEEK_SET, NULL, NULL);
        return SAVE_FILE_NOT_VALID;
    }
}

// Read until the end of the line of a file stream, and store the read data on the 'destination' buffer.
// (newline characters not included on the output)
// Note: Unicode characters are not supported in the file's contents. But that should be fine, since the
//       save file never includes any.
gssize hb_read_line(GInputStream *save_file, char *destination, size_t max_size)
{
    #define READ_CHAR() g_input_stream_read(save_file, &current_character, sizeof(char), NULL, NULL)
    #define IS_NEWLINE() ( (current_character == '\n') || (current_character == '\r') )
    
    char current_character;

    // Read the first character
    gssize status = READ_CHAR();
    size_t pos = 0;

    // Keep reading until a non-control character is found
    while( iscntrl(current_character) )
    {
        status = READ_CHAR();
        if (status <= 0) return status;
    }

    // Store the first character
    destination[pos++] = current_character;

    // Keep reading while we did not reach the end of the file or the size limit
    while( (status > 0) && (pos < max_size - 1) )
    {
        status = READ_CHAR();       // Read the next character
        if ( IS_NEWLINE() ) break;  // Stop if we reached the end of the line
        
        if ( !iscntrl(current_character) )  // Store the character if it isn't a control character
        {
            destination[pos++] = current_character;
        }
    }

    destination[pos] = '\0';    // Add a null terminator to the string
    return status;

    #undef READ_CHAR
    #undef IS_NEWLINE
}

// Save the contents back to the save file
int hb_write_save()
{
    // Open save file for writting
    int chdir_status;
    GFile *save_file = g_file_new_for_path(CURRENT_FILE);
    
    /* Note:
        We are switching between directories in order to take the loader into consideration,
        because it is on the directory above of the main executable. Relative paths opened
        through the loader are relative to the loader itself.
    */
    
    GOutputStream *output = G_OUTPUT_STREAM(g_file_replace(save_file, NULL, FALSE, G_FILE_CREATE_NONE, NULL, NULL));
    /* Note:
        We are not using the standard "fopen()" function because it does not
        support the Unicode characters that originate from the GTK functions.
        So we are using the GIO functions that come from the GTK headers.
        This way a file with non-English characters on its path is displayed
        and works properly.
    */

    // Return if the file could not be opened for writing
    if (output == NULL)
    {
        g_object_unref(save_file);
        return FILE_SAVING_FAILURE;
    }

    char *restrict line_buffer = calloc(SAVE_LINE_BUFFER, sizeof(char));

    // Game seed
    snprintf(line_buffer, SAVE_LINE_BUFFER, "%s\n", hb_game_seed);
    g_output_stream_write(output, line_buffer, strlen(line_buffer), NULL, NULL);

    // Current room
    snprintf(line_buffer, SAVE_LINE_BUFFER, "%s\n", hb_room_id);
    g_output_stream_write(output, line_buffer, strlen(line_buffer), NULL, NULL);

    // Coordinates on the room
    snprintf(line_buffer, SAVE_LINE_BUFFER, "%.0f \n", hb_x_axis);
    g_output_stream_write(output, line_buffer, strlen(line_buffer), NULL, NULL);
    snprintf(line_buffer, SAVE_LINE_BUFFER, "%.0f \n", hb_y_axis);
    g_output_stream_write(output, line_buffer, strlen(line_buffer), NULL, NULL);

    // Hit points
    snprintf(line_buffer, SAVE_LINE_BUFFER, "%.0f \n", hb_hitpoints_current);
    g_output_stream_write(output, line_buffer, strlen(line_buffer), NULL, NULL);
    snprintf(line_buffer, SAVE_LINE_BUFFER, "%.0f \n", hb_hitpoints_maximum);
    g_output_stream_write(output, line_buffer, strlen(line_buffer), NULL, NULL);

    // Known languages
    snprintf(line_buffer, SAVE_LINE_BUFFER, "%.0f \n", hb_known_glyphs);
    g_output_stream_write(output, line_buffer, strlen(line_buffer), NULL, NULL);

    // Storyline variables
    for (size_t i = 1; i < NUM_STORY_VARS; i++)
    {
        snprintf(line_buffer, SAVE_LINE_BUFFER, "%.0f \n", hb_save_data[i].value);
        g_output_stream_write(output, line_buffer, strlen(line_buffer), NULL, NULL);
    }
    
    // Close the save file
    free(line_buffer);
    g_output_stream_close(output, NULL, NULL);
    g_object_unref(output);
    g_object_unref(save_file);
    
    hb_save_get_modified_time(); 
    return FILE_SAVING_SUCCESS;
    // TO DO: Backup of the original save file
}

// Store the file's last modification time on the variable 'hb_save_modification_time'
void hb_save_get_modified_time()
{
    // Open the file
    int chdir_status;
    GFile *save_file = g_file_new_for_path(CURRENT_FILE);

    GFileInfo *file_info = NULL;        // File's medatada
    GDateTime *modified_time = NULL;    // Last write time of the file

    // Get and store the file's last modified time
    file_info = g_file_query_info(save_file, G_FILE_ATTRIBUTE_TIME_MODIFIED, 0, NULL, NULL);
    if (file_info != NULL) modified_time = g_file_info_get_modification_date_time(file_info);
    if (modified_time != NULL) hb_save_modification_time = g_date_time_to_unix(modified_time);

    // Perform garbage collection and close the file
    if (modified_time != NULL) g_date_time_unref(modified_time);    // For some reason, 'GDateTime' requires its own unref function
    if (file_info != NULL) g_object_unref(file_info);
    g_object_unref(save_file);
}