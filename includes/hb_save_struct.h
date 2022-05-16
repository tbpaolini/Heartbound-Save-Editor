/* Template and data structure for the saved data of Heartbound */

#ifndef _HB_SAVE_STRUCT
#define _HB_SAVE_STRUCT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <gtk/gtk.h>

#define SAVE_STRUCT_LOC "../lib/heartbound-save-editor/structure/save_structure.tsv"   // Path to the file with the save structure
#define SEED_SIZE (size_t)11                        // Amount of characters (plus null terminator and newline) of the game seed string
#define ROOM_NAME_SIZE (size_t) 50                  // Maximum amount of characters (plus null terminator and newline) for the room name string
#define SAVE_STRUCT_BUFFER (size_t)500              // Buffer size (in bytes) for each line of the structure file
#define NUM_STORY_VARS (size_t)1000                 // Amount of storyline variables in the save file
#define COLUMN_OFFSET (size_t)6                     // Amount of columns until the storyline variable values
#define ROW_OFFSET (size_t)7                        // Amount of rows before the first storyline variable

#define TURTLEFARM_STRUCT_LOC "..\\lib\\structure\\turtlefarm_crops.tsv"    // The file that stores the layout of the Mossback's farm
#define TURTLEFARM_STRUCT_BUFFER (size_t)150                                // Buffer size (in bytes) for each line of the farm layout file
#define TURTLEFARM_WIDTH (size_t)16     // Amount of columns of crops on the Mossback's farms
#define TURTLEFARM_HEIGHT (size_t)15    // Amount of rows of crops on the Mossback's farms

// Player attributes
extern char hb_game_seed[SEED_SIZE];                       // Game seed (9 decimal characters long)
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

// Store the pointer of the GTK widget used to store the value of a storyline variable
typedef union VariableWidget
{
    GtkEntry *entry;
    GSList *group;
} VariableWidget;

// The storyline variables
typedef struct StorylineVars
{
    size_t index;                // Number of the storyline var (used for debugging)
    double value;                // Value of this entry on the save file
    char *location;              // In-game location that this entry applies to
    char *name;                  // Description of the in-game feature this entry refers to
    char *info;                  // Further description of the feature
    char *unit;                  // Measurement unit of the value this entry represents
    double def;                  // Default value of the variable
    double maximum;              // Maximum value of the variable (0.0 if there is no maximum)
    size_t num_entries;          // Amount of different values that the field accept (0 if it accepts any value)
    ValueAlias *aliases;         // Associate each numeric value to its meaning (as strings)
    VariableWidget widget;       // Pointer to the GTK entry or the radio buttons group used to display the variable's value
    bool is_bitmask;             // If the value represents a bitmask (32-bits)
    bool used;                   // Wheter the variable has data on it
} StorylineVars;

// Lookup table of storyline variables
extern StorylineVars hb_save_data[NUM_STORY_VARS];

// Names of the columns (array of strings)
extern char **hb_save_headers;

// Storage of which crops of the Mossback's farm have been destroyed
// Note: Their states are stored as a bitmask across 8 different variables,
//       with 1 meaning destroyed, and 0 meaning not destroyed.
typedef struct TurtlefarmCrop
{
    size_t var;     // Which storyline variable has the crop's state
    size_t bit;     // Which bit of the variable stores this specific crop's state
    size_t x;       // Column on the farm's grid
    size_t y;       // Row on the farm's grid
    GtkToggleButton *widget;    // Pointer to the checkbox widget that controls this crop
} TurtlefarmCrop;

extern TurtlefarmCrop hb_turtlefarm_layout[TURTLEFARM_HEIGHT][TURTLEFARM_WIDTH];
extern double hb_turtlefarm_mask[32];  // The first 32 powers of 2, as double precision floats
// Note: Since I am storing all variable values as doubles, I can't just do bitwise operations
//       with the values of the crops, without changing how the rest of the variables work.
//       Since a value using a bitmask is the exception, rather than the rule, then I am
//       just going to add or subtract the powers if two to the bitmask values.

// Create the data structure for the save file
bool hb_create_save_struct();

// Load into memory the layout of the Mossbacks farm
static void turtlefarm_init();

// Free the used memory by the save data structure
void hb_destroy_save_struct();

#endif