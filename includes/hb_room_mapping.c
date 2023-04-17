/* 
    Hashmaps for these associations:
    - each room with the starting (x, y) coordinate of the player
    - each Room/Object of the save structure with its world
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "../config.h"
#include <hb_room_mapping.h>

#ifndef __USE_XOPEN2K8
#define __USE_XOPEN2K8          // Enable the 'strnlen()' function
#endif

// Array of chapter names (as strings)
char *hb_chapter[CHAPTER_AMOUNT] = {"_Global", "H_ometown", "The _Tower", "_Animus", "_Jotunheim", "_REDACTED"};

// List of all rooms in the game and their repective coordinates where the player spawns
HeartboundRoom* hb_room_list;                  // Lookup table for the rooms
HeartboundRoom* hb_room_map[ROOM_MAP_SIZE];    // Maps to an element of the table by a hash function
size_t hb_rooms_amount;                        // Number of rooms in the data structures

// List of Room/Objects and their respective worlds
HeartboundLocation* hb_location_list;                   // Lookup table for the rooms
HeartboundLocation* hb_location_map[PLACE_MAP_SIZE];    // Maps to an element of the table by a hash function
size_t hb_locations_amount;                             // Number of locations in the data structures

// Whether the rooms data structures have been built already
static bool rooms_are_initialized = false;

// ********************************************************************
// Hashmap for retrieving the values from the lists of rooms and locations
// ********************************************************************

// Bit-mask for obtaining the most significant bit of a 32-bit unsigned integer
static const uint32_t MASK = (1 << 31);

// Obtain the most significant bit (the leftmost binary digit) of a 32-bit unsigned integer
#define MSB32(N) ((N & MASK) >> 31)

// Take a string ('key') and calculate an integer from 0 to 'slots - 1'.
// The result will always be the same for the same key.
static uint32_t hash(char *key, uint32_t slots)
{
    // Initial value of the hash (0xA5 = 0b1010'0101)
    uint32_t my_hash = 0xA5A5A5A5;

    // Length of the key
    const size_t size = key ? strlen(key) : 0;

    if ( (size > 0) && (size < SAVE_STRUCT_BUFFER) )
    {
        // Loop through each byte of the string
        for (size_t i = 0; i < size; i++)
        {
            my_hash += key[i];              // Add the byte value to the hash
            my_hash += key[i] * my_hash;    // Multiply the hash by the byte value, then add the result to the hash
            my_hash = (my_hash << 1) | MSB32(my_hash);  // Rotate the bits by 1 to the left
        }
    }

    // Return the remainder of the division of the hash by the 'slots' amount
    return (my_hash % slots);
}

#undef MSB32

// Count the number of entries in the Room/Place files
static size_t count_entries(FILE *my_file, size_t max_entries)
{
    // Get the current position at the file
    fpos_t init_pos;
    fgetpos(my_file, &init_pos);

    // Return back to the beginning of the file
    rewind(my_file);

    // Count the number of rooms
    size_t amount = 0;
    char prev_character = '\0';
    char cur_character = '\0';
    
    while (!feof(my_file))
    {
        prev_character = cur_character;
        cur_character = fgetc(my_file);    // Get the next character
        if (cur_character == '\n' || cur_character == EOF) amount++;    // Count line breaks or the end of file
        if (amount >= max_entries) break;       // Cap the count to the maximum value
    }

    if (prev_character == '\n' && cur_character == EOF) amount--;
    
    // Return back to the initial position on the file
    fsetpos(my_file, &init_pos);

    // Return the amount of entries (not considering the header line)
    return amount - 1;

    /* NOTE:
        Since the function counts both the newline character and the end of file,
        then if the file ends with a newline then the amount returned will be 1
        more than the actual amount. So the function subtracts 1 if the last two
        charates are a New Line followed by an End of File.
        
        The end of file is counted in order to prevent a buffer overrun on the
        array, in case the file does not end in a newline.
    */
}

// Parse the rooms/locations files and build the lookup tables and hashmaps
void hb_parse_rooms_locations()
{
    if (rooms_are_initialized) return;
    
    char *restrict read_buffer = malloc(STRUCT_LINE_BUFFER);  // Bufer for reading the lines of each file
    size_t list_index;      // Current index on the lookup table
    uint32_t map_index;     // Current index on the hashmap
    char *token;            // Value of the current column on the current line

    /* ------------------ Room File ------------------ */
    
    // Open the Room File
    FILE *rooms_file = fopen(ROOMS_FILE_PATH, "rt");

    // Display error dialog and exit if the rooms file is missing
    if (rooms_file == NULL)
    {
        NATIVE_ERROR(
            "File %s could not be found.\nPlease download Heartbound Save Editor again.",
            ROOMS_FILE_PATH,
            1024
        );
    }

    // Count the number of rooms in the file
    hb_rooms_amount = count_entries(rooms_file, MAX_ROOM_AMOUNT);
    hb_room_list = calloc( hb_rooms_amount, sizeof(HeartboundRoom) );
    
    // Skip the header line
    {char *fgets_status = fgets(read_buffer, STRUCT_LINE_BUFFER, rooms_file);}

    // Initialize all room map entries to NULL
    for (uint32_t i = 0; i < ROOM_MAP_SIZE; i++)
    {
        hb_room_map[i] = NULL;
    }

    list_index = 0;
    while ( !feof(rooms_file) || (list_index < hb_rooms_amount) )
    {
        // Get the next line from the file
        if (fgets(read_buffer, STRUCT_LINE_BUFFER, rooms_file) == NULL)
        {
            // Break if no more data could be read from the file
            break;
        }
        
        // Get the index number
        token = strtok(read_buffer, "\t");
        if (atoll(token) != list_index)
        {
            // Display error dialog and exit if the rooms file fails the consistency check
            NATIVE_ERROR(
                "File %s is corrupted.\nPlease download Heartbound Save Editor again.",
                ROOMS_FILE_PATH,
                1024
            );
        }
        hb_room_list[list_index].index = atoll(token);  // Store the index number

        // Get the room name
        token = strtok(NULL, "\t");
        strncpy(hb_room_list[list_index].name, token, ROOM_NAME_SIZE);

        // Get (x,y) coordinates
        token = strtok(NULL, "\t");
        hb_room_list[list_index].x = atof(token);
        token = strtok(NULL, "\t");
        hb_room_list[list_index].y = atof(token);

        // Pointer to the next element on the hashmap (in case a collision happens)
        hb_room_list[list_index].next = NULL;
        
        // Add the new room to the hashmap
        map_index = hash(hb_room_list[list_index].name, ROOM_MAP_SIZE);
        
        if (hb_room_map[map_index] == NULL)    // No collision
        {
            hb_room_map[map_index] = &(hb_room_list[list_index]);
        }
        else    // Collision happened
        {
            // Room already on this spot
            HeartboundRoom *room_ptr = hb_room_map[map_index];
            
            // Navigate through the linked list until the last element
            while (room_ptr->next != NULL)
            {
                room_ptr = room_ptr->next;
            }

            // Add the new room to the end of the linked list
            room_ptr->next = &(hb_room_list[list_index]);
        }

        // Move to the next index
        list_index++;
    }
    
    // Close the Room File
    fclose(rooms_file);

    /* ------------------ Locations File ------------------ */

    // Open the Place File
    FILE *locations_file = fopen(PLACES_FILE_PATH, "rt");

    // Display error dialog and exit if the places file is missing
    if (locations_file == NULL)
    {
        NATIVE_ERROR(
            "File %s could not be found.\nPlease download Heartbound Save Editor again.",
            PLACES_FILE_PATH,
            1024
        );
    }

    // Count the number of locations in the file
    hb_locations_amount = count_entries(locations_file, MAX_PLACE_AMOUNT);
    hb_location_list = calloc( hb_locations_amount, sizeof(HeartboundLocation) );
    
    // Skip the header line
    {char *fgets_status = fgets(read_buffer, STRUCT_LINE_BUFFER, locations_file);}

    // Initialize all location map entries to NULL
    for (uint32_t i = 0; i < PLACE_MAP_SIZE; i++)
    {
        hb_location_map[i] = NULL;
    }

    // Get the length of the textures' path
    texture_path_size = strlen(TEXTURES_FOLDER);

    list_index = 0;
    while ( !feof(locations_file) || (list_index < hb_locations_amount) )
    {
        // Get the next line from the file
        if (fgets(read_buffer, STRUCT_LINE_BUFFER, locations_file) == NULL)
        {
            // Break if no more data could be read from the file
            break;
        }
        
        // Get the index number
        token = strtok(read_buffer, "\t");
        if (atoll(token) != list_index)
        {
            // Display error dialog and exit if the places file fails the consistency check
            NATIVE_ERROR(
                "File %s is corrupted.\nPlease download Heartbound Save Editor again.",
                PLACES_FILE_PATH,
                1024
            );
        }

        // Get the location name
        token = strtok(NULL, "\t");
        strncpy(hb_location_list[list_index].name, token, ROOM_NAME_SIZE);

        // Get the world number
        token = strtok(NULL, "\t");
        hb_location_list[list_index].world = atoll(token);

        // Get the position
        token = strtok(NULL, "\t");
        hb_location_list[list_index].position = atoll(token);

        // Get the image's file name
        token = strtok(NULL, "\t");
        size_t filename_size = strnlen(token, STRUCT_LINE_BUFFER) + texture_path_size;
        if ( (filename_size > 0) && (filename_size < STRUCT_LINE_BUFFER) )
        {
            // Copy the file name
            size_t filename_buffer_size = filename_size * sizeof(char) + 1;
            hb_location_list[list_index].image = malloc( filename_buffer_size );
            snprintf(
                hb_location_list[list_index].image,     // Where to store the file path
                filename_buffer_size, "%s%s",           // The pattern: folder + file name
                TEXTURES_FOLDER,                        // Textures folder
                token                                   // File name of the texture
            );
            
            // Remove the line break at the end, if necessary
            if (hb_location_list[list_index].image[filename_size - 1] == '\n')
            {
                hb_location_list[list_index].image[filename_size - 1] = '\0';
            }
        }
        else
        {
            hb_location_list[list_index].image = NULL;
        }

        // Pointer to the next element on the hashmap (in case a collision happens)
        hb_location_list[list_index].next = NULL;

        // Add the new location to the hashmap
        map_index = hash(hb_location_list[list_index].name, PLACE_MAP_SIZE);
        
        if (hb_location_map[map_index] == NULL)    // No collision
        {
            hb_location_map[map_index] = &(hb_location_list[list_index]);
        }
        else    // Collision happened
        {
            // Place already on this spot
            HeartboundLocation *location_ptr = hb_location_map[map_index];
            
            // Navigate through the linked list until the last element
            while (location_ptr->next != NULL)
            {
                location_ptr = location_ptr->next;
            }

            // Add the new location to the end of the linked list
            location_ptr->next = &(hb_location_list[list_index]);
        }

        // Move to the next index
        list_index++;
    }

    // Close the Place File
    fclose(locations_file);
    
    // Deallocate the read buffer
    free(read_buffer);

    rooms_are_initialized = true;
}

// Retrieve a room struct from its name
HeartboundRoom* hb_get_room(char *name)
{
    // Calculate the index on the hashmap
    uint32_t map_index = hash(name, ROOM_MAP_SIZE);
    HeartboundRoom *room_ptr = hb_room_map[map_index];

    // If there is nothing on that index, return NULL
    if (room_ptr == NULL) return NULL;

    // Loop through all elements on that index until one matches the name
    while ( strncmp(name, room_ptr->name, ROOM_NAME_SIZE) != 0 )
    {
        room_ptr = room_ptr->next;         // Go to the next element on the linked list
        if (room_ptr == NULL) return NULL;  // Return NULL if there is no next element
    }

    // Return the pointer to the struct if a match was found
    return room_ptr;
}

// Retrieve a location struct from its name
HeartboundLocation* hb_get_location(char *name)
{
    // Calculate the index on the hashmap
    uint32_t map_index = hash(name, PLACE_MAP_SIZE);
    HeartboundLocation *location_ptr = hb_location_map[map_index];

    // If there is nothing on that index, return NULL
    if (location_ptr == NULL) return NULL;

    // Loop through all elements on that index until one matches the name
    while ( strncmp(name, location_ptr->name, ROOM_NAME_SIZE) != 0 )
    {
        location_ptr = location_ptr->next;         // Go to the next element on the linked list
        if (location_ptr == NULL) return NULL;   // Return NULL if there is no next element
    }

    // Return the pointer to the struct if a match was found
    return location_ptr;
}

// Free the memory used by the Places and Rooms list
void hb_unmap_rooms_locations()
{
    for (size_t i = 0; i < hb_locations_amount; i++)
    {
        free(hb_location_list[i].image);
    }
    
    free(hb_room_list);
    free(hb_location_list);

    rooms_are_initialized = false;
}