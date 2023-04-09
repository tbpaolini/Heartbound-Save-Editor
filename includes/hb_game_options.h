#ifndef _HB_GAME_OPTIONS
#define _HB_GAME_OPTIONS

#include <hb_save_io.h>

#define OPTIONS_FNAME "options.thor"    // Name of the game options file
#define OPTIONS_COUNT 13                // Amount of options that the game store
#define OPTIONS_BUFFER 32               // Size (in bytes) to store the value of a game option

// Full file system path to the game options file
extern char OPTIONS_PATH[PATH_BUFFER];

// Store each of the game options as a C-style string
extern char hb_game_options[OPTIONS_COUNT][OPTIONS_BUFFER];

// Read the game's options file
void hb_read_game_options();

// Save the game's options file
void hb_save_game_options();

#endif // _HB_GAME_OPTIONS