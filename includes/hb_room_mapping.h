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
#include <hb_save_struct.h>

#define ROOMS_FILE_PATH "..\\lib\\room_coordinates.tsv"     // File listing the player spawn point for each room
#define PLACES_FILE_PATH "..\\lib\\places_list.tsv"         // File that correlates each Room/Object on the save structure to its world
#define ROOM_MAP_SIZE (size_t)547                       // Number of slots on the hashmap for the Room File
#define PLACE_MAP_SIZE (size_t)307                      // Number of slots on the hashmap for the Place File
#define MAX_ROOM_AMOUNT (size_t)1000                    // Maximum number of rooms the Room File can have
#define MAX_PLACE_AMOUNT (size_t)1000                   // Maximum number of locations the Places File can have
#define STRUCT_LINE_BUFFER (size_t)100                  // Maximum number of characters per line

#define CHAPTER_AMOUNT 6                    // Number of chapters of the game
extern char *hb_chapter[CHAPTER_AMOUNT];    // Array of chapter names (as strings)

// List of all rooms in the game and their repective coordinates where the player spawns
typedef struct HeartboundRoom
{
    char name[ROOM_NAME_SIZE];      // ID of the room
    double x;                       // x-axis coordinate
    double y;                       // y-axis coordinate
    struct HeartboundRoom *next;    // Next entry on the linked list (for when there is a collision on the hashmap)
} HeartboundRoom;

extern HeartboundRoom* hb_room_list;                  // Lookup table for the rooms
extern HeartboundRoom* hb_room_map[ROOM_MAP_SIZE];    // Maps to an element of the table by a hash function
extern size_t hb_rooms_amount;                        // Number of rooms in the data structures

// List of Room/Objects and their respective worlds
typedef struct HeartboundLocation
{
    char name[ROOM_NAME_SIZE];        // Value of the Room/Object column of the save structure file
    uint8_t world;                    // Number of the world (0 - Global | 1 - Hometown | 2 - The Tower | 3 - Animus | 4 - Jotunheim | 5 - End)
    size_t position;                  // The order in which the location will appear in the program's window
    char *image;                      // The file name of the image used to illustrate the location on the program's window
    struct HeartboundLocation *next;  // Next entry on the linked list (for when there is a collision on the hashmap)
} HeartboundLocation;

extern HeartboundLocation* hb_location_list;                 // Lookup table for the rooms
extern HeartboundLocation* hb_location_map[PLACE_MAP_SIZE];  // Maps to an element of the table by a hash function
extern size_t hb_locations_amount;                           // Number of locations in the data structures


// ********************************************************************
// Hashmap for retrieving the values from the lists of rooms and locations
// ********************************************************************

// Take a string ('key') and calculate an integer from 0 to 'slots - 1'.
// The result will always be the same for the same key.
static uint32_t hash(char *key, uint32_t slots);

// Count the number of entries in the Room/Place files
static size_t count_entries(FILE *my_file, size_t max_entries);

// Parse the rooms/locations files and build the lookup tables and hashmaps
void hb_parse_rooms_locations();

// Retrieve a room struct from its name
HeartboundRoom* hb_get_room(char *name);

// Retrieve a location struct from its name
HeartboundLocation* hb_get_location(char *name);

// Free the memory used by the Places and Rooms list
void hb_unmap_rooms_locations();

#endif