/* 
    Include all source files related to the handling of the save file and its data.
    
    The function 'hb_open_save()' should be used for initializing the data structures
    and parsing the save file.
    
    The function 'hb_close_save()' destroys the data structures.
*/

#include <stdbool.h>
#include <hb_save.h>
#include <unistd.h>

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

            // Print the error to stderr
            fprintf(stderr, "Heartbound Save Editor - Error: %s", message);

            // Try showing a native error dialog
            GtkWidget *error_dialog = gtk_message_dialog_new(
                NULL,
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "%s",
                message
            );
            if (error_dialog != NULL)\
            {
                gtk_window_set_title(GTK_WINDOW(error_dialog), "Heartbound Save Editor - error");
                gtk_dialog_run(GTK_DIALOG(error_dialog));
                gtk_widget_destroy(error_dialog);
            }
            
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