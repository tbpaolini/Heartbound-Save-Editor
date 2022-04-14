/* 
    Include all source files related to the handling of the save file and its data.
    
    The function 'hb_open_save()' should be used for initializing the data structures
    and parsing the save file.
    
    The function 'hb_close_save()' destroys the data structures.
*/

#include <stdbool.h>
#include <hb_save.h>
#include <windows.h>

// Initialize the data structures and parse the save file
void hb_open_save(char *path)
{
    if (hb_save_is_open) return;
    hb_create_save_struct();
    hb_find_save();
    
    // Try opening the provided path
    int status = hb_read_save(path);    // This function sets 'hb_save_is_open' to 'true' if successful
    if (status == SAVE_FILE_NOT_VALID && path != SAVE_PATH)
    {
        // Create the text of the error dialog
        char *message = calloc(TEXT_BUFFER_SIZE, sizeof(char));
        snprintf(
            message,
            TEXT_BUFFER_SIZE,
            "Coult not open:\n%s\n\n"
            "The default Heartbound save is going to be opened instead.",
            path
        );

        MessageBoxA(
            NULL,
            message,
            "Heartbound Save Editor - error",
            MB_ICONERROR | MB_OK
        );
        
        // Deallocate the message buffer
        free(message);
        
        // If not possible to open it, then open the default path
        hb_read_save(SAVE_PATH);
    }

    hb_parse_rooms_locations();
}

// Free the memory allocated by the functions on 'hb_open_save()'
void hb_close_save()
{
    hb_save_is_open = false;
    hb_destroy_save_struct();
    hb_unmap_rooms_locations();
}