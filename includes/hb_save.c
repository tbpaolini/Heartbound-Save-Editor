/* 
    Include all source files related to the handling of the save file and its data.
    
    The function 'hb_open_save()' should be used for initializing the data structures
    and parsing the save file.
    
    The function 'hb_close_save()' destroys the data structures.
*/

#include <hb_save.h>

// Initialize the data structures and parse the save file
void hb_open_save()
{
    hb_create_save_struct();
    hb_find_save();
    hb_read_save(SAVE_PATH);
    hb_parse_rooms_locations();
}

// Free the memory allocated by the functions on 'hb_open_save()'
void hb_close_save()
{
    hb_destroy_save_struct();
    hb_unmap_rooms_locations();
}