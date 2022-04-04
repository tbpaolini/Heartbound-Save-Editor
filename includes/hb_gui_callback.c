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
#include <hb_save.h>
#include <hb_gui_callback.h>

// Set a storyline variable's value when one of its radio buttons is clicked
void hb_setvar_radio_button(GtkRadioButton* widget, StorylineVars *story_var)
{
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

// Update the Room ID variable, that stores the current room.
// Also update the coordinates field when the room is changed,
// so the player do not end up stuck out of bounds or in a wall.
void hb_set_coordinates_from_room(GtkComboBoxText *widget)
{
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
        if (attribute == &hb_hitpoints_current)
        {
            // Check if the current HP is not bigger than the maximum HP
            if (hb_hitpoints_current > hb_hitpoints_maximum)
            {
                // Set the current HP variable and its field to the maximum value
                hb_hitpoints_current = hb_hitpoints_maximum;
                snprintf(text_entry_buffer, sizeof(text_entry_buffer), "%.0f", hb_hitpoints_current);
                gtk_entry_set_text(widget, text_entry_buffer);
            }
        }
    }
    else
    {
        // Set the attribute to zero if there is not any text
        *attribute = 0.0;
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

// Set the game seed's variable when its field changes
void hb_setvar_game_seed(GtkEntry *widget,  char *game_seed)
{
    // Get the text from the widget
    const char *my_text = gtk_entry_get_text(widget);

    // Copy the text to the buffer
    if (gtk_entry_get_text_length(widget) > 0)
    {
        strncpy(text_entry_buffer, my_text, sizeof(text_entry_buffer));
    }

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