#ifndef _HB_GAME_OPTIONS
#define _HB_GAME_OPTIONS

#include <hb_save_io.h>

#define OPTIONS_FNAME "options.thor"    // Name of the game options file
#define OPTIONS_COUNT 13                // Amount of options that the game store
#define OPTIONS_BUFFER 32               // Size (in bytes) to store the value of a game option

// Full file system path to the game options file
extern char OPTIONS_PATH[PATH_BUFFER];

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
extern char hb_game_options[OPTIONS_COUNT][OPTIONS_BUFFER];

// Read the game's options file
void hb_read_game_options();

// Save the game's options file
void hb_save_game_options();

#endif // _HB_GAME_OPTIONS