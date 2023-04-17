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
extern char hb_game_options[OPTIONS_COUNT][OPTIONS_BUFFER];

// Read the game's options file
// Note: this function should be called after the save was opened,
//       because that sets the variable SAVE_ROOT (path to the save's folder).
void hb_read_game_options();

// Save the game's options file
void hb_save_game_options();

// Reset the game's options back to their default values
void hb_reset_game_options();

// Add to the user interface the fields corresponding to the game options
// Note: the function needs to receive the GTK container where the fields will be added on.
void hb_insert_options_fields(GtkWidget *container);

// Draw the changed game options fields to the interface
static inline void __update_options_interface();

// Change the displayed text of the gamepad buttons in order to match the selected gamepad type
// (the text will be toggled between "ABXY" and "×○□△")
static void __update_buttons_text(GtkToggleButton* widget, gpointer user_data);

// Update the values on the 'hb_game_options[]' array from the selections on the interface
static inline void __update_game_options();

// On Windows: Open a file that has unicode (UTF-8) characters in its path
#ifdef _WIN32
static inline FILE* __win_fopen(const char *path, const char *mode);
#endif // _WIN32

#endif // _HB_GAME_OPTIONS