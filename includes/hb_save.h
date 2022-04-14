/* 
    Include all source files related to the handling of the save file and its data.
    
    The function 'hb_open_save()' should be used for initializing the data structures
    and parsing the save file.
    
    The function 'hb_close_save()' destroys the data structures.
*/

#ifndef _HB_SAVE
#define _HB_SAVE

#include <stdbool.h>
#include <hb_save_struct.h>     // Creation of the data structure for the save data
#include <hb_save_io.h>         // Reading and saving to the save file
#include <hb_room_mapping.h>    // Hashmaps for the game's rooms
#include <hb_gui_callback.h>    // Callback funtions for the user interface

// Initialize the data structures and parse the save file
void hb_open_save(char *path, bool using_loader);

// Free the memory allocated by the functions on 'hb_open_save()'
void hb_close_save();

#endif