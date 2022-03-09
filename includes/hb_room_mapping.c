/* 
    Hashmaps for these associations:
    - each room with the starting (x, y) coordinate of the player
    - each Room/Object of the save structure with its world
*/

#ifndef _HB_ROOM_MAPPING
#define _HB_ROOM_MAPPING

#include <stdint.h>
#include <string.h>
#include <hb_save_struct.c>

#define ROOM_LIST_MAX_SIZE (size_t)1000
#define PLACE_LIST_MAX_SIZE (size_t)500

// List of all rooms in the game and their repective coordinates where the player spawns
typedef struct
{
    char name[ROOM_NAME_SIZE];  // ID of the room
    double x;                   // x-axis coordinate
    double y;                   // y-axis coordinate
    HeartboundRoom *_next;      // Next entry on the linked list (for when there is a collision on the hashmap)
} HeartboundRoom;

room_list[ROOM_LIST_MAX_SIZE];

// List of Room/Objects and their respective worlds
typedef struct
{
    char name[ROOM_NAME_SIZE];  // Value of the Room/Object column of the save structure file
    uint8_t world;              // Number of the world (0 - Global | 1 - Hometown | 2 - The Tower | 3 - Animus | 4 - Jotunheim | 5 - End)
    HeartboundPlace *_next;     // Next entry on the linked list (for when there is a collision on the hashmap)
} HeartboundPlace;

HeartboundPlace place_list[PLACE_LIST_MAX_SIZE];


// ********************************************************************
// Hashmap for retrieving the values from the lists of rooms and places
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

#endif