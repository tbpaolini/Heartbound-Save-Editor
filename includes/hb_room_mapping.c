/* 
    Hashmaps for these associations:
    - each room with the starting (x, y) coordinate of the player
    - each Room/Object of the save structure with its world
*/

#ifndef _HB_ROOM_MAPPING
#define _HB_ROOM_MAPPING

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <hb_save_struct.c>

#define ROOMS_FILE_PATH "..\\lib\\room_coordinates.tsv"     // File listing the player spawn point for each room
#define PLACES_FILE_PATH "..\\lib\\places_list.tsv"         // File that correlates each Room/Object on the save structure to its world
#define ROOM_MAP_SIZE (size_t)547                       // Number of slots on the hashmap for the Room File
#define PLACE_MAP_SIZE (size_t)307                      // Number of slots on the hashmap for the Place File
#define MAX_ROOM_AMOUNT (size_t)1000                    // Maximum number of rooms the Room File can have
#define MAX_PLACE_AMOUNT (size_t)1000                   // Maximum number of locations the Places File can have
static const size_t READ_BUFFER_SIZE = 100;             // Maximum number of characters per line

#define CHAPTER_AMOUNT 6
char *hb_chapter[CHAPTER_AMOUNT] = {"Global", "Hometown", "The Tower", "Animus", "Jotunheim", "REDACTED"};

// List of all rooms in the game and their repective coordinates where the player spawns
typedef struct HeartboundRoom
{
    char name[ROOM_NAME_SIZE];      // ID of the room
    double x;                       // x-axis coordinate
    double y;                       // y-axis coordinate
    struct HeartboundRoom *next;   // Next entry on the linked list (for when there is a collision on the hashmap)
} HeartboundRoom;

HeartboundRoom* hb_room_list;                  // Lookup table for the rooms
HeartboundRoom* hb_room_map[ROOM_MAP_SIZE];    // Maps to an element of the table by a hash function
size_t rooms_amount;                        // Number of rooms in the data structures

// List of Room/Objects and their respective worlds
typedef struct HeartboundLocation
{
    char name[ROOM_NAME_SIZE];      // Value of the Room/Object column of the save structure file
    uint8_t world;                  // Number of the world (0 - Global | 1 - Hometown | 2 - The Tower | 3 - Animus | 4 - Jotunheim | 5 - End)
    struct HeartboundLocation *next;  // Next entry on the linked list (for when there is a collision on the hashmap)
} HeartboundLocation;

HeartboundLocation* location_list;                    // Lookup table for the rooms
HeartboundLocation* location_map[PLACE_MAP_SIZE];     // Maps to an element of the table by a hash function
size_t locations_amount;                           // Number of locations in the data structures


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
    size_t size = strnlen_s(key, SAVE_STRUCT_BUFFER);

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
    while (!feof(my_file))
    {
        char cur_character = fgetc(my_file);    // Get the next character
        if (cur_character == '\n' || cur_character == EOF) amount++;    // Count line breaks or the end of file
        if (amount >= max_entries) break;       // Cap the count to the maximum value
    }
    
    // Return back to the initial position on the file
    fsetpos(my_file, &init_pos);

    // Return the amount of entries (not considering the header line)
    return amount - 1;

    /* NOTE:
        Since the function counts both the newline character and the end of file,
        then if the file ends with a newline then the amount returned will be 1
        more than the actual amount. I do not find this a big issue, since this
        value will be used to allocate enough memory to the array, and the size
        of just one element isn't that big.
        
        The end of file is counted in order to prevent a buffer overrun on the
        array, in case the file does not end in a newline.
    */
}

// Parse the rooms/locations files and build the lookup tables and hashmaps
static void hb_parse_rooms_locations()
{
    char *restrict read_buffer = malloc(READ_BUFFER_SIZE);  // Bufer for reading the lines of each file
    size_t list_index;      // Current index on the lookup table
    uint32_t map_index;     // Current index on the hashmap
    char *token;            // Value of the current column on the current line

    /* ------------------ Room File ------------------ */
    
    // Open the Room File
    FILE *rooms_file = fopen(ROOMS_FILE_PATH, "rt");

    // Count the number of rooms in the file
    rooms_amount = count_entries(rooms_file, MAX_ROOM_AMOUNT);
    hb_room_list = calloc( rooms_amount, sizeof(HeartboundRoom) );
    
    // Skip the header line
    fgets(read_buffer, READ_BUFFER_SIZE, rooms_file);

    // Initialize all room map entries to NULL
    for (uint32_t i = 0; i < ROOM_MAP_SIZE; i++)
    {
        hb_room_map[i] = NULL;
    }

    list_index = 0;
    while ( !feof(rooms_file) || (list_index < rooms_amount) )
    {
        // Get the next line from the file
        if (fgets(read_buffer, READ_BUFFER_SIZE, rooms_file) == NULL)
        {
            // Break if no more data could be read from the file
            break;
        }
        
        // Get the index number
        token = strtok(read_buffer, "\t");
        if (atoll(token) != list_index)
        {
            exit(EXIT_FAILURE);
            // TO DO: Error handling
        }

        // Get the room name
        token = strtok(NULL, "\t");
        strcpy_s(hb_room_list[list_index].name, ROOM_NAME_SIZE, token);

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

    /* ------------------ Place File ------------------ */

    // Open the Place File
    FILE *locations_file = fopen(PLACES_FILE_PATH, "rt");

    // Count the number of locations in the file
    locations_amount = count_entries(locations_file, MAX_PLACE_AMOUNT);
    location_list = calloc( locations_amount, sizeof(HeartboundLocation) );
    
    // Skip the header line
    fgets(read_buffer, READ_BUFFER_SIZE, locations_file);

    // Initialize all location map entries to NULL
    for (uint32_t i = 0; i < PLACE_MAP_SIZE; i++)
    {
        location_map[i] = NULL;
    }

    list_index = 0;
    while ( !feof(locations_file) || (list_index < locations_amount) )
    {
        // Get the next line from the file
        if (fgets(read_buffer, READ_BUFFER_SIZE, locations_file) == NULL)
        {
            // Break if no more data could be read from the file
            break;
        }
        
        // Get the index number
        token = strtok(read_buffer, "\t");
        if (atoll(token) != list_index)
        {
            exit(EXIT_FAILURE);
            // TO DO: Error handling
        }

        // Get the location name
        token = strtok(NULL, "\t");
        strcpy_s(location_list[list_index].name, ROOM_NAME_SIZE, token);

        // Get the world number
        token = strtok(NULL, "\t");
        location_list[list_index].world = atoi(token);

        // Pointer to the next element on the hashmap (in case a collision happens)
        location_list[list_index].next = NULL;

        // Add the new location to the hashmap
        map_index = hash(location_list[list_index].name, PLACE_MAP_SIZE);
        
        if (location_map[map_index] == NULL)    // No collision
        {
            location_map[map_index] = &(location_list[list_index]);
        }
        else    // Collision happened
        {
            // Place already on this spot
            HeartboundLocation *location_ptr = location_map[map_index];
            
            // Navigate through the linked list until the last element
            while (location_ptr->next != NULL)
            {
                location_ptr = location_ptr->next;
            }

            // Add the new location to the end of the linked list
            location_ptr->next = &(location_list[list_index]);
        }

        // Move to the next index
        list_index++;
    }

    // Close the Place File
    fclose(locations_file);
    
    // Deallocate the read buffer
    free(read_buffer);
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
    HeartboundLocation *location_ptr = location_map[map_index];

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
    free(hb_room_list);
    free(location_list);
}

#endif