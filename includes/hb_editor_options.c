/*
    Options manager for the Heartbound save editor.
    It stores and retrieves configurations of Heartbound Save Editor that should persist between different sessions.
    They are stored as a key/value pair. The config file is a text file in which each line is in the form:
        key=value
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <gtk/gtk.h>
#include <hb_editor_options.h>

#ifndef __USE_XOPEN2K8
#define __USE_XOPEN2K8          // Enable the 'strnlen()' function
#endif

// Amount of options currently stored in memory
static size_t config_count = 0;

// Data structures
static EditorSetting *config_list = NULL;              // Linked list of of settings (first iyem)
static EditorSetting *config_list_tail = NULL;         // Last item of the linked list
static EditorSetting *config_map[EDITOR_CFG_BUCKETS];  // Hash map of settings

// Map a key (as a string) into a position on the hash map
static uint32_t config_hash(char *key)
{
    // Get the most significant bit of a 32-bit unsigned integer
    #define MASK ((uint32_t)1 << (uint32_t)31)
    #define MSB32(N) ((N & MASK) >> 31)

    // Initial value of the hash (0xA5 = 0b1010'0101)
    uint32_t my_hash = 0xA5A5A5A5;

    // Length of the key
    size_t size = strnlen(key, EDITOR_CFG_BUFFER);

    if ( (size > 0) && (size < EDITOR_CFG_BUFFER) )
    {
        // Loop through each byte of the string
        for (size_t i = 0; i < size; i++)
        {
            my_hash += key[i];              // Add the byte value to the hash
            my_hash += key[i] * my_hash;    // Multiply the hash by the byte value, then add the result to the hash
            my_hash = (my_hash << 1) | MSB32(my_hash);  // Rotate the bits by 1 to the left
        }
    }

    // Return the remainder of the division of the hash by the amount of buckets on the hash map
    return (my_hash % EDITOR_CFG_BUCKETS);

    #undef MSB32
    #undef MASK
}

// Add a new key/value pair, and add it to the linked list and hash map
static bool config_add(char *key, char *value)
{
    // Allocate memory for the setting struct
    EditorSetting *my_setting = calloc(1, sizeof(EditorSetting));
    if (my_setting == NULL) return false;  // Stop if there's no available memory
    
    // Store the key/value pair on the struct

    // Get the size of the key and value
    size_t key_len = strnlen(key  , EDITOR_CFG_BUFFER);
    size_t value_len = strnlen(value  , EDITOR_CFG_BUFFER);
    
    // Try allocating memory for the key and value
    my_setting->key   = calloc(key_len + 1, sizeof(char));
    if (my_setting->key == NULL)
    {
        free(my_setting);
        return false;
    }
    
    my_setting->value = calloc(value_len + 1, sizeof(char));
    if (my_setting->value == NULL)
    {
        free(my_setting);
        free(my_setting->key);
        return false;
    }

    // Copy the key and value to the allocated memory
    strncpy(my_setting->key  , key  , key_len);
    strncpy(my_setting->value, value, value_len);

    // Pointers to the next entries on the data structures
    my_setting->list_next = NULL;
    my_setting->map_next  = NULL;

    // Store the struct at the end linked list
    if (config_list_tail != NULL)
    {
        // Next item of the list
        config_list_tail->list_next = my_setting;
    }
    else
    {
        // First item of the list
        config_list = my_setting;
    }

    // Set the added setting as the list's tail
    config_list_tail = my_setting;

    // Store the struct on the hash map
    size_t map_index = config_hash(key);
    if (config_map[map_index] == NULL)
    {
        // No collision, just store the element
        config_map[map_index] = my_setting;
    }
    else
    {
        // Collision happened, link the elements
        EditorSetting *previous = config_map[map_index];
        EditorSetting *current = config_map[map_index]->map_next;
        
        // Keep moving to the next element on the map's bucket until we get to the last element
        while (current != NULL)
        {
            previous = current;
            current = current->map_next;
        }
        
        // Store the element on the last position
        previous->map_next = my_setting;
    }

    return true;
}

// Load the editor's settings into memory
void hb_config_init()
{
    const char *home_folder = getenv("HOME");
    EDITOR_CFG_LOCATION = calloc(FILENAME_MAX+1, sizeof(char));

    // Find and create (if necessary) the folder of the configurations file
    snprintf(EDITOR_CFG_LOCATION, FILENAME_MAX+1, "%s/%s", home_folder, ".config/heartbound-save-editor");
    g_mkdir_with_parents(EDITOR_CFG_LOCATION, 0700);

    // Build the absolute path to the configurations file
    strncat(EDITOR_CFG_LOCATION, "/", FILENAME_MAX);
    strncat(EDITOR_CFG_LOCATION, EDITOR_CFG_NAME, FILENAME_MAX);
    
    // Open the configurations file
    FILE *config_file = fopen(EDITOR_CFG_LOCATION, "r");
    if (config_file == NULL) return;    // Return if there is no file to be read

    char line_buffer[EDITOR_CFG_BUFFER];    // Stores the line being read
    char *status, *key, *value;             // Stores the key/value pair and the status of the read operation
    EditorSetting *list_head = NULL;        // The current position on the linked list
    
    // Keep reading the lines of the file
    // (until the end of the file or the line limit have been reached)
    while (!feof(config_file))
    {
        // Read a line from the file
        status = fgets(line_buffer, EDITOR_CFG_BUFFER, config_file);
        if (status == NULL) break;  // Break if no line could be read

        // Parse the key/value pair from the line
        // (move to the next line if the pair could not be parsed)
        key   = strtok(line_buffer, "=");
        if (key == NULL)   continue;
        
        value = strtok(NULL, "\n");
        if (value == NULL) continue;

        // Strip the newline at the end of the value, if there is any
        size_t v_last_char = strlen(value) - 1;
        if (value[v_last_char] == '\n') value[v_last_char] = '\0';

        // Add the key/value pair to the data structures
        bool add_success = config_add(key, value);
        if (!add_success) break;    // Stop parsing if there's no available memory

        // Increment the amount of settings and break if we reached the maximum amount of lines
        if (++config_count >= EDITOR_CFG_MAX_ENTRIES) break;
    }
    
    // Close the configurations file
    fclose(config_file);
}

// Store the key/value pair of a setting
void hb_config_set(char *key, char *value)
{
    // Get the index of the key on the hash map
    if (key == NULL) return;
    uint32_t map_index = config_hash(key);

    if (config_map[map_index] != NULL)
    {
        // Go through all keys on that index until one matches the searched key
        EditorSetting *current = config_map[map_index];
        while (current != NULL)
        {
            if (strncmp(key, current->key, EDITOR_CFG_BUFFER) == 0)
            {
                // Pointer to the old value
                char *old_value = current->value;

                // Try to replace the old value
                size_t value_len = strnlen(value, EDITOR_CFG_BUFFER);
                current->value = calloc(value_len + 1, sizeof(char));
                if (current->value != NULL)
                {
                    // Store the new value and delete the old one
                    strncpy(current->value, value, value_len);
                    free(old_value);
                    config_write();
                }
                else
                {
                    // Keep the old value if there is not enough memory
                    current->value = old_value;
                }

                // Return if a matching key was found
                return;
            }
            
            // Go to the next key
            current = current->map_next;
        }
    }
    
    // Create the key if it does not exist
    config_add(key, value);
    config_write();
}

// Retrieve the key/value pair of a setting
// (return and store the default value, if the key could not be found)
char *hb_config_get(char *key, char *default_value)
{
    // Get the index of the key on the hash map
    uint32_t map_index = config_hash(key);
    
    // Is there a key there?
    if (config_map[map_index] != NULL)
    {
        // Go through all keys on that index until one matches the searched key
        EditorSetting *current = config_map[map_index];
        while (current != NULL)
        {
            if (strncmp(key, current->key, EDITOR_CFG_BUFFER) == 0)
            {
                // Return the value of the matching key
                return current->value;
            }
            
            // Go to the next key
            current = current->map_next;
        }
    }
    
    // Store and return the default value if no match was found
    config_add(key, default_value);
    return default_value;
}

// Delete a key (and its value) from the configurations' data structures
void hb_config_remove(char *key)
{
    if (key == NULL) return;

    EditorSetting *current = config_list;   // Current setting
    EditorSetting *previous = NULL;         // Previous setting
    EditorSetting *to_delete = NULL;        // Setting to be deleted

    // Navigate through the linked list until we find a setting that matches the key
    while (current != NULL)
    {
        // Check if the current setting's key matches
        if (strncmp(key, current->key, EDITOR_CFG_BUFFER) == 0)
        {
            
            if (previous != NULL)
            {
                // If we are not at the beginning of the list, link the previous item to the next
                previous->list_next = current->list_next;
            }
            else
            {
                // If we are at the beginning of the list, set the next item as the new first item
                config_list = current->list_next;
            }
            
            // Mark the current setting to be deleted
            to_delete = current;

            // If the deleted setting is the last one, set the previous item as the list's last item
            if (to_delete == config_list_tail) config_list_tail = previous;

            // Break from the loop once the matching key was found
            break;
        }
        
        // Move to the next setting on the linked list
        previous = current;
        current = current->list_next;
    }
    
    // If a matching key was found
    if (to_delete != NULL)
    {
        size_t map_index = config_hash(key);    // Get the setting's index at the hash map
        current = config_map[map_index];        // Get that index from the hash map
        previous = NULL;                        // Previous setting, if there are more than one at that map's index

        // Navigate through the settings on the hash map's index
        while (current != NULL)
        {
            // Check if the setting matches the key
            if (strncmp(key, current->key, EDITOR_CFG_BUFFER) == 0)
            {
                if (previous != NULL)
                {
                    // If there is a previous setting, link it to the next one
                    previous->map_next = current->map_next;
                }
                else
                {
                    // Are there more settings afterwards?
                    if (current->map_next == NULL)
                    {
                        // If there are no more settings, set the map index to NULL
                        config_map[map_index] = NULL;
                    }
                    else
                    {
                        // If there are other settings, set the index to the next setting
                        config_map[map_index] = current->map_next;
                    }
                }

                // Break from the loop once a matching setting was found
                break;
            }

            // Move to the next setting on the hash map's index
            previous == current;
            current = current->map_next;
        }

        // Delete the setting from memory, once its links have been removed from the linked list and the hash map
        free(to_delete);
        --config_count;     // Decrement the count of configurations
        config_write();     // Save the remaining settings to file
    }
}

// Write the settings (key/value pairs) to the configurations file
static void config_write()
{
    // Open the file for writing
    FILE *config_file = fopen(EDITOR_CFG_LOCATION, "w");
    if (config_file == NULL) return;

    // Write all key/value pairs to the file
    EditorSetting *current = config_list;
    while (current != NULL)
    {
        fprintf(config_file, "%s=%s\n", current->key, current->value);
        current = current->list_next;   // Move to the next item on the linked list
    }

    // Close the file
    fclose(config_file);
}

// Save the settings then free the memory used by them
void hb_config_close()
{
    // Write the settings to file
    config_write();
    
    EditorSetting *current_config = config_list;
    EditorSetting *previous_config;
    
    while (current_config != NULL)
    {
        free(current_config->key);
        free(current_config->value);
        previous_config = current_config;
        current_config = current_config->list_next;
        free(previous_config);
    }

    free(EDITOR_CFG_LOCATION);
    config_count = 0;
}