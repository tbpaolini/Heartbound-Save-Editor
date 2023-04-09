#include <hb_game_options.h>

// Full file system path to the game options file
char OPTIONS_PATH[PATH_BUFFER] = {0};

// Store each of the game options as a C-style string
char hb_game_options[OPTIONS_COUNT][OPTIONS_BUFFER] = {0};