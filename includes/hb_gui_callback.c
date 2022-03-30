/*
    Callback functions used by the graphical user interface (GUI) in order to
    validate or modify the save data.
*/

#include <gtk\gtk.h>
#include <hb_save.h>

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