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
#include <gtk/gtk.h>
#include <windows.h>
#include <hb_save.h>
#include <hb_gui_callback.h>

// If the editor is configured for automatically reloading the save file
bool hb_automatic_reloading = false;

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
            hb_flag_data_as_changed(GTK_WIDGET(widget));    // Flag the editor's data as unsaved
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

    // Flag the editor's data as unsaved
    hb_flag_data_as_changed(GTK_WIDGET(widget));

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

    // Flag the editor's data as unsaved
    hb_flag_data_as_changed(GTK_WIDGET(widget));

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

    // Flag the editor's data as unsaved
    hb_flag_data_as_changed(GTK_WIDGET(widget));

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
            hb_flag_data_as_changed(GTK_WIDGET(widget));    // Flag the editor's data as unsaved
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

    // Flag the editor's data as unsaved
    hb_flag_data_as_changed(GTK_WIDGET(widget));

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

        if (status == FILE_SAVING_SUCCESS)
        {
            // Show for 2.6 seconds the indicator that the file was saved
            gtk_label_set_text(GTK_LABEL(file_indicator), FILE_SAVED_MESSAGE);
            gtk_widget_show(file_indicator);
            g_timeout_add(INDICATOR_TIMEOUT, G_SOURCE_FUNC(hb_hide_file_indicator), NULL);

            // Update the window's title with the path of the saved file
            hb_update_window_title(window);

            // Flag the editor's data as saved
            has_unsaved_data = false;

            // After a new file was saved, we can be sure that the path is absolute
            // So no need to take the loader into consideration
            using_loader = false;

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
    if (event.button != 1) return;    // Return if the pressed button is not the left mouse button

    // Check if the data has changed
    bool proceed = hb_check_if_data_changed("Confirm opening a file", window);
    if (!proceed) return;   // Return if the user has chosen to cancel
    
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
            hb_load_data_into_interface(window);
            
            #ifdef _DEBUG
            g_message("Loaded: %s", file_name);
            #endif
        }
        else if (status != SAVE_FILE_IS_VALID)
        {
            // File loading failed

            // Create the text of the error dialog
            char *restrict message = calloc(TEXT_BUFFER_SIZE, sizeof(char));
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
        
        // Add the path of the loaded file to the window's title
        if (status == SAVE_FILE_IS_VALID) hb_update_window_title(window);
        
        // Deallocate the memory of the file's name
        g_free(file_name);
    }

    // Destroy the file chooser dialog
    g_object_unref(file_chooser);
}

// Updates the user interface with the values on the data structure of the save file
void hb_load_data_into_interface(GtkWindow *window)
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
    
    // Deallocate the text buffer
    free(text_buffer);

    // Show for 2.6 seconds the indicator that the file was loaded
    gtk_label_set_text(GTK_LABEL(file_indicator), FILE_LOADED_MESSAGE);
    gtk_widget_show(file_indicator);
    g_timeout_add(INDICATOR_TIMEOUT, G_SOURCE_FUNC(hb_hide_file_indicator), NULL);

    // Flag the editor's data as saved
    has_unsaved_data = false;

    is_loading_file = false;
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
            hb_create_default_save(SAVE_PATH, main_window);
            hb_open_save(SAVE_PATH);
            break;
        
        case OPEN_ANOTHER_SAVE:
            // Open the file chooser to select another save file
            hb_open_file(NULL, (GdkEventButton){.type = GDK_BUTTON_PRESS, .button = 1}, main_window);
            if (!hb_save_is_open) break;
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
bool hb_create_default_save(char *path, GtkWindow *window)
{
    // Open the file for writing
    GFile *save_file = g_file_new_for_path(path);
    GOutputStream *output = G_OUTPUT_STREAM(g_file_replace(save_file, NULL, FALSE, G_FILE_CREATE_NONE, NULL, NULL));
    if (output == NULL)
    {
        // Failed to save file
        g_object_unref(save_file);

        // Display an error dialog

        char *restrict text_buffer = calloc(TEXT_BUFFER_SIZE, sizeof(char));
        if (text_buffer == NULL) return false;

        // Create the error message using the file path
        snprintf(
            text_buffer,
            TEXT_BUFFER_SIZE,
            "Could not create a new save file at:\n%s\n\n"
            "Maybe you do not have permission to write there or some other program already has that file open.",
            path
        );
        
        // Create and display the error dialog
        GtkWidget *error_dialog = hb_create_dialog_with_title_and_image(
            window,
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "File creation error",
            "dialog-error",
            text_buffer
        );

        gtk_dialog_run(GTK_DIALOG(error_dialog));

        // Garbage collection
        gtk_widget_destroy(error_dialog);
        free(text_buffer);

        return false;
    }
    
    // Generate a random game seed
    char random_seed[SEED_SIZE];
    hb_random_seed(random_seed);

    // Buffer for writing the lines of the save file
    char *restrict line_buffer = calloc(SAVE_LINE_BUFFER, sizeof(char));
    #define WRITE_LINE(format, value) \
        snprintf(line_buffer, SAVE_LINE_BUFFER, format, value);\
        g_output_stream_write(output, line_buffer, strlen(line_buffer), NULL, NULL)

    // Write the player's attributes to file
    WRITE_LINE("%s\n", random_seed);    // Game seed
    WRITE_LINE("%s\n", "home_bedroom"); // Starting room
    WRITE_LINE("%.0f \n", 0.0);         // X coordinate
    WRITE_LINE("%.0f \n", 0.0);         // Y coordinate
    WRITE_LINE("%.0f \n", 10.0);        // Current HP
    WRITE_LINE("%.0f \n", 10.0);        // Maximum HP
    WRITE_LINE("%.0f \n", 0.0);         // Known glyphs

    // Write the storyline variables
    for (size_t var = 1; var < NUM_STORY_VARS; var++)
    {
        WRITE_LINE("%.0f \n", hb_save_data[var].def);
    }
    
    // Close the save file
    free(line_buffer);
    g_output_stream_close(output, NULL, NULL);
    g_object_unref(output);
    g_object_unref(save_file);

    #undef WRITE_LINE
    
    // File saved successfully
    #ifdef _DEBUG
    g_message("Created: %s", path);
    #endif
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

// Update the title of the window with the path of the save file
void hb_update_window_title(GtkWindow *window)
{
    char *restrict text_buffer = calloc(TEXT_BUFFER_SIZE, sizeof(char));
    snprintf(text_buffer, TEXT_BUFFER_SIZE, "%s - %s", CURRENT_FILE, WINDOW_TITLE);
    gtk_window_set_title(window, text_buffer);
    free(text_buffer);
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

// Scroll the notebook's contents with the keyboard (Page Up/Down and arrows)
void hb_notebook_keyboard_scrolling(GtkScrolledWindow *widget, GdkEventKey event, GtkAdjustment *adjustment)
{
    // Get the vertical position of the page
    gdouble position = gtk_adjustment_get_value(adjustment);

    // Get the scrolling increments
    gdouble step = gtk_adjustment_get_step_increment(adjustment);  // Amount to be scrolled by the arrows (roughly a line)
    gdouble page = gtk_adjustment_get_page_increment(adjustment);  // Amount to be scrolled by the Page keys (roughly the visible area)

    // Check which key was pressed
    switch (event.keyval)
    {
        case GDK_KEY_Page_Down:
            // Scroll down a page
            gtk_adjustment_set_value(adjustment, position + page);
            break;
        
        case GDK_KEY_Page_Up:
            // Scroll up a page
            gtk_adjustment_set_value(adjustment, position - page);
            break;
        
        case GDK_KEY_Down:
            // Scroll down a line
            gtk_adjustment_set_value(adjustment, position + step);
            break;
        
        case GDK_KEY_Up:
            // Scroll up a line
            gtk_adjustment_set_value(adjustment, position - step);
            break;
        
        default:
            break;
    }
}

// Open a file that was dragged into the window
void hb_drag_and_drop_file(
    GtkWindow *window,
    GdkDragContext *context,
    gint x,
    gint y,
    GtkSelectionData *data,
    guint info,
    guint time,
    gpointer user_data
)
{
    // Get the file's URI from the selection data
    char **file_paths = gtk_selection_data_get_uris(data);
    
    // Return if there are no paths
    if (file_paths == NULL) return;

    // Strip the 'file:///' prefix and unescape the URI (notably, '%20' gets replaced by an actual space)
    char *file_name_raw = g_str_has_prefix(file_paths[0], "file:///") ? file_paths[0] + 8 : file_paths[0];
    char *file_name = g_uri_unescape_string(file_name_raw, NULL);

    // Check if there is unsaved data before proceeding
    bool can_proceed = hb_check_if_data_changed("Confirm opening a dropped file", window);
    if (can_proceed)
    {
        // Attempt to read the data from the file
        int status = hb_read_save(file_name);

        if (status == SAVE_FILE_IS_VALID)
        {
            // If the file is valid, load the data into the interface and update the window's title
            hb_load_data_into_interface(window);
            hb_update_window_title(window);
                    
            #ifdef _DEBUG
            g_message("Loaded: %s", file_name);
            #endif
        }
        else
        {
            // Display an error dialog if the file could not be read from

            char error_message[TEXT_BUFFER_SIZE];
            snprintf(
                error_message,
                sizeof(error_message),
                "The file you dropped into the window is not a valid Heartbound save:\n%s",
                file_name
            );

            GtkWidget *error_dialog = hb_create_dialog_with_title_and_image(
                window,
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "File loading error",
                "dialog-error",
                error_message
            );

            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
        }
    }

    // Garbage collection
    g_free(file_name);      // Delete a string
    g_strfreev(file_paths); // Delete an array of strings
}

// Detect if the open file has been changed by another program.
// This function is triggered when the window loses or regains focus.
void hb_file_has_changed(GtkWindow self, GdkEventFocus event, GtkWindow *window)
{
    static gint64 last_known_modified_time = 0;

    // Do nothing if automatic reloading is disabled
    if (!hb_automatic_reloading) return;
    
    // Is the window getting or losing focus?
    if (event.in)
    {
        // Window got focus
        // Check if the current modification time is newer than the last known one
        hb_save_get_modified_time();
        if (last_known_modified_time < hb_save_modification_time)
        {
            // Reload if the file has been modified
            hb_menu_edit_reload(NULL, window);
        }
    }
    else
    {
        // Window lost focus
        // Update the last known modified time
        last_known_modified_time = hb_save_modification_time;
    }
}

// ***********************
// Options of the menu bar
// ***********************

void placeholder(void* widget, void* data)
{
    printf("Clicked: %s\n", gtk_menu_item_get_label(GTK_MENU_ITEM(widget)));
}

// File > New
// Create a new Heartbound save with the default values
void hb_menu_file_new(GtkMenuItem *widget, GtkWindow *window)
{
    // Buffer to store the path
    char *new_path = calloc(PATH_BUFFER, sizeof(char));
    if (new_path == NULL) return;

    bool create_new_file = hb_save_dialog("Create a new Heartbound save", window, new_path);

    if (create_new_file)
    {
        // Check if the file exists
        gboolean file_exists = g_file_test(new_path, G_FILE_TEST_EXISTS);
        bool file_can_be_created = true;
        if (file_exists)
        {
            // If the file exists, ask the user to confirm whether to overwrite the file
            file_can_be_created = hb_confirmation(
                "Confirm new file",
                "There is already a file on that location.\n"
                "Do you want to overwrite it with the new file?",
                window
            );
        }

        // User has confirmed to overwrite or there isn't another file there
        if (file_can_be_created)
        {
            hb_create_default_save(new_path, window);   // Create the new file
            hb_read_save(new_path);                     // Load the file into memory
            hb_load_data_into_interface(window);        // Load the file's data to the editor
            
            // Update the window's title with the new file's path
            hb_update_window_title(window);

            // Display a "file created" message
            // Note: The 'hb_read_save()' function already set the timeout for the file indicator
            gtk_label_set_text(GTK_LABEL(file_indicator), FILE_CREATED_MESSAGE);
        }
    }

    free(new_path);
}

// File > Open
// Choose a Heartbound save to open
void hb_menu_file_open(GtkMenuItem *widget, GtkWindow *window)
{
    hb_open_file(
        widget,
        (GdkEventButton){.type = GDK_BUTTON_PRESS, .button = 1},
        window
    );
}

// File > Open default file
// Open the default Heartbound save
void hb_menu_file_open_default(GtkMenuItem *widget, GtkWindow *window)
{
    // Do nothing if the default file is already open
    if (strncmp(CURRENT_FILE, SAVE_PATH, PATH_BUFFER) == 0) return;
    
    // Check if the data has changed
    bool proceed = hb_check_if_data_changed("Confirm opening a file", window);
    if (!proceed) return;   // Return if the user has chosen to cancel

    // Read the default save file and update the interface
    int status = hb_read_save(SAVE_PATH);

    if (status == SAVE_FILE_IS_VALID)
    {
        hb_load_data_into_interface(window);
        hb_update_window_title(window);

        #ifdef _DEBUG
        g_message("Loaded: %s", SAVE_PATH);
        #endif
    }
    else
    {
        bool create_save = hb_confirmation(
            "File opening error",
            "There is not a valid Heartbound save file at the default location.\n\n"
            "Do you want to create a new Heartbound save there?",
            window
        );

        if (create_save)
        {
            bool success = hb_create_default_save(SAVE_PATH, window);

            if (success)
            {
                hb_load_data_into_interface(window);
                hb_update_window_title(window);
            }
        }
    }
}

// File > Save
// Save the values to the current file
void hb_menu_file_save(GtkMenuItem *widget, GtkWindow *window)
{
    hb_save_file(
        widget,
        (GdkEventButton){.type = GDK_BUTTON_PRESS, .button = 1},
        window
    );
}

// File > Save as...
// File > Save as...
void hb_menu_file_save_as(GtkMenuItem *widget, GtkWindow *window)
{
    // Buffer to store the path
    char *my_path = calloc(PATH_BUFFER, sizeof(char));
    if (my_path == NULL) return;

    // Ask the user where to save
    bool user_saved = hb_save_dialog(NULL, window, my_path);

    // If the user has chosen to save
    if (user_saved)
    {
        // Check if the file exists
        gboolean file_exists = g_file_test(my_path, G_FILE_TEST_EXISTS);
        if (file_exists)
        {
            // If the file exists, ask the user to confirm whether to overwrite the file
            bool user_confirmed = hb_confirmation(
                "Confirm save as",
                "The file already exists.\n"
                "Do you want to overwrite it?",
                window
            );
            if (!user_confirmed) return;
        }
        
        // Set the current file to the saved file
        snprintf(
            CURRENT_FILE,
            sizeof(CURRENT_FILE),
            my_path
        );

        // Write to the file
        hb_save_file(
            widget,
            (GdkEventButton){.type = GDK_BUTTON_PRESS, .button = 1},
            window
        );
    }
    
    // Free the memory of the buffer
    free(my_path);
}

// File > Save to default file
// Write the values to the default Heartbound save
void hb_menu_file_save_to_default(GtkMenuItem *widget, GtkWindow *window)
{
    snprintf(CURRENT_FILE, PATH_BUFFER, SAVE_PATH);
    
    hb_save_file(
        widget,
        (GdkEventButton){.type = GDK_BUTTON_PRESS, .button = 1},
        window
    );
}

// File > Exit
// Close the program
void hb_menu_file_exit(GtkMenuItem *widget, GtkWindow *window)
{
    // Display a confirmation dialog if there is unsaved changes
    bool can_exit = hb_check_if_data_changed("Confirm exit", window);
    
    // Close the program if there isn't any changes or the user chose to close anyways
    if (can_exit)
    {
        has_unsaved_data = false;   // Prevents the confirmation dialog from also showing because of 'gtk_window_close(window)'
        gtk_window_close(window);
    }
}

// Edit > Reload
// Load the save values from the save file, replacing the unsaved ones
void hb_menu_edit_reload(GtkMenuItem *widget, GtkWindow *window)
{
    // Check if there is unsaved changes
    bool can_proceed = hb_check_if_data_changed("Confirm reload", window);
    if (!can_proceed) return;

    // Read from the currently open file
    int status = hb_read_save(CURRENT_FILE);
    
    // Check if the file could be read from
    if (status == SAVE_FILE_IS_VALID)
    {
        // Load the file's data into the interface
        hb_load_data_into_interface(window);

        // Update the window title to remove the asterisk that means "unsaved"
        hb_update_window_title(window);
    }
    else
    {
        // Display an error dialog if the file could not be read from
        GtkWidget *error_dialog = hb_create_dialog_with_title_and_image(
            window,
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "File loading error",
            "dialog-error",
            "Could not read from the file.\n"
            "It might be that the file was deleted after it was opened by this program."
        );

        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
    }
}

// Edit > Clear
// Reset the save values to their default
void hb_menu_edit_clear(GtkMenuItem *widget, GtkWindow *window)
{
    // Check if there are unsaved changes before continuing
    bool can_proceed = hb_check_if_data_changed("Confirm reset", window);
    if (!can_proceed) return;

    //  Player attributes
    hb_random_seed(hb_game_seed);   // Generate a new random seed
    snprintf(hb_room_id, ROOM_NAME_SIZE, "home_bedroom");
    hb_x_axis = 0.0;
    hb_y_axis = 0.0;
    hb_hitpoints_current = 10.0;
    hb_hitpoints_maximum = 10.0;
    hb_known_glyphs = 0.0;

    // Storyline variables
    for (size_t var = 0; var < NUM_STORY_VARS; var++)
    {
        hb_save_data[var].value = hb_save_data[var].def;
    }
    
    // Load the values into the interface
    hb_load_data_into_interface(window);
    hb_flag_data_as_changed(GTK_WIDGET(window));
    gtk_label_set_text(GTK_LABEL(file_indicator), "Save data cleared!");
}

// Edit > Dark mode
// Switch between dark and light mode
void hb_menu_edit_dark_mode(GtkCheckMenuItem *widget, GtkCssProvider *style)
{
    // Get whether the user prefers the dark theme
    gboolean prefers_dark_theme = gtk_check_menu_item_get_active(widget);
    
    // Change the corresponding application's settings
    g_object_set(
        gtk_settings_get_default(),
        "gtk-application-prefer-dark-theme",
        prefers_dark_theme
    );

    // Change the color of the titles
    char *title_theme = prefers_dark_theme ? CSS_TITLE_DARK : CSS_TITLE_LIGHT;
    gtk_css_provider_load_from_data(style, title_theme, -1, NULL);

    // Open the settings file
    FILE *settings_ini = fopen("../etc/gtk-3.0/settings.ini", "r+");
    
    // Get the size of the file
    fseek(settings_ini, 0, SEEK_END); 
    long file_size = ftell(settings_ini);
    if (file_size > 10000000)
    {
        // Just a sanity check to prevent a huge file from being loaded
        // Maximum size: 10 MB
        fclose(settings_ini);
        return;
    }
    rewind(settings_ini);   // Return to the start of the file

    // Allocate a buffer big enough for the file
    // Note: Buffer is being initialized to all null bytes in order to prevent
    //       garbage data from being written back to the file.
    char *settings_buffer = calloc(file_size, sizeof(char));
    if (settings_buffer == NULL)
    {
        // Return if there isn't enough memory
        fclose(settings_ini);
        return;
    }
    
    // Read the file into the buffer
    fread(settings_buffer, file_size, 1, settings_ini);

    // Find the dark mode setting
    char *dark_theme = strstr(settings_buffer, "gtk-application-prefer-dark-theme=");

    if (dark_theme != NULL)
    {
        // If the setting was found, update its value
        dark_theme[34] = prefers_dark_theme ? '1' : '0';
        /* Note:
            The substrig to search for the setting is 35 characters long.
            The 'strstr()' function returns a pointer to the first character of that substring.
            So we need to move forward 34 characters in order to get to the setting's value.
            That's why we are modifying dark_theme[34]
        */

        // Write the buffer into the file
        rewind(settings_ini);
        fprintf(settings_ini, settings_buffer);
    }
    else
    {
        // If the setting was not found, append it to the file
        fclose(settings_ini);
        settings_ini = fopen("../etc/gtk-3.0/settings.ini", "a");
        fprintf(settings_ini, "\ngtk-application-prefer-dark-theme=%d", prefers_dark_theme);
    }

    // Deallocate the buffer and close the file
    free(settings_buffer);
    fclose(settings_ini);
}

// Edit > Automatic reloading
// Toggle on/off the automatic reloading of the save file
void hb_edit_automatic_reloading(GtkCheckMenuItem *widget, gpointer user_data)
{
    // Get the current value from the check button
    hb_automatic_reloading = (bool)gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));
    
    // Store the value to the configurations
    char value_string[2];
    snprintf(value_string, sizeof(value_string), "%d", hb_automatic_reloading);
    hb_config_set("automatic_reloading", value_string);

    // Show for 2.6 seconds the indicator that the setting was changed
    char *message = hb_automatic_reloading ? "Automatic reloading enabled" : "Automatic reloading disabled";
    gtk_label_set_text(GTK_LABEL(file_indicator), message);
    gtk_widget_show(file_indicator);
    g_timeout_add(INDICATOR_TIMEOUT, G_SOURCE_FUNC(hb_hide_file_indicator), NULL);
}

// Help > Help
// Display the program's help text
void hb_menu_help_help(GtkMenuItem *widget, GtkWindow *parent_window)
{
    // Pointer to the current help window (NULL, if there's none).
    // This pointer is remembered between different function calls.
    static GtkWidget *current_help_window = NULL;

    // Check if a help window already exists
    if (current_help_window != NULL)
    {
        // If there is a help window already,
        // just bring it to the front instead of creating a new one.
        gtk_window_present(GTK_WINDOW(current_help_window));
        return;
    }
    
    // Create the help window
    GtkApplication *app = gtk_window_get_application(parent_window);
    GtkWidget *help_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(help_window), "Help - Heartbound Save Editor");
    gtk_window_set_default_size(GTK_WINDOW(help_window), WINDOW_WIDTH * 0.9, WINDOW_HEIGHT * 0.96);
    
    // The 'current_help_window' is set to NULL when the help window is closed
    current_help_window = help_window;
    g_signal_connect(help_window, "destroy", G_CALLBACK(hb_help_window_destroyed), &current_help_window);

    // Close the help window when the main window is closed
    g_signal_connect_swapped(parent_window, "destroy", G_CALLBACK(gtk_widget_destroy), help_window);

    /* Note:
        I am aware that it would be better to just have this text to be generated
        automatically. But for the time being, it should suffice to have it hardcoded.
        TO DO for a future update.
    */

    char *help_text = (
        "The program opens the default Heartbound Save automatically, since the"
        " game only aloows one save file. However you can open and save to other"
        " locations using this program (click the Open button, or drag the save"
        " file into the window or the executable). This way you can create backups.\n\n"
        "Your default Heartbound Save is located at:\n<tt>%s</tt>\n\n"
        "This version of the editor supports Heartbound until its version <b>1.0.9.55.</b>"
        " Whenever Heartbound gets updated, this editor will be updated as soon"
        " as possible to support it. So feel free to check our "
        "<a href='https://github.com/tbpaolini/Heartbound-Save-Editor/releases'>download page</a>"
        " for the newest version of the editor.\n\n"
        "The editor is divided in tabs, one for each chapter of the game. And"
        " their contents are divided by locations, which contains the values"
        " that the game keep track of. You can edit those values, and the changes"
        " are saved to the file <i>after</i> you click the save button.\n\n"
        "Options of the menu:\n"
        "<b>File > New</b> (<i>Ctrl+N</i>): Create a new save file with the default values.\n"
        "<b>File > Open</b> (<i>Ctrl+O</i>): Open another save file.\n"
        "<b>File > Open default file</b> (<i>Shift+Ctrl+O</i>): Open the default Heartbound save.\n"
        "<b>File > Save</b> (<i>Ctrl+S</i>): Save the currently opened file.\n"
        "<b>File > Save as</b> (<i>Ctrl+A</i>): Save the opened file to a different location.\n"
        "<b>File > Save to default file</b> (<i>Shift+Ctrl+S</i>): Save to the default Heartbound file.\n"
        "<b>File > Exit</b> (<i>Alt+F4</i>): Close the editor.\n"
        "<b>Edit > Reload current saved data</b> (<i>F5</i>): Load the data again from the current file, replacing your unsaved changes.\n"
        "<b>Edit > Clear current saved data</b> (<i>Shift+Delete</i>): Reset all values on the editor as if you were starting the game anew.\n"
        "<b>Edit > Dark mode</b> (<i>F2</i>): Switch between the Dark and Light themes of the editor.\n"
        "<b>Edit > Automatic reloading</b> (<i>F3</i>): Reload the save file if it has been changed by another program.\n"
        "<b>Help > Help</b> (<i>F1</i>): Shows this help text.\n"
        "<b>Help > Download page</b>: Open a browser window with the official download page of this editor.\n"
        "<b>Help > About</b>: Show information and credits for the program.\n"
    );

    // Add the default Heartbound save location to the help text
    // (Yes, I still know that automatically generating the whole text would be better.)
    // (Please, just bear with be for the time being. It is coming in a later update ^_^)
    const size_t buffer_size = strlen(help_text) + PATH_BUFFER;
    char *help_text_format = calloc(buffer_size, sizeof(char));
    if (help_text_format == NULL) return;
    snprintf(help_text_format, buffer_size, help_text, SAVE_PATH);

    // Create the label to display the text
    GtkWidget *help_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(help_label), help_text_format);
    free(help_text_format);

    // Text layout of the label
    gtk_label_set_line_wrap(GTK_LABEL(help_label), TRUE);   // Wrap text to the next line
    gtk_label_set_line_wrap_mode(GTK_LABEL(help_label), PANGO_WRAP_WORD_CHAR);  // Wrap at word boundaries, break words if there's no room for them
    gtk_widget_set_halign(help_label, GTK_ALIGN_FILL);      // Fill the available horizontal space, while aligning to the left
    gtk_widget_set_valign(help_label, GTK_ALIGN_START);     // Align text to the top

    // Set the label's text as selectable
    // (because the user might want to copy the default save file's path)
    gtk_label_set_selectable(GTK_LABEL(help_label), TRUE);
    g_signal_connect(GTK_LABEL(help_label), "draw", G_CALLBACK(hb_help_window_fix), NULL);
    /* Note:
        All the label's text becomes selected when it is set as selectable.
        So I am connecting to a callback function that unselects it.
        That callback runs once (and only once) when the text is drawn for the first time.
    */

    // Create a container for the label
    GtkWidget *help_label_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_set_border_width(GTK_CONTAINER(help_label_box), WINDOW_BORDER);
    gtk_container_add(GTK_CONTAINER(help_label_box), help_label);

    // Create the scrollbar for the label box
    GtkWidget *help_scrollbar = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_hexpand(help_scrollbar, TRUE);
    gtk_container_add(GTK_CONTAINER(help_scrollbar), help_label_box);;

    // Add the text with the scrollbar to the help window
    gtk_container_add(GTK_CONTAINER(help_window), help_scrollbar);

    // Display the help window
    gtk_widget_show_all(help_window);
}

// Set the "current help window" to NULL, when it is destroyed
void hb_help_window_destroyed(GtkWidget *help_window, GtkWidget **current_help_window)
{
    *current_help_window = NULL;
}

// Unselect the help text when it is drawn for the first time
void hb_help_window_fix(GtkLabel *help_label)
{
    // Set the selection region from character 0 to character 0
    // (so nothing is selected)
    gtk_label_select_region(help_label, 0, 0);

    // Disconnect from this callback function after it is run
    // (so it does not runs continuously, which would prevent the user from selecting text)
    g_signal_handlers_disconnect_by_func(help_label, G_CALLBACK(hb_help_window_fix), NULL);
}

// Help > Download page
// Open the page for downloading the Heartbound Save Editor
void hb_menu_help_download(GtkMenuItem *widget, GtkWindow *window)
{
    ShellExecuteA(NULL, "open", "https://github.com/tbpaolini/Heartbound-Save-Editor/releases", NULL, NULL, SW_SHOWNORMAL);
}

// Help > About
// Show a dialog with credits and info about the program
void hb_menu_help_about(GtkMenuItem *widget, GtkWindow *window)
{
    GdkPixbuf *logo = gdk_pixbuf_new_from_file("../lib/icon.png", NULL);
    char *authors[] = {"Tiago Becerra Paolini https://github.com/tbpaolini", NULL};
    char *documenters[] = {"Steets https://steets.tech/", NULL};
    gtk_show_about_dialog(
        window,
        "logo", logo,
        "program-name", "Heartbound Save Editor",
        "version", "Version 1.0.0.2",
        "authors", authors,
        "documenters", documenters,
        "copyright", "Copyright 2022 by Tiago Becerra Paolini",
        "license-type", GTK_LICENSE_MIT_X11,
        "website", "https://github.com/tbpaolini/Heartbound-Save-Editor",
        "website-label", "Source Code",
        "comments", "Disclaimer:\nThis program was not made by the developers of Heartbound.",
        NULL
    );
}

// ****************
// Helper functions
// ****************

// Display a file chooser dialog for where to save a file.
// Returns 'true' if the user has chosen to save,
// and writes the the path to 'path_output' buffer.
bool hb_save_dialog(char *dialog_title, GtkWindow *window, char *path_output)
{
    // Create the file dialog
    GtkFileChooserNative *file_chooser = gtk_file_chooser_native_new(
        dialog_title,                   // Title of the dialog
        window,                         // Parent of the dialog (the main window)
        GTK_FILE_CHOOSER_ACTION_SAVE,   // Open file action
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
    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(file_chooser), file_filter_thor);

    // Display the dialog
    gint status = gtk_native_dialog_run(GTK_NATIVE_DIALOG(file_chooser));

    // Check if the user has chosen a file
    if (status != GTK_RESPONSE_ACCEPT) return false;

    // Get the path where the user chose to save
    char *file_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser));

    // Copy the file name to the output
    strncpy(path_output, file_name, PATH_BUFFER);
    
    // If the file filter is ".thor", append that suffix to the path if it does not have already
    if (!g_str_has_suffix(file_name, ".thor"))
    {
        strncat(path_output, ".thor", PATH_BUFFER);
    }
    
    // Free the memory used by the dialog
    g_free(file_name);
    g_object_unref(file_chooser);

    return true;
}

// Confirmation dialog to proceed or not with an action
bool hb_confirmation(char *dialog_title, char *dialog_message, GtkWindow *main_window)
{
    // Create an "Yes / No" dialog
    GtkWidget *warning_dialog = hb_create_dialog_with_title_and_image(
        main_window,
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_YES_NO,
        dialog_title,
        "dialog-warning",
        dialog_message
    );

    // Set the "No" button as selected by default
    GtkWidget *no_button = gtk_dialog_get_widget_for_response(GTK_DIALOG(warning_dialog), GTK_RESPONSE_NO);
    gtk_widget_grab_focus(no_button);

    // Display the dialog and get the user's response
    gint response = gtk_dialog_run(GTK_DIALOG(warning_dialog));
    gtk_widget_destroy(warning_dialog);
    
    if (response == GTK_RESPONSE_YES) return true; else return false;
}

// Checks whether the save data has been changed in the editor,
// so it may ask the user to save the unsaved data.
// The function returns 'true' if it is OK to proceed, otherwise 'false'.
bool hb_check_if_data_changed(char *dialog_title, GtkWindow *window)
{
    // Return 'true' if the data has not changed
    if (!has_unsaved_data) return true;

    // Ask the user what to do if the data has changed
    bool user_response = hb_confirmation(
        dialog_title,
        "Unsaved changes will be lost.\n"
        "Proceed?",
        window
    );

    // Return the user's response
    return user_response;
}

// Marks the data as "changed"
// Note: The open/save functions already set the data as "unchanged".
//       This function exists for also adding an asterisk to the window
//       title, in order to indicate that the file is unsaved.
void hb_flag_data_as_changed(GtkWidget *widget)
{
    // Do nothing if the data is already flagged as "changed"
    if (has_unsaved_data) return;
    
    // Flag data as "unsaved"
    has_unsaved_data = true;

    // Allocate buffer for the changed window title
    char *restrict title = calloc(PATH_BUFFER, sizeof(char));
    if (title == NULL) return;

    // Place an asterisk at the beginning of the window title
    snprintf(title, PATH_BUFFER, "*%s - %s", CURRENT_FILE, WINDOW_TITLE);
    GtkWidget *my_window = gtk_widget_get_toplevel(widget);
    if (GTK_IS_WINDOW(my_window)) gtk_window_set_title(GTK_WINDOW(my_window), title);

    // Note: The open/save functions already remove the asterisk.

    // Deallocate the buffer's memory
    free(title);
}

// Confirm if the user wants to close the editor when there is unsaved data
void hb_confirm_close(GtkWindow *window)
{
    bool proceed = hb_check_if_data_changed("Confirm exit", window);
    if (proceed)
    {
        // Close the window if the there is no unsaved changes
        // or if the user has chosen to leave
        gtk_window_close(window);
    }
    else
    {
        // Keep showing the window if the user has chosen to stay
        gtk_window_present(window);
    }
}