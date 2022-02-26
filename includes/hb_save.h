/* 
    Include all source files related to the handling of the save file and its data.
    
    The function 'init_save()' should be used for initializing the data structures
    and parsing the save file.
*/

#ifndef _HB_SAVE
#define _HB_SAVE

#include <hb_save_struct.c>     // Creation of the data structure for the save data
#include <hb_save_io.c>         // Reading and saving to the save file

void init_save()
{
    create_save_struct();
    find_save();
    read_save();
}

#endif