/*
    Callback functions used by the graphical user interface (GUI) in order to
    validate or modify the save data.
*/

#ifndef _HB_GUI_CALLBACK
#define _HB_GUI_CALLBACK

#include <gtk\gtk.h>
#include <hb_save.h>
#include "..\config.h"

// Buffer for filtering the contents of the text entries
static char text_entry_buffer[TEXT_FIELD_MAX_CHARS + 1];
/* Note:
    Accordig to the GTK's documentation (https://docs.gtk.org/gobject/func.signal_connect.html),
    the callback functions are called synchronously. From that I infer that there is no issue
    in having a single buffer for filtering all text fields, since only one entry is going
    to use it at any given time.
    
    If somehow multiple text fields were edited at the same time, their callbacks would be
    executed one after the other. Plus there isn't any means for an user to edit multiple
    fields at once on this program.
*/

// Set a storyline variable's value when one of its radio buttons is clicked
void hb_setvar_radio_button(GtkRadioButton* widget, StorylineVars *story_var);

// Set a storyline variable's value when a No/Yes radio button was clicked
// These kind of buttons are used as a fallback when the variable does not fall in the other types
void hb_setvar_no_yes(GtkRadioButton* widget, StorylineVars *story_var);

// Set a storyline variable's value after its text box is edited
void hb_setvar_text_entry(GtkEntry *widget, StorylineVars *story_var);

// Filter the non numeric characters out of a string.
// In other words, ensure that the string represents a natural number (positive integer).
void hb_text_filter_natural(char *text, size_t max_length);

// Filter out from a string characters that aren't numbers, signs, or points.
// That is, ensure the resulting string represents a real number.
void hb_text_filter_real(char *text, size_t max_length);

#endif