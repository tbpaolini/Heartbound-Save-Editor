/*
    Callback functions used by the graphical user interface (GUI) in order to
    validate or modify the save data.
*/

#include <gtk\gtk.h>
#include <hb_save.h>

void hb_setvar_radio_button(GtkRadioButton* widget, StorylineVars *story_var)
{
    gboolean button_is_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    if (!button_is_active) return;
    
    GSList *my_group = gtk_radio_button_get_group(widget);
    GtkToggleButton *current_button = GTK_TOGGLE_BUTTON(my_group->data);

    for (size_t i = story_var->num_entries - 1; i >= 0; i--)
    {
        if ( gtk_toggle_button_get_active(current_button) )
        {
            #ifdef _DEBUG
            g_message("Var %llu -> %s = %s (%s)", story_var->number, story_var->name, story_var->aliases[i].description, story_var->aliases[i].header[0]);
            #endif
            return;
        }

        my_group = my_group->next;
        current_button = GTK_TOGGLE_BUTTON(my_group->data);
    }
    

    // gtk_toggle_button_get_active();
    // hb_save_data[story_var->number].value = story_var->value;
    // printf("Var %d = %.0f\n", story_var->number, story_var->value);
}