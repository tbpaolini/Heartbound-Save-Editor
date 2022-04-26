/* 
    Include all source files related to the handling of the save file and its data.
    
    The function 'hb_open_save()' should be used for initializing the data structures
    and parsing the save file.
    
    The function 'hb_close_save()' destroys the data structures.
*/

#include <stdbool.h>
#include <hb_save.h>
#include <unistd.h>
#include <windows.h>

// Initialize the data structures and parse the save file
void hb_open_save(char *path)
{
    if (hb_save_is_open) return;
    hb_config_init();
    hb_create_save_struct();
    hb_find_save();
    
    // Try opening the provided path
    if (path == SAVE_PATH)
    {
        hb_read_save(SAVE_PATH);
    }
    else
    {
        int status = hb_read_save(path);    // This function sets 'hb_save_is_open' to 'true' if successful
    
        // Check if the file loadind was successful
        if (status == SAVE_FILE_NOT_VALID)
        {
            // Create the text of the error dialog if it failed
            char *message = calloc(TEXT_BUFFER_SIZE, sizeof(char));
            snprintf(
                message,
                TEXT_BUFFER_SIZE,
                "Coult not open '%s'.\n\n"
                "The default Heartbound save is going to be opened instead...",
                path
            );

            MessageBoxA(
                NULL,
                message,
                "Heartbound Save Editor - error",
                MB_ICONWARNING | MB_OK
            );
            
            // Deallocate the message buffer
            free(message);
            
            // If it was not possible to open, then open the default path
            hb_read_save(SAVE_PATH);
        }
    }

    hb_parse_rooms_locations();
}

// Free the memory allocated by the functions on 'hb_open_save()'
void hb_close_save()
{
    hb_save_is_open = false;
    hb_destroy_save_struct();
    hb_unmap_rooms_locations();
    hb_config_close();
}