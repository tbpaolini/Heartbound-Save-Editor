/*
    Options manager for the Heartbound save editor.
    It stores and retrieves configurations of Heartbound Save Editor that should persist between different sessions.
    They are stored as a key/value pair. The config file is a text file in which each line is in the form:
        key=value
*/

#ifndef _HB_EDITOR_OPTIONS
#define _HB_EDITOR_OPTIONS

#include <stdio.h>
#include <stdbool.h>

#define EDITOR_CFG_LOCATION "..\\etc\\editor.cfg"   // Location of the configurations file (relative to the main executable)
#define EDITOR_CFG_BUCKETS     (uint32_t)101        // Amount of slots in memory that the configurations's hash map has
#define EDITOR_CFG_BUFFER      (size_t)1024         // Maximum amount of characters on each line of the configurations file
#define EDITOR_CFG_MAX_ENTRIES (size_t)10000        // Maximum amount of lines of the settings file
                                                    // (just a sanity check to prevent a huge file from being loaded)

typedef struct EditorSetting
{
    char *key;      // Key (as string)
    char *value;    // Value (as string, it should be converted to numeric if necessary)
    struct EditorSetting *list_next;   // Pointer to next struct on the linked list
    struct EditorSetting *map_next;    // Pointer to next struct on the hash map
} EditorSetting;

static size_t config_count;         // How many key/value pairs (of settings) there are in memory
static EditorSetting *config_list;                      // Linked list of of settings (first item)
static EditorSetting *config_list_tail;                 // Last item of the linked list
static EditorSetting *config_map[EDITOR_CFG_BUCKETS];   // Hash map of settings

// Map a key (as a string) into a position on the hash map
static uint32_t config_hash(char *key);

// Add a new key/value pair, and add it to the linked list and hash map
static bool config_add(char *key, char *value);

// Load the editor's settings into memory
void hb_config_init();

// Store the key/value pair of a setting
void hb_config_set(char *key, char *value);

// Store the key/value pair of a setting, by using a boolean value ('true' or 'false')
void hb_config_set_bool(char *key, bool value);

// Retrieve the key/value pair of a setting
// (return and store the default value, if the key could not be found)
char *hb_config_get(char *key, char *default_value);

// Get the key's value as a boolean data type ('true' or 'false')
// (return and create a key with the default value, if an existing key is not found)
bool hb_config_get_bool(char *key, bool default_value);

// Delete a key (and its value) from the configurations' data structures
void hb_config_remove(char *key);

// Write the settings (key/value pairs) to the configurations file
static void config_write();

// Save the settings then free the memory used by them
void hb_config_close();

#endif