/* 
    Include all source files related to the handling of the save file and its data.
    
    The function 'open_save()' should be used for initializing the data structures
    and parsing the save file.
    
    The function 'close_save()' destroys the data structures.
*/

#ifndef _HB_SAVE
#define _HB_SAVE

#include <hb_save_struct.c>     // Creation of the data structure for the save data
#include <hb_save_io.c>         // Reading and saving to the save file
#include <hb_room_mapping.c>    // Hashmaps for the game's rooms

// Initialize the data structures and parse the save file
void open_save()
{
    create_save_struct();
    find_save();
    read_save();
    parse_rooms_locations();
}

// Free the memory allocated by the functions on 'open_save()'
void close_save()
{
    destroy_save_struct();
    unmap_rooms_locations();
}

#endif