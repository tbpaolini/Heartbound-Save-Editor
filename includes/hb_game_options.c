#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <hb_game_options.h>
#include "../config.h"
#ifdef _WIN32
#include <windows.h>
#include <wchar.h>
#endif // _WIN32

// Full file system path to the game options file
char OPTIONS_PATH[PATH_BUFFER] = {0};

/* Store each of the game options as a C-style string

Indices:
    [0]  Global audio volume (float from 0.00 to 1.00)
    [1]  Keyboard move Up (digit or uppercase A-Z letter)
    [2]  Keyboard move Left key (digit or uppercase A-Z letter)
    [3]  Keyboard move Down (digit or uppercase A-Z letter)
    [4]  Keyboard move Right (digit or uppercase A-Z letter)
    [5]  Accept key (digit or uppercase A-Z letter)
    [6]  Cancel key (digit or uppercase A-Z letter)
    [7]  Gamepad button 1 ('accept' function)
    [8]  Gamepad button 2 ('cancel' function)
    [9]  Gamepad button 3 (used on minigames, function varies)
    [10] Gamepad button 4 (used on minigames, function varies)
    [11] Controller type (0 = Xbox, 1 = PlayStation)
    [12] Fullscreen is active (0 = false,  1 = true)
*/
char hb_game_options[OPTIONS_COUNT][OPTIONS_BUFFER] = {0};

// Map the four controller buttons to their respective ID
// Note: values obtained from data mining the game
static const char gamepad_buttons[4][6] = {
    "32769",    // A or ×
    "32770",    // B or ○
    "32771",    // X or □
    "32772",    // Y or △
};
static const size_t gamepad_buttons_len = sizeof(gamepad_buttons) / sizeof(gamepad_buttons[0]);

// Default game options
static const char *default_options[OPTIONS_COUNT] = {
    "0.5",
    "W", "A", "S", "D", "Z", "X",
    "32769", "32770", "32771", "32772",
    "0", "0",
};

// The keyboard controls that the player can change
static const char *kb_controls[] = {"Up :", "Left :", "Down :", "Right :", "Accept :", "Cancel :"};
static const size_t kb_controls_len = sizeof(kb_controls) / sizeof(char *);

// The allowed keys of the keyboard
static const char *kb_inputs[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8",
    "9", "A", "B", "C", "D", "E", "F", "G", "H",
    "I", "J", "K", "L", "M", "N", "O", "P", "Q",
    "R", "S", "T", "U", "V", "W", "X", "Y", "Z" };
static const size_t kb_inputs_len = sizeof(kb_inputs) / sizeof(char *);

// The allowed buttons of the gamepad
// (the button order matches the order on the 'gamepad_buttons[]' array)
static const char *gp_inputs[2][4] = {
    {"A", "B", "X", "Y"},           // Xbox controller
    {u8"×", u8"○", u8"□", u8"△"},   // PlayStation controller
};
static const size_t gp_inputs_num = sizeof(gp_inputs) / sizeof(gp_inputs[0]);   // Amount of gamepad types
static const size_t gp_inputs_len = sizeof(gp_inputs[0]) / sizeof(char *);      // Amount of buttons on the gamepad

// The gamepad controls that the player can change
static const char *gp_controls[] = {"Accept :", "Cancel :", "Button 3 :", "Button 4 :"};
static const size_t gp_controls_len = sizeof(gp_controls) / sizeof(char *);

// Names of the gamepad types
const char *gp_types[2] = {"Xbox", "PlayStation"};
const size_t gp_types_len = sizeof(gp_types) / sizeof(char *);

// The GTK widgets of the game options fields on the user interface
// (the new option values will be read from there when saving the options)
static GtkWidget *options_widgets[OPTIONS_COUNT];

// Read the game's options file
// Note: this function should be called after the save was opened,
//       because that sets the variable SAVE_ROOT (path to the save's folder).
void hb_read_game_options()
{
    // If this function was run before
    static bool options_init = false;
    
    // Initialize the game options' path when running the function for the first time
    if (!options_init)
    {
        // Parse the path to the options file
        #ifdef _WIN32
        snprintf(OPTIONS_PATH, sizeof(OPTIONS_PATH), "%s\\%s", SAVE_ROOT, OPTIONS_FNAME);
        #else
        snprintf(OPTIONS_PATH, sizeof(OPTIONS_PATH), "%s/%s", SAVE_ROOT, OPTIONS_FNAME);
        #endif
    }

    // Set the default values for the game options
    for (size_t i = 0; i < OPTIONS_COUNT; i++)
    {
        snprintf(hb_game_options[i], OPTIONS_BUFFER, "%s", default_options[i]);
    }

    // Path were to save the options file
    char *my_path;
    char path_alt[PATH_BUFFER]; // Buffer for the case the options file needs to be saved to a non-default location
    
    if (strncmp(CURRENT_FILE, SAVE_PATH, PATH_BUFFER) == 0)
    {
        // If saving to the default save location,
        // the options file will be saved to the default options location
        my_path = OPTIONS_PATH;
    }
    else
    {
        // If not saving to the default save file location,
        // the options file will be saved to a new file (different from the save file)
        snprintf(path_alt, PATH_BUFFER, "%s-options", CURRENT_FILE);
        my_path = path_alt;
    }

    // Parse the values from the game options file
    
    #ifdef _WIN32
    FILE *options_file = __win_fopen(my_path, "rt");
    #else
    FILE *options_file = fopen(my_path, "rt");
    #endif // _WIN32
    if (!options_file) return;  // The default option values will be used if opening the file fails
    
    char line_buffer[OPTIONS_BUFFER+1] = {0};   // Buffer for reading the lines of the options file
    size_t pos = 0; // Position on the line buffer

    // Volume's value
    {
        char *const status = fgets(line_buffer, OPTIONS_BUFFER, options_file);
        if (!status) {fclose(options_file); return;}

        // Validate the volume's value
        bool is_valid = false;
        size_t dot_count = 0;
        while (line_buffer[pos] != '\0')
        {
            const char my_char = line_buffer[pos++];
            
            if (my_char == '\n')
            {
                // If we arrived at the end of the line without invalid characters
                is_valid = true;
                line_buffer[pos-1] = '\0';    // Remove the newline character
                break;
            }

            // Each character can only be a digit or a dot (being one dot at most)
            if (!isdigit(my_char) && my_char != '.') break;
            if (my_char == '.') dot_count++;
            if (dot_count > 1) break;
        }

        // Store the value if valid
        if (is_valid)
        {
            snprintf(hb_game_options[0], OPTIONS_BUFFER, "%s", line_buffer);
        }
    }

    // Keyboard input
    for (size_t i = 0; i < 6; i++)
    {
        char *const status = fgets(line_buffer, OPTIONS_BUFFER, options_file);
        if (!status) {fclose(options_file); return;}

        // Validate the input's value
        // The line can only have one character, and it has to be a digit or letter
        if (
            (isalpha(line_buffer[0]) || isdigit(line_buffer[0]))
            &&
            (line_buffer[1] == '\n')
        )
        {
            // Store the character, converting it to uppercase if needed.
            snprintf(hb_game_options[1+i], OPTIONS_BUFFER, "%c", (char)toupper(line_buffer[0]));
        }
    }

    // Gamepad input
    for (size_t i = 0; i < 4; i++)
    {
        char *const status = fgets(line_buffer, OPTIONS_BUFFER, options_file);
        if (!status) {fclose(options_file); return;}

        // Validate the input's value
        // The line can only have digits
        bool is_valid = false;
        size_t pos = 0;
        while (line_buffer[pos] != '\0')
        {
            const char my_char = line_buffer[pos++];
            if (my_char == '\n')
            {
                // If we arrived at the end of the line without invalid characters
                is_valid = true;
                line_buffer[pos-1] = '\0';    // Remove the newline character
                break;
            }

            // Each character can only be a digit
            if (!isdigit(my_char)) break;
        }

        // Store the value if valid
        if (is_valid)
        {
            snprintf(hb_game_options[7+i], OPTIONS_BUFFER, "%s", line_buffer);
        }
    }

    // Controller type and fullscreen (booleans)
    for (size_t i = 0; i < 2; i++)
    {
        char *const status = fgets(line_buffer, OPTIONS_BUFFER, options_file);
        if (!status) {fclose(options_file); return;}

        // Validate the input's value
        // The line can only be '0' or '1'
        if (
            (line_buffer[0] == '0' || line_buffer[0] == '1')
            &&
            (line_buffer[1] == '\n')
        )
        {
            // Store the value, if valid
            snprintf(hb_game_options[11+i], OPTIONS_BUFFER, "%c", line_buffer[0]);
        }
    }

    fclose(options_file);

    // Update the interface in case the options are loaded again
    if (options_init) __update_options_interface();

    // Flag that this function has been run at least once
    options_init = true;
}

// Save the game's options file
void hb_save_game_options()
{
    // Update the options' values before saving
    __update_game_options();
    
    // Path were to save the options file
    char *my_path;
    char path_alt[PATH_BUFFER]; // Buffer for the case the options file needs to be saved to a non-default location
    
    if (strncmp(CURRENT_FILE, SAVE_PATH, PATH_BUFFER) == 0)
    {
        // If saving to the default save location,
        // the options file will be saved to the default options location
        my_path = OPTIONS_PATH;
    }
    else
    {
        // If not saving to the default save file location,
        // the options file will be saved to a new file (different from the save file)
        snprintf(path_alt, PATH_BUFFER, "%s-options", CURRENT_FILE);
        my_path = path_alt;
    }

    #ifdef _WIN32
    FILE *options_file = __win_fopen(my_path, "wt");
    #else
    FILE *options_file = fopen(my_path, "wt");
    #endif // _WIN32
    if (!options_file) return;

    for (size_t i = 0; i < OPTIONS_COUNT; i++)
    {
        // Write the values to the options file
        fprintf(options_file, "%s\n", hb_game_options[i]);
    }

    fclose(options_file);
}

// Reset the game's options back to their default values
void hb_reset_game_options()
{
    // Set the the game options back to the default values
    for (size_t i = 0; i < OPTIONS_COUNT; i++)
    {
        snprintf(hb_game_options[i], OPTIONS_BUFFER, "%s", default_options[i]);
    }

    // Draw the options' values on the interface
    __update_options_interface();
}

// Add to the user interface the fields corresponding to the game options
// Note: the function needs to receive the GTK container where the fields will be added on.
void hb_insert_options_fields(GtkWidget *container)
{
    // Create and add a new box with a label (for the name of an option)
    GtkWidget *my_wrapper;
    GtkWidget *my_name_label;
    #define NEW_LABEL_BOX(text) \
        my_wrapper = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, ENTRY_VERTICAL_SPACING);\
        gtk_widget_set_hexpand(my_wrapper, TRUE);\
        my_name_label = gtk_label_new(text);\
        gtk_widget_set_valign(my_name_label, GTK_ALIGN_START);\
        gtk_widget_set_margin_end(my_name_label, ENTRY_HORIZONTAL_SPACING);\
        gtk_widget_set_margin_top(my_name_label, 8);\
        gtk_container_add(GTK_CONTAINER(my_wrapper), my_name_label);\
        gtk_container_add(GTK_CONTAINER(container), my_wrapper)
    
    // Create and add flow box for the contents
    GtkWidget *my_flowbox;
    #define NEW_FLOWBOX() \
        my_flowbox = gtk_flow_box_new();\
        gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(my_flowbox), GTK_SELECTION_NONE);\
        gtk_widget_set_valign(my_flowbox, GTK_ALIGN_START);\
        gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(my_flowbox), 2);\
        gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(my_flowbox), 14);\
        gtk_container_add(GTK_CONTAINER(my_wrapper), my_flowbox);
        
    // Volume slider

    // Create the wrapper box for the volume slider
    NEW_LABEL_BOX("Audio volume :");
    NEW_FLOWBOX();
    gtk_widget_set_tooltip_text(
        my_name_label,
        "Global volume control for the game."
    );

    // Create the slider
    GtkWidget *volume_slider = gtk_scale_new_with_range(
        GTK_ORIENTATION_HORIZONTAL,
        0.0, 1.0, 0.01  // Minumum and max values, and step when moving the slider with the keyboard
    );
    options_widgets[0] = volume_slider;
    gtk_widget_set_hexpand(volume_slider, TRUE);
    gtk_widget_set_size_request(volume_slider, VOLUME_SLIDER_MIN_WIDTH, -1);
    gtk_container_add(GTK_CONTAINER(my_flowbox), volume_slider);
    gtk_scale_set_has_origin(GTK_SCALE(volume_slider), TRUE);   // Highlight the position between the start and the current value
    gtk_scale_set_draw_value(GTK_SCALE(volume_slider), TRUE);   // Display the current value next to the slider
    gtk_scale_set_digits(GTK_SCALE(volume_slider), 2);          // Amount of decimal places to show on the value
    gtk_scale_set_value_pos(GTK_SCALE(volume_slider), GTK_POS_RIGHT);   // Where to display the value in relation to the slider

    // Keyboard input

    // Create the wrapper box for the keyboard controls
    NEW_LABEL_BOX("Keyboard controls :");
    NEW_FLOWBOX();
    gtk_widget_set_tooltip_text(
        my_name_label,
        "Game input when using a keyboard."
    );
    
    // Create all fields for the keyboard controls
    for (size_t i = 0; i < kb_controls_len; i++)
    {
        // Text label for the key
        GtkWidget *my_label = gtk_label_new(kb_controls[i]);
        gtk_container_add(GTK_CONTAINER(my_flowbox), my_label);
        gtk_widget_set_margin_start(my_label, TEXT_FIELD_MARGIN);

        // Drop down menu for each possible key value
        GtkWidget *key_selection = gtk_combo_box_text_new();
        options_widgets[1+i] = key_selection;
        gtk_container_add(GTK_CONTAINER(my_flowbox), key_selection);
        
        // Add the possible keys to the dropdown menu
        for (size_t j = 0; j < kb_inputs_len; j++)
        {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(key_selection), kb_inputs[j]);
        }
    }

    // Gamepad input

    // Create the wrapper box for the gamepad controls
    NEW_LABEL_BOX("Gamepad controls :");
    NEW_FLOWBOX();
    gtk_widget_set_tooltip_text(
        my_name_label,
        "Game input when using a controller."
    );

    // Whether using a Xbox or a PlayStation controller (0 or 1, respectively)
    size_t control_type = hb_game_options[11][0] - '0';
    if (control_type >= gp_inputs_num) control_type = 0;    // Check if not out-of-bound
    
    // Create all fields for the gamepad controls
    for (size_t i = 0; i < gp_controls_len; i++)
    {
        // Text label for the button
        GtkWidget *my_label = gtk_label_new(gp_controls[i]);
        gtk_container_add(GTK_CONTAINER(my_flowbox), my_label);
        gtk_widget_set_margin_start(my_label, TEXT_FIELD_MARGIN);

        // Drop down menu for each possible button value
        GtkWidget *button_selection = gtk_combo_box_text_new();
        options_widgets[7+i] = button_selection;
        gtk_container_add(GTK_CONTAINER(my_flowbox), button_selection);
        
        // Add the possible buttons to the dropdown menu
        for (size_t j = 0; j < gp_inputs_len; j++)
        {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(button_selection), gp_inputs[control_type][j]);
        }
    }

    // Controller type

    NEW_LABEL_BOX("Gamepad type :");
    NEW_FLOWBOX();
    gtk_widget_set_tooltip_text(
        my_name_label,
        "Which kind of controller is being used for the game."
    );

    {
        GtkWidget *my_radio_button = NULL;
        GtkWidget *previous_button = NULL;

        for (size_t i = 0; i < gp_types_len; i++)
        {
            // Create a radio button within the controller type's group
            my_radio_button = gtk_radio_button_new_with_label(NULL, gp_types[i]);
            gtk_radio_button_join_group(GTK_RADIO_BUTTON(my_radio_button), GTK_RADIO_BUTTON(previous_button));
            previous_button = my_radio_button;

            // Add the radio button to the flow box
            gtk_container_add(GTK_CONTAINER(my_flowbox), my_radio_button);

            // Callback function for updating the buttons's text when the gamepad's type is toggled
            // (the text will be toggled between "ABXY" and "×○□△")
            g_signal_connect(GTK_TOGGLE_BUTTON(my_radio_button), "toggled", G_CALLBACK(__update_buttons_text), NULL);
        }

        options_widgets[11] = previous_button;
    }

    // Full screen active

    NEW_LABEL_BOX("Full screen active :");
    NEW_FLOWBOX();
    gtk_widget_set_tooltip_text(
        my_name_label,
        "Whether the game is running in windowed or full screen mode."
    );

    {
        GtkWidget *my_radio_button = NULL;
        GtkWidget *previous_button = NULL;

        const char *my_text[2] = {"No", "Yes"};
        const bool is_fullscreen = (hb_game_options[12][0] != '0') ? true : false;

        for (size_t i = 0; i < 2; i++)
        {
            // Create a radio button within the full screen's group
            my_radio_button = gtk_radio_button_new_with_label(NULL, my_text[i]);
            gtk_radio_button_join_group(GTK_RADIO_BUTTON(my_radio_button), GTK_RADIO_BUTTON(previous_button));
            previous_button = my_radio_button;

            // Add the radio button to the flow box
            gtk_container_add(GTK_CONTAINER(my_flowbox), my_radio_button);
        }

        options_widgets[12] = previous_button;
    }

    // Draw the modified options to the interface
    __update_options_interface();

    #undef NEW_LABEL_BOX
    #undef NEW_FLOWBOX
}

// Draw the changed game options fields to the interface
static inline void __update_options_interface()
{
    // Volume slider's initial position
    const double volume_value = atof(hb_game_options[0]);
    gtk_range_set_value(GTK_RANGE(options_widgets[0]), volume_value);   // Function automatically clamps the value to fit between min and max

    // Keyboard controls
    for (size_t i = 0; i < kb_controls_len; i++)
    {
        // Current key set on the game options file
        const char my_key = toupper((int)hb_game_options[1+i][0]);
        
        // Get the index on the dropdown menu for the current key
        size_t my_index = SIZE_MAX;

        if (isupper(my_key))
        {
            my_index = (my_key - 'A') + 10;
        }
        else if (isdigit(my_key))
        {
            my_index = my_key - '0';
        }

        // Set the dropdown menu to the current key
        if (my_index < kb_inputs_len)
        {
            gtk_combo_box_set_active( GTK_COMBO_BOX(options_widgets[1+i]), my_index );
        }
    }
    
    // Gamepad controls
    for (size_t i = 0; i < gp_controls_len; i++)
    {
        // Current button set on the game options file
        const char *my_button = hb_game_options[7+i];
        
        // Get the index on the dropdown menu for the current button
        size_t my_index = SIZE_MAX;

        for (size_t j = 0; j < gamepad_buttons_len; j++)
        {
            if (strcmp(my_button, gamepad_buttons[j]) == 0)
            {
                my_index = j;
                break;
            }
        }

        // Set the dropdown menu to the current button
        if (my_index < gp_inputs_len)
        {
            gtk_combo_box_set_active( GTK_COMBO_BOX(options_widgets[7+i]), my_index );
        }
    }

    // Controller type and full screen
    for (size_t i = 0; i < 2; i++)
    {
        // Get which one of the two radio buttons is active
        GtkWidget *radio_button = options_widgets[11+i];
        bool is_active = (hb_game_options[11+i][0] != '0');
        
        if (is_active)
        {
            // Set the right button to active
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(radio_button), TRUE );
        }
        else
        {
            // Set the left button to active
            GSList *my_group = gtk_radio_button_get_group( GTK_RADIO_BUTTON(radio_button) );
            GtkToggleButton *other_button = GTK_TOGGLE_BUTTON(my_group->next->data);
            gtk_toggle_button_set_active( other_button, TRUE );
        }
    }
}

// Change the displayed text of the gamepad buttons in order to match the selected gamepad type
// (the text will be toggled between "ABXY" and "×○□△")
static void __update_buttons_text(GtkToggleButton* widget, gpointer user_data)
{
    // Is the button that triggered the 'toggled' event active?
    gboolean button_is_active = gtk_toggle_button_get_active(widget);
    if (!button_is_active) return;

    // Get the group that the button is part of
    GSList *my_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(widget));
    GtkToggleButton *current_button = GTK_TOGGLE_BUTTON(my_group->data);

    // If using a PlayStation or a Xbox controller (1 or 0, respectively)
    const size_t control_type = gtk_toggle_button_get_active(current_button) ? 1 : 0;

    for (size_t i = 0; i < gp_inputs_len; i++)
    {
        GtkComboBoxText *button_selection = GTK_COMBO_BOX_TEXT(options_widgets[7+i]);
        
        // Remember the current item's index
        gint my_index = gtk_combo_box_get_active(GTK_COMBO_BOX(button_selection));

        // Remove all items from the menu
        gtk_combo_box_text_remove_all(button_selection);
        
        // Add the corresponding buttons to the dropdown menu
        for (size_t j = 0; j < gp_inputs_len; j++)
        {
            gtk_combo_box_text_append_text(button_selection, gp_inputs[control_type][j]);
        }

        // Set the index back to the original position
        gtk_combo_box_set_active(GTK_COMBO_BOX(button_selection), my_index);
    }
}

// Update the values on the 'hb_game_options[]' array from the selections on the interface
static inline void __update_game_options()
{
    // Audio's volume
    GtkRange *volume_slider = GTK_RANGE(options_widgets[0]);
    const double volume_value = gtk_range_get_value(volume_slider);
    snprintf(hb_game_options[0], OPTIONS_BUFFER, "%.2f", volume_value);

    // Keyboard controls
    for (size_t i = 0; i < kb_controls_len; i++)
    {
        GtkComboBox *key_selection = GTK_COMBO_BOX(options_widgets[1+i]);
        const gint my_index = gtk_combo_box_get_active(key_selection);
        if (my_index < kb_inputs_len)
        {
            snprintf(hb_game_options[1+i], OPTIONS_BUFFER, "%s", kb_inputs[my_index]);
        }
    }

    // Gamepad controls
    for (size_t i = 0; i < gp_controls_len; i++)
    {
        GtkComboBox *button_selection = GTK_COMBO_BOX(options_widgets[7+i]);
        const gint my_index = gtk_combo_box_get_active(button_selection);
        if (my_index < gp_inputs_len)
        {
            snprintf(hb_game_options[7+i], OPTIONS_BUFFER, "%s", gamepad_buttons[my_index]);
        }
    }
    
    // If using a PlayStation or a Xbox controller (ASCII '1' or '0', respectively)
    const char control_type = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(options_widgets[11])) ? '1' : '0';
    snprintf(hb_game_options[11], OPTIONS_BUFFER, "%c", control_type);

    // If full screen is active on the game
    const char is_fullscreen = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(options_widgets[12])) ? '1' : '0';
    snprintf(hb_game_options[12], OPTIONS_BUFFER, "%c", is_fullscreen);
}

// On Windows: Open a file that has unicode (UTF-8) characters in its path
#ifdef _WIN32
static inline FILE* __win_fopen(const char *path, const char *mode)
{
    // Size of the string buffers
    size_t path_len = strlen(path) + 1;
    size_t mode_len = strlen(mode) + 1;

    // Convert the path string from UTF-8 to wide char
    int w_path_len = MultiByteToWideChar(CP_UTF8, 0, path, path_len, NULL, 0);
    if (w_path_len <= 0) return NULL;
    wchar_t w_path[w_path_len];
    MultiByteToWideChar(CP_UTF8, 0, path, path_len, w_path, w_path_len);

    // Convert the mode string from UTF-8 to wide char
    int w_mode_len = MultiByteToWideChar(CP_UTF8, 0, mode, mode_len, NULL, 0);
    if (w_mode_len <= 0) return NULL;
    wchar_t w_mode[w_mode_len];
    MultiByteToWideChar(CP_UTF8, 0, mode, mode_len, w_mode, w_mode_len);
    
    // Open the file using the wide strings
    return _wfopen(w_path, w_mode);
}
#endif // _WIN32