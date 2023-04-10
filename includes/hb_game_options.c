#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <hb_game_options.h>

// Full file system path to the game options file
char OPTIONS_PATH[PATH_BUFFER] = {0};

/* Store each of the game options as a C-style string

Indices:
    [0]  Global audio volume (float from 0.00 to 1.00)
    [1]  Keyboard move Up (digit or uppercase A-Z letter)
    [2]  Keyboard move Left key (digit or uppercase A-Z letter)
    [3]  Keyboard move Down (digit or uppercase A-Z letter)
    [4]  Keyboard move Right (digit or uppercase A-Z letter)
    [5]  Accept key (digit or uppercase A-Z letter)
    [6]  Cancel key (digit or uppercase A-Z letter)
    [7]  Gamepad button 1 ('accept' function)
    [8]  Gamepad button 2 ('cancel' function)
    [9]  Gamepad button 3 (used on minigames, function varies)
    [10] Gamepad button 4 (used on minigames, function varies)
    [11] Controller type (0 = Xbox, 1 = PlayStation)
    [12] Fullscreen is active (0 = false,  1 = true)
*/
char hb_game_options[OPTIONS_COUNT][OPTIONS_BUFFER] = {0};

// Map the four controller buttons to their respective ID
// Note: values obtained from data mining the game
static const char gamepad_buttons[4][6] = {
    "32769",    // A or ×
    "32770",    // B or ○
    "32771",    // X or □
    "32772",    // Y or △
};

// Default game options
static const char *default_options[OPTIONS_COUNT] = {
    "0.5",
    "W", "A", "S", "D", "Z", "X",
    "32769", "32770", "32771", "32772",
    "0", "0",
};

// Read the game's options file
// Note: this function should be called after the save was opened,
//       because that sets the variable SAVE_ROOT (path to the save's folder).
void hb_read_game_options()
{
    // Initialize the game options' variables when running the function for the first time
    static bool options_init = false;
    if (!options_init)
    {
        // Parse the path to the options file
        #ifdef _WIN32
        snprintf(OPTIONS_PATH, sizeof(OPTIONS_PATH), "%s\\%s", SAVE_ROOT, OPTIONS_FNAME);
        #else
        snprintf(OPTIONS_PATH, sizeof(OPTIONS_PATH), "%s/%s", SAVE_ROOT, OPTIONS_FNAME);
        #endif

        // Set the default values for the game options
        for (size_t i = 0; i < OPTIONS_COUNT; i++)
        {
            snprintf(hb_game_options[i], OPTIONS_BUFFER, "%s", default_options[i]);
        }

        options_init = true;
    }

    // Path were to save the options file
    char *my_path;
    char path_alt[PATH_BUFFER]; // Buffer for the case the options file needs to be saved to a non-default location
    
    if (strncmp(CURRENT_FILE, SAVE_PATH, PATH_BUFFER) == 0)
    {
        // If saving to the default save location,
        // the options file will be saved to the default options location
        my_path = OPTIONS_PATH;
    }
    else
    {
        // If not saving to the default save file location,
        // the options file will be saved to a new file (different from the save file)
        snprintf(path_alt, PATH_BUFFER, "%s-options", CURRENT_FILE);
        my_path = path_alt;
    }

    // Parse the values from the game options file
    
    FILE *options_file = fopen(my_path, "rt");
    if (!options_file) return;  // The default option values will be used if opening the file fails
    
    char line_buffer[OPTIONS_BUFFER+1] = {0};   // Buffer for reading the lines of the options file
    size_t pos = 0; // Position on the line buffer

    // Volume's value
    {
        char *const status = fgets(line_buffer, OPTIONS_BUFFER, options_file);
        if (!status) {fclose(options_file); return;}

        // Validate the volume's value
        bool is_valid = false;
        size_t dot_count = 0;
        while (line_buffer[pos] != '\0')
        {
            const char my_char = line_buffer[pos++];
            
            if (my_char == '\n')
            {
                // If we arrived at the end of the line without invalid characters
                is_valid = true;
                line_buffer[pos] = '\0';    // Remove the newline character
                break;
            }

            // Each character can only be a digit or a dot (being one dot at most)
            if (!isdigit(my_char) && my_char != '.') break;
            if (my_char == '.') dot_count++;
            if (dot_count > 1) break;
        }

        // Store the value if valid
        if (is_valid)
        {
            snprintf(hb_game_options[0], OPTIONS_BUFFER, "%s", line_buffer);
        }
    }

    // Keyboard input
    for (size_t i = 0; i < 6; i++)
    {
        char *const status = fgets(line_buffer, OPTIONS_BUFFER, options_file);
        if (!status) {fclose(options_file); return;}

        // Validate the input's value
        // The line can only have one character, and it has to be a digit or letter
        if (
            (isalpha(line_buffer[0]) || isdigit(line_buffer[0]))
            &&
            (line_buffer[1] == '\n')
        )
        {
            // Store the character, converting it to uppercase if needed.
            snprintf(hb_game_options[1+i], OPTIONS_BUFFER, "%c", (char)toupper(line_buffer[0]));
        }
    }

    // Gamepad input
    for (size_t i = 0; i < 4; i++)
    {
        char *const status = fgets(line_buffer, OPTIONS_BUFFER, options_file);
        if (!status) {fclose(options_file); return;}

        // Validate the input's value
        // The line can only have digits
        bool is_valid = false;
        size_t pos = 0;
        while (line_buffer[pos] != '\0')
        {
            const char my_char = line_buffer[pos++];
            if (my_char == '\n')
            {
                // If we arrived at the end of the line without invalid characters
                is_valid = true;
                line_buffer[pos] = '\0';    // Remove the newline character
                break;
            }

            // Each character can only be a digit
            if (!isdigit(my_char)) break;
        }

        // Store the value if valid
        if (is_valid)
        {
            snprintf(hb_game_options[7+i], OPTIONS_BUFFER, "%s", line_buffer);
        }
    }
    
    /* TO DO: other values... */

    fclose(options_file);
}

// Save the game's options file
void hb_save_game_options()
{

}