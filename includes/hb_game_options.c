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

// Read the game's options file
void hb_read_game_options()
{
    
}

// Save the game's options file
void hb_save_game_options()
{

}