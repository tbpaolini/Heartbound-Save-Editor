/*
    Callback functions used by the graphical user interface (GUI) in order to
    validate or modify the save data.
*/

#include <gtk\gtk.h>
#include <hb_save.h>

// Set a storyline variable's value when one of its radio buttons is clicked
void hb_setvar_radio_button(GtkRadioButton* widget, StorylineVars *story_var);

// Set a storyline variable's value when a No/Yes radio button was clicked
// These kind of buttons are used as a fallback when the variable does not fall in the other types
void hb_setvar_no_yes(GtkRadioButton* widget, StorylineVars *story_var);