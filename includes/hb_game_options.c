#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <hb_game_options.h>

// Full file system path to the game options file
char OPTIONS_PATH[PATH_BUFFER] = {0};

/* Store each of the game options as a C-style string

Indices:
    [0]  Global audio volume (float from 0.00 to 1.00)
    [1]  Keyboard move Up (printable character)
    [2]  Keyboard move Left key (printable character)
    [3]  Keyboard move Down (printable character)
    [4]  Keyboard move Right (printable character)
    [5]  Accept key (printable character or controller button)
    [6]  Cancel key (printable character or controller button)
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
static char gamepad_buttons[4][6] = {
    "32769",    // A or ×
    "32770",    // B or ○
    "32771",    // X or □
    "32772",    // Y or △
};

// Default game options
static char *default_options[OPTIONS_COUNT] = {
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
    static options_init = false;
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

    /* TO DO: Parse the values from the file */
}

// Save the game's options file
void hb_save_game_options()
{

}