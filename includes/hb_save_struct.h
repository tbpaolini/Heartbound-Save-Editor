/* Template and data structure for the saved data of Heartbound */

#ifndef _HB_SAVE_STRUCT
#define _HB_SAVE_STRUCT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define SAVE_STRUCT_LOC "..\\lib\\structure\\save_structure.tsv"   // Path to the file with the save structure
#define SEED_SIZE (size_t)11                        // Amount of characters (plus one) of the game seed string
#define ROOM_NAME_SIZE (size_t) 50                  // Maximum amount of characters (plus one) for the room name string
#define SAVE_STRUCT_BUFFER (size_t)500              // Buffer size (in bytes) for each line of the structure file
#define NUM_STORY_VARS (size_t)1000                 // Amount of storyline variables in the save file
#define COLUMN_OFFSET (size_t)6                     // Amount of columns until the storyline variable values
#define ROW_OFFSET (size_t)7                        // Amount of rows before the first storyline variable

// Player attributes
extern char hb_game_seed[SEED_SIZE];                       // Game seed (10 decimal characters long)
extern char hb_room_id[ROOM_NAME_SIZE];                    // The ID (as a string) of the room the player is
extern double hb_x_axis, hb_y_axis;                        // Coordinates of the player in the room
extern double hb_hitpoints_current, hb_hitpoints_maximum;  // Current and maximum hit points of the player

// Which glyph sets the player know:
// 0 = None; 1 = Lightbringer; 2 = Lightbringer and Darksider
extern double hb_known_glyphs;

// Alias for the variable's values
// (associate the variable value with its meaning)
typedef struct ValueAlias
{
    char **header;      // Pointer to the row's name
    char *description;  // What the value does in-game (as a string)
} ValueAlias;

// The storyline variables
typedef struct StorylineVars
{
    double value;                // Value of this entry on the save file
    char *location;              // In-game location that this entry applies to
    char *name;                  // Description of the in-game feature this entry refers to
    char *info;                  // Further description of the feature
    char *unit;                  // Measurement unit of the value this entry represents
    size_t num_entries;          // Amount of different values that the field accept (0 if it accepts any value)
    ValueAlias *aliases;         // Associate each numeric value to its meaning (as strings)
    bool used;                   // Wheter the variable has data on it
} StorylineVars;

// Lookup table of storyline variables
extern StorylineVars hb_save_data[NUM_STORY_VARS];

// Names of the columns (array of strings)
extern char **hb_save_headers;

// Create the data structure for the save file
int hb_create_save_struct();

// Free the used memory by the save data structure
void hb_destroy_save_struct();

#endif