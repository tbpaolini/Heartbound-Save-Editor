/*
    Callback functions used by the graphical user interface (GUI) in order to
    validate or modify the save data.
*/

#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <gtk\gtk.h>
#include <windows.h>
#include <hb_save.h>
#include <hb_gui_callback.h>

// Set a storyline variable's value when one of its radio buttons is clicked
void hb_setvar_radio_button(GtkRadioButton* widget, StorylineVars *story_var)
{
    // Exit the function if the editor is currently loading a file
    if (is_loading_file) return;
    
    // Is the button that triggered the 'toggled' event active?
    gboolean button_is_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    if (!button_is_active) return;
    /*
        Since the 'toggled' event is fired by both the button that got unselected
        and the one that got selected, then we need to check whether the button is
        active so we don't end up executing the same function twice.
    */
    
    // Get the group that the button is part of
    GSList *my_group = gtk_radio_button_get_group(widget);
    GtkToggleButton *current_button = GTK_TOGGLE_BUTTON(my_group->data);
    /*
        The group is a linked list. The '->data' attribute is the pointer to
        a button, and the '->next' attribute the pointer to the next element
        on the list.
    */

    // Loop backwards through the possible variable's values
    // (Because whem iterating through the buttons they come in the opposite
    //  order they were added: from newest to oldest)
    for (size_t i = story_var->num_entries - 1; i >= 0; i--)
    {
        // Is the current button active?
        if ( gtk_toggle_button_get_active(current_button) )
        {
            // Change the storyline variable's value
            char **header = story_var->aliases[i].header;
            story_var->value = header != NULL ? atof(header[0]) : (double)i;
            /*
                The '.header' attribute is a list of the actual values (as strings)
                that correspond to each alias (the user friendly name).
                If there is no header, then the number of the value is used.
            */
            
            // Show a message on the console if this is the debug build
            #ifdef _DEBUG
            g_message(
                "Var %llu -> %s = %s (%.0f)",
                story_var->index,
                story_var->name,
                story_var->aliases[i].description,
                story_var->value
            );
            #endif
            
            // Exit the function when the active button of the group was found
            return;
        }

        // If the button is not active, move to the next one on the group
        my_group = my_group->next;
        current_button = GTK_TOGGLE_BUTTON(my_group->data);
    }
}

// Set a storyline variable's value when a No/Yes radio button was clicked
// These kind of buttons are used as a fallback when the variable does not fall in the other types
void hb_setvar_no_yes(GtkRadioButton* widget, StorylineVars *story_var)
{
    // Exit the function if the editor is currently loading a file
    if (is_loading_file) return;
    
    // Is the button that triggered the 'toggled' event active?
    gboolean button_is_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    if (!button_is_active) return;

    // Get the group that the button is part of
    GSList *my_group = gtk_radio_button_get_group(widget);
    GtkToggleButton *current_button = GTK_TOGGLE_BUTTON(my_group->data);

    // Get whether the first button of the group is active
    // (Since in this case the group has necessarily two buttons,
    //  then we only need to check one of them.)
    if ( gtk_toggle_button_get_active(current_button) )
    {
        // Set the storyline variable's value to "True"
        story_var->value = 1.0;
    }
    else
    {
        // Set the storyline variable's value to "False"
        story_var->value = 0.0;
    }

    // Show a message on the console if this is the debug build
    #ifdef _DEBUG
    g_message(
        "Var %llu -> %s = %s (%.0f)",
        story_var->index,
        story_var->name,
        (story_var->value == 0.0 ? "No" : "Yes"),
        story_var->value
    );
    #endif
}

// Set a storyline variable's value after its text box is edited
void hb_setvar_text_entry(GtkEntry *widget, StorylineVars *story_var)
{
    // Exit the function if the editor is currently loading a file
    if (is_loading_file) return;
    
    // Get the text from the widget
    const char *my_text = gtk_entry_get_text(widget);

    // Copy the text to the temporary buffer (since the widget's text should not be edited directly)
    strncpy(text_entry_buffer, my_text, sizeof(text_entry_buffer));

    // Remove the non-digit characters
    hb_text_filter_natural(text_entry_buffer, sizeof(text_entry_buffer) - 1);

    // Limit the value to the variable's maximum (if there is a maximum)
    double new_value;
    if (gtk_entry_get_text_length(widget) > 0)
    {
        new_value = atof(text_entry_buffer);
        if (story_var->maximum > 0.0)
        {
            new_value = (new_value <= story_var->maximum) ? new_value : story_var->maximum;
            snprintf(text_entry_buffer, sizeof(text_entry_buffer), "%.0f", new_value);
        }
    }
    else
    {
        new_value = 0.0;
    }
    
    // Set the filtered text to the widget
    g_signal_handlers_block_by_func(widget, G_CALLBACK(hb_setvar_text_entry), story_var);   // Prevents the 'changed' signal from being emitted recursively
    gtk_entry_set_text(widget, text_entry_buffer);
    g_signal_handlers_unblock_by_func(widget, G_CALLBACK(hb_setvar_text_entry), story_var);

    // Modify the storyline variable
    story_var->value = new_value;

    // Show a message on the console if this is the debug build
    #ifdef _DEBUG
    g_message(
        "Var %llu -> %s = %.0f",
        story_var->index,
        story_var->name,
        story_var->value
    );
    #endif
}

// Filter the non numeric characters out of a string.
// In other words, ensure that the string represents a natural number (positive integer).
void hb_text_filter_natural(char *text, size_t max_length)
{
    // Get the amount of characters on the string
    size_t my_length = strnlen_s(text, max_length);
    if (my_length < 0 || my_length > max_length) return;

    // Position on the new string
    size_t pos = 0;

    // Copy the digits to the begining of the string
    for (size_t i = 0; i < my_length; i++)
    {
        if ( isdigit(text[i]) )
        {
            // Advance the destination position if the character is a digit
            text[pos++] = text[i];
        }
    }

    // Add a null terminator to the string
    text[pos] = '\0';
}

// Make the dropdown list to show when clicked for the first time
// For some reason, it only shows when clicked for the second time, perhaps because that it has several items.
// So this call function is called once after the first time the widget is drawn, to force the dropdown to open then close.
// This way when the user click on the list, then it will show normally.
void hb_dropdown_list_fix(GtkComboBox *widget)
{
    gtk_combo_box_popup(widget);    // Display the dropdown list
    gtk_combo_box_popdown(widget);  // Close the dropdown list
    
    // Note: An warning is shown on terminal when connecting to the 'draw' signal, but I found no way around that warning.
    //       Fortunately, that warning does not prevent the program from working, and it does not show on the release build
    //       (that does not have a terminal). I could not find a way of doing the workaround without connecting to 'draw'.
    g_message(
        "Please ignore the above warning. It is a product of a workaround that I, the developer, "
        "had to do in order to get the dropdown list to display when clicked for the first time."
    );
    
    // Disconnect from this callback so it does not execute more than once
    g_signal_handlers_disconnect_by_func(widget, G_CALLBACK(hb_dropdown_list_fix), NULL);
}

// Store the pointers for the text entries of the player's coordinates,
// so the dropdown list can change them when a room is selected.
void hb_bind_xy_entries(GtkEntry *x, GtkEntry *y)
{
    if (x != NULL) x_entry = x;
    if (y != NULL) y_entry = y;
}

// Store the pointers for the text entries of the player's hit points,
// so they can interact with each other (current HP capped to the maximum HP)
void hb_bind_hp_entries(GtkEntry *current, GtkEntry *maximum)
{
    if (current != NULL) hp_cur_entry = current;
    if (maximum != NULL) hp_max_entry = maximum;
}

// Store the pointers to the Rooms List, Known Glyphs, and Game Seed,
// so they can be updated when a new file is loaded.
void hb_bind_widgets(GtkComboBox *room_widget, GSList *glyph_widget, GtkEntry *seed_widget)
{
    if (room_widget != NULL) room_dropdown_list = room_widget;
    if (glyph_widget != NULL) known_glyphs_group = glyph_widget;
    if (seed_widget != NULL) game_seed_entry = seed_widget;
}

// Update the Room ID variable, that stores the current room.
// Also update the coordinates field when the room is changed,
// so the player do not end up stuck out of bounds or in a wall.
void hb_set_coordinates_from_room(GtkComboBoxText *widget)
{
    // Exit the function if the editor is currently loading a file
    if (is_loading_file) return;
    
    char *room_name = gtk_combo_box_text_get_active_text(widget);
    HeartboundRoom *my_room = hb_get_room(room_name);
    
    // Set the Room ID variable
    snprintf(hb_room_id, ROOM_NAME_SIZE-1, room_name);

    // Print a debug message when the Room ID changes
    #ifdef _DEBUG
    g_message("Room = %s", hb_room_id);
    #endif

    // Update the XY coordinates to the room's spaw point
    if (my_room != NULL)
    {
        // Get the XY coordinates for the room
        double x_coord = my_room->x;
        double y_coord = my_room->y;

        // Set the X entry
        snprintf(text_entry_buffer, sizeof(text_entry_buffer), "%.0f", x_coord);
        gtk_entry_set_text(x_entry, text_entry_buffer);

        // Set the Y entry
        snprintf(text_entry_buffer, sizeof(text_entry_buffer), "%.0f", y_coord);
        gtk_entry_set_text(y_entry, text_entry_buffer);
    }
}

// Filter out from a string characters that aren't numbers or signs.
// That is, ensure the resulting string represents a positive or negative integer.
void hb_text_filter_integer(char *text, size_t max_length)
{
    // Get the amount of characters on the string
    size_t my_length = strnlen_s(text, max_length);
    if (my_length < 0 || my_length > max_length) return;

    // Position on the new string
    size_t pos = 0;

    // Copy the digits to the begining of the string
    for (size_t i = 0; i < my_length; i++)
    {
        if ( isdigit(text[i]) || text[i] == '+' || text[i] == '-' )
        {
            // Only accept a sign if it is the first character of the new string
            if ( (text[i] == '+' || text[i] == '-') && (pos != 0) ) continue;
            
            // Advance the destination position if the character is a digit
            text[pos++] = text[i];
        }
    }

    // Add a null terminator to the string
    text[pos] = '\0';
}

// Set one of the variables that hold the player's attributes
// (coordinates or hitpoints)
void hb_setvar_player_attribute(GtkEntry *widget, double *attribute)
{
    // Exit the function if the editor is currently loading a file
    if (is_loading_file) return;
    
    // Prevents the the current callback function from being called recursively when it changes the text entry
    g_signal_handlers_block_by_func(widget, G_CALLBACK(hb_setvar_player_attribute), attribute);
    
    // Get the text from the widget
    const char *my_text = gtk_entry_get_text(widget);

    // Copy the text to the temporary buffer (since the widget's text should not be edited directly)
    strncpy(text_entry_buffer, my_text, sizeof(text_entry_buffer));

    // Remove the non-digit characters
    if (attribute == &hb_x_axis || attribute == &hb_y_axis)
    {
        // Accept signs if it is one of the coordinates' fields
        hb_text_filter_integer(text_entry_buffer, sizeof(text_entry_buffer) - 1);
    }
    else
    {
        // Only accepts digits for the other fields
        hb_text_filter_natural(text_entry_buffer, sizeof(text_entry_buffer) - 1);
    }

    // Update the text entry with the filtered text
    gtk_entry_set_text(widget, text_entry_buffer);

    // Set the value of the attribute
    if (gtk_entry_get_text_length(widget) > 0)
    {
        // Convert the text to float (if there is any text) and set it to the attribute
        *attribute = atof(text_entry_buffer);

        // In case the entry field is of the current HP
        if ( attribute == &hb_hitpoints_current || attribute == &hb_hitpoints_maximum )
        {
            // Check if the current HP is not bigger than the maximum HP
            if (hb_hitpoints_current > hb_hitpoints_maximum)
            {
                // Set the current HP variable and its field to the maximum value
                hb_hitpoints_current = hb_hitpoints_maximum;
                snprintf(text_entry_buffer, sizeof(text_entry_buffer), "%.0f", hb_hitpoints_current);
                gtk_entry_set_text(hp_cur_entry, text_entry_buffer);
            }
        }
    }
    else
    {
        // Set the attribute to zero if there is not any text
        *attribute = 0.0;

        // Set the current HP to zero when there is no text on the maximum HP field
        if ( attribute == &hb_hitpoints_maximum )
        {
            hb_hitpoints_current = 0.0;
            gtk_entry_set_text(hp_cur_entry, "");   // Erase the text on the current HP field
        }
    }

    // Re-enable the current callback function
    g_signal_handlers_unblock_by_func(widget, G_CALLBACK(hb_setvar_player_attribute), attribute);

    // Print a debug message when the attribute changes
    #ifdef _DEBUG

    char *attr_name;

    if (attribute == &hb_x_axis)
    {
        attr_name = "X-axis";
    }
    else if (attribute == &hb_y_axis)
    {
        attr_name = "Y-axis";
    }
    else if (attribute == &hb_hitpoints_current)
    {
        attr_name = "Current Hit Points";
    }
    else if (attribute == &hb_hitpoints_maximum)
    {
        attr_name = "Maximum Hit Points";
    }
    else
    {
        g_message("No valid variable was associated to the field you are trying to change.");
        return;
    }

    g_message("%s = %0.f", attr_name, *attribute);

    #endif
}

// Set the radio button of the 'known glyphs' choice
void hb_setvar_known_glyphs(GtkRadioButton *widget, double *known_glyphs)
{
    // Exit the function if the editor is currently loading a file
    if (is_loading_file) return;
    
    // Is the button that triggered the 'toggled' event active?
    gboolean button_is_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    if (!button_is_active) return;

    // Get the first button of the group
    GSList *group = gtk_radio_button_get_group(widget);
    GtkToggleButton *current_button = GTK_TOGGLE_BUTTON(group->data);

    // Loop through the 3 buttons to fing which one is active
    for (size_t value = 2; value >= 0; value--)
    {
        if (gtk_toggle_button_get_active(current_button) == TRUE)
        {
            // Set the known glyphs variable to the value
            *known_glyphs = value;

            // Print a message on the debug build
            #ifdef _DEBUG
            char *glyph_labels[3] = {"None", "Guardian", "Guardian and Darksider"};
            g_message(
                "Known glyphs = %s (%.0f)",
                glyph_labels[(size_t)(*known_glyphs)],
                *known_glyphs
            );
            #endif
            
            // Exit the function once the active button was found
            return;
        }
        
        // Move to the next button on the
        group = group->next;
        if (group == NULL) break;
        current_button = GTK_TOGGLE_BUTTON(group->data);
    }
}

// Set the game seed's variable when its field changes
void hb_setvar_game_seed(GtkEntry *widget,  char *game_seed)
{
    // Exit the function if the editor is currently loading a file
    if (is_loading_file) return;
    
    // Get the text from the widget
    const char *my_text = gtk_entry_get_text(widget);

    // Copy the text to the buffer
    strncpy(text_entry_buffer, my_text, sizeof(text_entry_buffer));

    // Filter out the non-digits characters
    hb_text_filter_natural(text_entry_buffer, SEED_SIZE - 2);

    // Copy the text to the game seed variable
    if ( strnlen(text_entry_buffer, SEED_SIZE-2) > 0 )
    {
        strncpy(game_seed, text_entry_buffer, SEED_SIZE - 1);
    }
    else
    {
        hb_random_seed(game_seed);
    }

    // Update the widget with the text
    g_signal_handlers_block_by_func(widget, G_CALLBACK(hb_setvar_game_seed), game_seed);
    gtk_entry_set_text(widget, text_entry_buffer);
    g_signal_handlers_unblock_by_func(widget, G_CALLBACK(hb_setvar_game_seed), game_seed);

    // Print a debug message when the variable changes
    #ifdef _DEBUG
    g_message("Game Seed = %s", game_seed);
    #endif
}

// Generate a random game seed (9 numeric digits)
void hb_random_seed(char *game_seed)
{
    // These variables are remembered between different function calls
    static bool random_initialized = false;  // If we have initialized the variables
    static double rand_size;                 // The amount of bits of the number generated by 'rand()'
    static double iterations;                // How many iterations to generate our random game seed

    // Initialize the variables, if they weren't already
    if (!random_initialized)
    {
        // Use the current time as the seed for the 'rand()' function
        srand(time(NULL));

        // Get the size (in bits) of each generated pseudo-random number
        rand_size = round( log(RAND_MAX) / log(2.0) );
        
        // How many iterations to generate a number of at least 64-bit
        iterations = ceil( 64.0 / rand_size );
        
        // Flag as 'initialized'
        random_initialized = true;
    }

    // Where to store the partial results
    uint64_t accumulator = 0;

    // Generate a 64-bit pseudo-random number
    for (size_t i = 0; i < iterations; i++)
    {
        accumulator <<= (uint64_t)rand_size;    // Shift the accumulator to the left by the size of the number generated by 'rand()'
        accumulator |= rand();                  // Bitwise 'Or' to add the generated number
    }
    
    // Get the last 9 decimal digits of our 64-bit number and use them as the game seed
    uint64_t new_seed = accumulator % (uint64_t)pow(10, SEED_SIZE - 1);
    snprintf(game_seed, SEED_SIZE-1, "%llu", new_seed);
}

// Highlight a menu item when the mouse pointer is over the item
void hb_menu_hover(GtkMenuItem *widget, GdkEventCrossing event, void *data)
{
    switch (event.type)
    {
    case GDK_ENTER_NOTIFY:
        // Select when the mouse enters the item
        gtk_menu_item_select(widget);
        break;
    
    case GDK_LEAVE_NOTIFY:
        // Select when the mouse leaves the item
        gtk_menu_item_deselect(widget);
        break;
    }
}

// A wrapper for the 'hb_write_save()' function from 'hb_gui_save_io.c'
// This function is called when the save option is left-clicked on the interface.
void hb_save_file(GtkMenuItem *widget, GdkEventButton event, GtkWindow *window)
{
    // If the left mouse button was pressed
    if ( event.type == GDK_BUTTON_PRESS && event.button == 1 )
    {
        // Save the file to disk
        int status = hb_write_save();

        if (status == 0)
        {
            // Show for 2.6 seconds the indicator that the file was saved
            gtk_label_set_text(GTK_LABEL(file_indicator), FILE_SAVED_MESSAGE);
            gtk_widget_show(file_indicator);
            g_timeout_add(INDICATOR_TIMEOUT, G_SOURCE_FUNC(hb_hide_file_indicator), NULL);

            #ifdef _DEBUG
            g_message("Saved: %s", CURRENT_FILE);
            #endif
        }
        else
        {
            // File saving failed

            // Create the text of the error dialog
            char *message = calloc(TEXT_BUFFER_SIZE, sizeof(char));
            snprintf(
                message,
                TEXT_BUFFER_SIZE,
                "Coult not write to:\n%s\n\n"
                "The file is already in use by another program.",
                CURRENT_FILE
            );

            // Display the error dialog
            GtkWidget *error_dialog = hb_create_dialog_with_title_and_image(
                window,
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "File saving error",
                "dialog-error",
                message
            );
            
            // Deallocate the message buffer
            free(message);

            // Display the dialog
            gtk_dialog_run(GTK_DIALOG(error_dialog));

            // Destroy the dialog
            gtk_widget_destroy(error_dialog);
        }
    }
}

// Load a Heartbound save file into the editor
// If a NULL pointer is provided as the "widget", then the main window contents are not updated.
// (That is the case when the program failed to find a save on startup, and ask if the user wants to open another save.)
void hb_open_file(GtkMenuItem *widget, GdkEventButton event, GtkWindow *window)
{
    // Create the file dialog
    GtkFileChooserNative *file_chooser = gtk_file_chooser_native_new(
        "Open another Heartbound save", // Title of the dialog
        window,                         // Parent of the dialog (the main window)
        GTK_FILE_CHOOSER_ACTION_OPEN,   // Open file action
        NULL,                           // Text of the "Open" button
        NULL                            // Text of the "Cancel" button
    );

    // Set the dialog to the folder where Heartbound saves the game
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(file_chooser), SAVE_ROOT);

    // Create a filter for the file dialog to show "*.thor" files (the format of the Heartbound save)
    GtkFileFilter *file_filter_thor = gtk_file_filter_new();
    gtk_file_filter_add_pattern(file_filter_thor, "*.thor");
    gtk_file_filter_set_name(file_filter_thor, "Heartbound save");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser), file_filter_thor);

    // Create a filter to show all files
    GtkFileFilter *file_filter_all = gtk_file_filter_new();
    gtk_file_filter_add_pattern(file_filter_all, "*");
    gtk_file_filter_set_name(file_filter_all, "All files");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser), file_filter_all);

    // Set the "*.thor" filter as the default
    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(file_chooser), file_filter_thor);

    // Display the dialog
    gint status = gtk_native_dialog_run(GTK_NATIVE_DIALOG(file_chooser));

    // Handle the file, if the user chose one
    if (status == GTK_RESPONSE_ACCEPT)
    {
        char *file_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser));

        int status = hb_read_save(file_name);

        // Update the variable's widgets
        // (if the file is valid and the action is coming from the main window)
        if (status == SAVE_FILE_IS_VALID && widget != NULL)
        {
            is_loading_file = true;
            char *restrict text_buffer = malloc( TEXT_BUFFER_SIZE * sizeof(char) );
            
            for (size_t var = 0; var < NUM_STORY_VARS; var++)
            {
                if (!hb_save_data[var].used) continue;

                if (hb_save_data[var].num_entries == 0 && (hb_save_data[var].unit != NULL || hb_save_data[var].maximum > 0.0))
                {
                    // Text field
                    GtkEntry *text_entry = hb_save_data[var].widget.entry;
                    snprintf(text_buffer, TEXT_BUFFER_SIZE, "%.0f", hb_save_data[var].value);
                    gtk_entry_set_text(text_entry, text_buffer);
                }
                else if (hb_save_data[var].num_entries >= 2)
                {
                    // Group of customized radio buttons
                    GSList *group = hb_save_data[var].widget.group;
                    GtkToggleButton *current_button = GTK_TOGGLE_BUTTON(group->data);

                    // Loop through the buttons in the group
                    for (size_t i = hb_save_data[var].num_entries - 1; i >= 0; i--)
                    {
                        // The header is a list of all possible values (as strings)
                        // If there isn't a header, the number of the button is considered to be the value
                        char **header = hb_save_data[var].aliases[i].header;
                        double header_value = (header != NULL) ? atof(header[0]) : (double)i;

                        // Check if the header's value correspond to the storyline variable's value
                        if ( header_value == hb_save_data[var].value )
                        {
                            // Set the button to active and exit the loop
                            gtk_toggle_button_set_active(current_button, TRUE);
                            break;
                        }

                        // Go to the next button in the group
                        group = group->next;
                        if (group == NULL) break;
                        current_button = GTK_TOGGLE_BUTTON(group->data);
                    }
                }
                else
                {
                    // Group generic Yes/No radio buttons
                    GSList *group = hb_save_data[var].widget.group;
                    GtkToggleButton *current_button = GTK_TOGGLE_BUTTON(group->data);

                    if (hb_save_data[var].value == 1)
                    {
                        gtk_toggle_button_set_active(current_button, TRUE);
                    }
                    else
                    {
                        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(group->next->data), TRUE);
                    }
                }
            }

            // Update the player attributes fields
            
            // Room selection list
            HeartboundRoom *current_room = hb_get_room(hb_room_id);
            if (current_room != NULL)
            {
                gtk_combo_box_set_active(room_dropdown_list, current_room->index);
            }

            // Coordinates fields
            snprintf(text_buffer, TEXT_BUFFER_SIZE, "%.0f", hb_x_axis);
            gtk_entry_set_text(x_entry, text_buffer);
            snprintf(text_buffer, TEXT_BUFFER_SIZE, "%.0f", hb_y_axis);
            gtk_entry_set_text(y_entry, text_buffer);

            // Hit Points fields
            snprintf(text_buffer, TEXT_BUFFER_SIZE, "%.0f", hb_hitpoints_current);
            gtk_entry_set_text(hp_cur_entry, text_buffer);
            snprintf(text_buffer, TEXT_BUFFER_SIZE, "%.0f", hb_hitpoints_maximum);
            gtk_entry_set_text(hp_max_entry, text_buffer);

            // Glyph radio buttons
            size_t button_index = (size_t)(2.0 - hb_known_glyphs);
            GSList *group = known_glyphs_group;
            GtkToggleButton *current_button = GTK_TOGGLE_BUTTON(group->data);

            for (size_t i = 0; i < button_index; i++)
            {
                // Navigate until the button that corresponds to the value of 'hb_known_glyphs'
                group = group->next;
                current_button = GTK_TOGGLE_BUTTON(group->data);
            }
            
            gtk_toggle_button_set_active(current_button, TRUE);

            // Game Seed field
            strncpy(text_buffer, hb_game_seed, sizeof(hb_game_seed));
            gtk_entry_set_text(game_seed_entry, text_buffer);

            // Place the file name on the window title, if the file isn't the default save file
            if (strncmp(SAVE_PATH, CURRENT_FILE, PATH_BUFFER) != 0)
            {
                snprintf(text_buffer, TEXT_BUFFER_SIZE, "%s - %s", CURRENT_FILE, WINDOW_TITLE);
                gtk_window_set_title(window, text_buffer);
            }
            else
            {
                gtk_window_set_title(window, WINDOW_TITLE);
            }
            
            // Deallocate the text buffer
            free(text_buffer);

            #ifdef _DEBUG
            g_message("Loaded: %s", file_name);
            #endif

            // Show for 2.6 seconds the indicator that the file was loaded
            gtk_label_set_text(GTK_LABEL(file_indicator), FILE_LOADED_MESSAGE);
            gtk_widget_show(file_indicator);
            g_timeout_add(INDICATOR_TIMEOUT, G_SOURCE_FUNC(hb_hide_file_indicator), NULL);

            is_loading_file = false;
        }
        else if (status != SAVE_FILE_IS_VALID)
        {
            // File loading failed

            // Create the text of the error dialog
            char *message = calloc(TEXT_BUFFER_SIZE, sizeof(char));
            snprintf(message, TEXT_BUFFER_SIZE, "The file you tried to open is not a valid Heartbound save:\n\n%s", file_name);

            // Display the error dialog
            GtkWidget *error_dialog = hb_create_dialog_with_title_and_image(
                window,
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "File loading error",
                "dialog-error",
                message
            );
            
            // Deallocate the message buffer
            free(message);

            // Display the dialog
            gtk_dialog_run(GTK_DIALOG(error_dialog));

            // Destroy the dialog
            gtk_widget_destroy(error_dialog);
        }
        
        // Deallocate the memory of the file's name
        g_free(file_name);
    }

    // Destroy the file chooser dialog
    g_object_unref(file_chooser);
}

// Ask the user what to do if the program could not load the default save during startup
void hb_failed_to_open_default_save(GtkWindow *main_window)
{
    #define CREATE_NEW_SAVE 0
    #define OPEN_ANOTHER_SAVE 1
    #define DOWNLOAD_LATEST_VERSION 2
    #define CLOSE_PROGRAM 3
    
    GtkWidget *warning_dialog = hb_create_dialog_with_title_and_image(
        main_window,
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_NONE,
        "Heartbound Save Editor",
        "dialog-warning",
        "Could not find a valid Heartbound save file at the default location.\n\n"
        "Maybe you have not played the game yet or this program is outdated.\n\n"
        "What do you want to do?"
    );

    gtk_dialog_add_buttons(
        GTK_DIALOG(warning_dialog),
        "Create a new save",
        CREATE_NEW_SAVE,
        "Open another save",
        OPEN_ANOTHER_SAVE,
        "Download the latest version",
        DOWNLOAD_LATEST_VERSION,
        "Close program",
        CLOSE_PROGRAM,
        NULL
    );

    g_signal_connect(GTK_DIALOG(warning_dialog), "response", G_CALLBACK(hb_failed_to_open_default_save_response), main_window);

    // Display the dialog
    gtk_dialog_run(GTK_DIALOG(warning_dialog));

    // Destroy the dialog
    gtk_widget_destroy(warning_dialog);
}

// Handle the user's response to 'hb_failed_to_open_default_save()'
void hb_failed_to_open_default_save_response(GtkDialog dialog, gint response_id, GtkWindow *main_window)
{
    switch (response_id)
    {
        case CREATE_NEW_SAVE:
            g_mkdir_with_parents(SAVE_ROOT, 755);   // Create the save directory if it does not exist
            hb_create_default_save(SAVE_PATH);
            hb_open_save(SAVE_PATH);
            break;
        
        case OPEN_ANOTHER_SAVE:
            // Open the file chooser to select another save file
            hb_open_file(NULL, (GdkEventButton){}, main_window);
            if (!hb_save_is_open) break;
            
            // Update the name of the main window if the save file is not the default one
            char *restrict text_buffer = calloc(TEXT_BUFFER_SIZE, sizeof(char));
            if (strncmp(SAVE_PATH, CURRENT_FILE, PATH_BUFFER) != 0)
            {
                snprintf(text_buffer, TEXT_BUFFER_SIZE, "%s - %s", CURRENT_FILE, WINDOW_TITLE);
                gtk_window_set_title(main_window, text_buffer);
            }
            else {gtk_window_set_title(main_window, WINDOW_TITLE);}
            free(text_buffer);

            break;
        
        case DOWNLOAD_LATEST_VERSION:
            // Open a browser window to the download page, and exit the Save Editor
            ShellExecuteA(NULL, "open", "https://github.com/tbpaolini/Heartbound-Save-Editor/releases", NULL, NULL, SW_SHOWNORMAL);
            exit(EXIT_SUCCESS);
            break;
        
        case CLOSE_PROGRAM:
            exit(EXIT_SUCCESS);  // Signaling as "success" because it comes from an user's choice
            break;
        
        default:
            // Close the program if no choice was made
            // (the user just closed the dialog window by clicking "X" at the top )
            exit(EXIT_FAILURE);
    }

    #undef CREATE_NEW_SAVE
    #undef OPEN_ANOTHER_SAVE
    #undef DOWNLOAD_LATEST_VERSION
    #undef CLOSE_PROGRAM
}

// Create a save file with the default values
bool hb_create_default_save(char *path)
{
    FILE *save_file = fopen(path, "w");
    if (save_file == NULL) return false;
    
    char random_seed[SEED_SIZE];
    hb_random_seed(random_seed);

    fprintf(save_file, "%s\n", random_seed);
    fprintf(save_file, "home_bedroom\n");
    fprintf(save_file, "%.0f \n", 0.0);
    fprintf(save_file, "%.0f \n", 0.0);
    fprintf(save_file, "%.0f \n", 10.0);
    fprintf(save_file, "%.0f \n", 10.0);
    fprintf(save_file, "%.0f \n", 0.0);

    for (size_t var = 1; var < NUM_STORY_VARS; var++)
    {
        fprintf(save_file, "%.0f \n", hb_save_data[var].def);
    }
    
    fclose(save_file);
    return true;
}

// Display a message dialog with a custom image and title
GtkWidget *hb_create_dialog_with_title_and_image(
    GtkWindow *parent,
    GtkDialogFlags flags,
    GtkMessageType type,
    GtkButtonsType buttons, 
    const char *title,
    const char *image_icon_name,
    const char *message
)
{
    // Create the dialog
    GtkWidget *dialog = gtk_message_dialog_new(
        parent,
        flags,
        type,
        buttons,
        NULL
    );

    // Create an image to the dialok
    GtkWidget *image = gtk_image_new_from_icon_name(image_icon_name, GTK_ICON_SIZE_DIALOG);
    gtk_widget_set_halign(image, GTK_ALIGN_START);

    // Create the text of the dialog
    GtkWidget *text = gtk_label_new(message);
    gtk_widget_set_halign(text, GTK_ALIGN_START);
    
    // Add the image and text to the dialog
    // Note: I have to add them manually because GTK does not include an image on dialogs
    GtkWidget *content_area = gtk_message_dialog_get_message_area(GTK_MESSAGE_DIALOG(dialog));
    GtkWidget *wrapper = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(wrapper), image);
    gtk_container_add(GTK_CONTAINER(wrapper), text);
    gtk_container_add(GTK_CONTAINER(content_area), wrapper);
    gtk_widget_show_all(dialog);

    // Add title to the dialog
    gtk_window_set_title(GTK_WINDOW(dialog), title);

    return dialog;
}

// Bind the pointer of the "file loaded" indicator, so it can be shown or hidden.
void hb_bind_file_indicator(GtkWidget *widget)
{
    if (widget != NULL) file_indicator = widget;
}

// Hide the "file loaded" indicator
gboolean hb_hide_file_indicator()
{
    // Hide the widget from the window
    gtk_widget_hide(file_indicator);
    
    // Returning this value makes the function to be called only once if called with a "g_timeout_add()"
    return G_SOURCE_REMOVE;
}

// Make the widgets on the notebook to be clickable after the menu items have been used.
// Without this fix, if one clicks on the menu, then tries to click on something on
// the notebook, it would be necessary to click twice to interact with the notebook.
void hb_notebook_fix(GtkNotebook *widget, GdkEventCrossing event, void *data)
{
    // When the mouse enters the notebook
    if (event.type == GDK_ENTER_NOTIFY)
    {
        // "Switch" to the same page on the notebook to force it to get focus
        gint page = gtk_notebook_get_current_page(widget);
        gtk_notebook_set_current_page(widget, page);
    }
}