/*
    Callback functions used by the graphical user interface (GUI) in order to
    validate or modify the save data.
*/

#include <ctype.h>
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
        (story_var->value == 1.0 ? "Yes" : "No"),
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
    strncpy(text_entry_buffer, my_text, TEXT_FIELD_MAX_CHARS + 1);

    // Remove the non-digit characters
    hb_text_filter_natural(text_entry_buffer, TEXT_FIELD_MAX_CHARS);
    
    // Set the filtered text to the widget
    g_signal_handlers_block_by_func(widget, G_CALLBACK(hb_setvar_text_entry), story_var);   // Prevents the 'changed' signal from being emitted recursively
    gtk_entry_set_text(widget, text_entry_buffer);
    g_signal_handlers_unblock_by_func(widget, G_CALLBACK(hb_setvar_text_entry), story_var);

    // Modify the storyline variable
    story_var->value = atof(text_entry_buffer);

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