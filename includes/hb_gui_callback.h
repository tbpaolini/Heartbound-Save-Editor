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

// Pointers to the entry fields that hold the player's coordinates
static GtkEntry *x_entry, *y_entry;

// Pointers to the entry fields that hold the player's hit points
static GtkEntry *hp_cur_entry, *hp_max_entry;

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

// Filter out from a string characters that aren't numbers or signs.
// That is, ensure the resulting string represents a positive or negative integer.
void hb_text_filter_integer(char *text, size_t max_length);

// Make the dropdown list to show when clicked for the first time
// For some reason, it only shows when clicked for the second time, perhaps because that it has several items.
// So this call function is called once after the first time the widget is drawn, to force the dropdown to open then close.
// This way when the user click on the list, then it will show normally.
void hb_dropdown_list_fix(GtkComboBox *widget);

// Store the pointers for the text entries of the player's coordinates,
// so the dropdown list can change them when a room is selected.
void hb_bind_xy_entries(GtkEntry *x, GtkEntry *y);

// Store the pointers for the text entries of the player's hit points,
// so they can interact with each other (current HP capped to the maximum HP)
void hb_bind_hp_entries(GtkEntry *current, GtkEntry *maximum);

// Update the coordinates field when the room is changed,
// so the player do not end up stuck out of bounds or in a wall.
void hb_set_coordinates_from_room(GtkComboBoxText *widget);

// Set one of the variables that hold the player's attributes
// (coordinates or hitpoints)
void hb_setvar_player_attribute(GtkEntry *widget, double *varriable);

// Set the game seed's variable when its field changes
void hb_setvar_game_seed(GtkEntry *widget,  char *data);

// Generate a random game seed (9 numeric digits)
void hb_random_seed(char *game_seed);

#endif