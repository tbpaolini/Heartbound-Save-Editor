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

// Pointer to the room list
static GtkComboBox *room_dropdown_list;

// Pointer to the group of radio buttons of the known glyphs
static GSList *known_glyphs_group;

// Pointer to the game seed entry
static GtkEntry *game_seed_entry;

// Text that shows up temporarily when a file is saved or loaded
static GtkWidget *file_indicator;
static const char *FILE_LOADED_MESSAGE = "File loaded successfully!";
static const char *FILE_SAVED_MESSAGE = "File saved successfully!";

// Whether the editor is currently loading a save file
// (used to prevent the callback functions to fire while the file is loaded)
static bool is_loading_file = false;

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

// Store the pointers to the Rooms List, Known Glyphs, and Game Seed,
// so they can be updated when a new file is loaded.
void hb_bind_widgets(GtkComboBox *room_widget, GSList *glyph_widget, GtkEntry *seed_widget);

// Update the coordinates field when the room is changed,
// so the player do not end up stuck out of bounds or in a wall.
void hb_set_coordinates_from_room(GtkComboBoxText *widget);

// Set one of the variables that hold the player's attributes
// (coordinates or hitpoints)
void hb_setvar_player_attribute(GtkEntry *widget, double *attribute);

// Set the radio button of the 'known glyphs' choice
void hb_setvar_known_glyphs(GtkRadioButton *widget, double *known_glyphs);

// Set the game seed's variable when its field changes
void hb_setvar_game_seed(GtkEntry *widget,  char *game_seed);

// Generate a random game seed (9 numeric digits)
void hb_random_seed(char *game_seed);

// Highlight a menu item when the mouse pointer is over the item
void hb_menu_hover(GtkMenuItem *widget, GdkEventCrossing event, void *data);

// A wrapper for the 'hb_write_save()' function from 'hb_gui_save_io.c'
// This function is called when the save option is left-clicked on the interface.
void hb_save_file(GtkMenuItem *widget, GdkEventButton event, GtkWindow *window);

// Load a Heartbound save file into the editor
void hb_open_file(GtkMenuItem *widget, GdkEventButton event, GtkWindow *window);

// Updates the user interface with the values on the data structure of the save file
void hb_load_data_into_interface(GtkWindow *window);

// Ask the user what to do if the program could not load the default save during startup
void hb_failed_to_open_default_save(GtkWindow *main_window);

// Handle the user's response to 'hb_failed_to_open_default_save()'
void hb_failed_to_open_default_save_response(GtkDialog dialog, gint response_id, GtkWindow *main_window);

// Create a save file with the default values
bool hb_create_default_save(char *path);

// Create a message dialog with a custom image and title
GtkWidget *hb_create_dialog_with_title_and_image(
    GtkWindow *parent,
    GtkDialogFlags flags,
    GtkMessageType type,
    GtkButtonsType buttons, 
    const char *message,
    const char *title,
    const char *image_icon_name
);

// Bind the pointer of the "file loaded" indicator, so it can be shown or hidden.
void hb_bind_file_indicator(GtkWidget *widget);

// Hide the "file loaded" indicator
gboolean hb_hide_file_indicator();

// Make the widgets on the notebook to be clickable after the menu items have been used.
// Without this fix, if one clicks on the menu, then tries to click on something on
// the notebook, it would be necessary to click twice to interact with the notebook.
void hb_notebook_fix(GtkNotebook *widget, GdkEventCrossing event, void *data);

// ***********************
// Options of the menu bar
// ***********************

void placeholder(void* widget, void* data);

// File > Open
void hb_menu_file_open(GtkMenuItem *widget, GtkWindow *window);

// File > Save
void hb_menu_file_save(GtkMenuItem *widget, GtkWindow *window);

#endif