#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <hb_save.h>
#include "config.h"

// Whether we are using the loader to open the application
bool using_loader;

static void activate( GtkApplication* app, gpointer user_data )
{
    GtkWidget *window;
    
    // Create the application window
    window = gtk_application_window_new(app);
    gtk_window_set_icon_from_file(GTK_WINDOW(window), WINDOW_ICON, NULL);
    gtk_window_set_title( GTK_WINDOW(window), WINDOW_TITLE );
    gtk_window_set_default_size( GTK_WINDOW(window), WINDOW_WIDTH, WINDOW_HEIGHT );
    gtk_widget_set_size_request(window, WINDOW_MIN_WIDTH, WINDOW_MIN_HEIGHT);  // Minimum dimensions of the window
    gtk_container_set_border_width(GTK_CONTAINER(window), WINDOW_BORDER);

    // Confirm if the user wants to close the editor when there is unsaved data
    g_signal_connect(GTK_WINDOW(window), "delete-event", G_CALLBACK(hb_confirm_close), NULL);

    // Store the pointer of the main window
    // (this is a workaround for Linux, so callback functions can properly deal with the window)
    hb_bind_main_window(GTK_WINDOW(window));

    // Ask the user what to do if the save file could not be opened during the program's startup
    while (!hb_save_is_open) hb_failed_to_open_default_save(GTK_WINDOW(window));

    // Check if the current save file has been changed by another program
    g_signal_connect_after(GTK_WINDOW(window), "focus-out-event", G_CALLBACK(hb_file_has_changed), GTK_WINDOW(window));
    g_signal_connect_after(GTK_WINDOW(window), "focus-in-event", G_CALLBACK(hb_file_has_changed), GTK_WINDOW(window));

    // ******************************************************************
    // Open a Heartbound save that is dragged into the application window
    // ******************************************************************

    // Set the main window as a target for dropped items
    GtkTargetEntry drop_target[1];
    drop_target[0] = *gtk_target_entry_new("text/uri-list", GTK_TARGET_OTHER_APP, 0);
    /* Note: the first and last arguments are just arbitrary identifiers,
             what really matters is the second argument (the target). */
    
    // Give the window the ability to receive dropped items
    gtk_drag_dest_set(
        window,
        GTK_DEST_DEFAULT_ALL,
        drop_target,
        1,
        GDK_ACTION_COPY
    );

    // Give the window the ability to receive file paths as the dropped items
    gtk_drag_dest_add_uri_targets(window);
    g_signal_connect(GTK_WINDOW(window), "drag-data-received", G_CALLBACK(hb_drag_and_drop_file), NULL);

    // **********************
    // Contents of the window
    // **********************

    // Create a wrapper for the window's contents
    GtkWidget *window_wrapper = gtk_box_new(GTK_ORIENTATION_VERTICAL, MENUBAR_SPACING);
    gtk_container_add(GTK_CONTAINER(window), window_wrapper);
    
    // Create a notebook with tabs for each of the game's chapter
    GtkWidget *notebook = gtk_notebook_new();
    GtkWidget *chapter_label[CHAPTER_AMOUNT];
    GtkWidget *chapter_page[CHAPTER_AMOUNT];
    GtkWidget *chapter_grid[CHAPTER_AMOUNT];

    // Create the wrapper for the menu and "file loaded" indicator, at the top of the window
    GtkWidget *top_wrapper = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(window_wrapper), top_wrapper);

    // Get whether the application prefers dark theme
    // (the default system-wide value for all GTK applications)
    gboolean prefers_dark_theme;
    g_object_get(
        gtk_settings_get_default(),
        "gtk-application-prefer-dark-theme",
        &prefers_dark_theme,
        NULL
    );

    // Get the global dark theme preference's value as a string
    char prefers_dark_theme_str[2];
    snprintf(prefers_dark_theme_str, 2, "%01d", prefers_dark_theme);

    // Get the dark theme preference for this specific program
    bool uses_dark_theme = hb_config_get("dark_mode", prefers_dark_theme_str)[0] != '0' ? true : false;

    // Set the dark or light theme for the program according to the its own configurations
    if (prefers_dark_theme != uses_dark_theme)
    {
        g_object_set(
            gtk_settings_get_default(),
            "gtk-application-prefer-dark-theme",
            uses_dark_theme,
            NULL
        );
    }

    // Set the style of the titles according to the theme
    char *title_style = uses_dark_theme ? CSS_TITLE_DARK : CSS_TITLE_LIGHT;
    GtkCssProvider *custom_css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(
        custom_css,
        title_style,
        -1,
        NULL
    );
    
    GdkScreen *screen = gdk_screen_get_default();
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(custom_css), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    // Create the accelerator group and add it to the window
    // (accelerators are the shortcut keys you press to use a feature)
    GtkAccelGroup *accel_group = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW (window), accel_group);

    // ******************************************
    // Create menu bar
    // ******************************************

    GtkWidget *menubar = gtk_menu_bar_new();
    gtk_widget_set_hexpand(menubar, TRUE);
    gtk_container_add(GTK_CONTAINER(top_wrapper), menubar);

    // Create indicator for when a file is saved or loaded
    GtkWidget *file_indicator = gtk_label_new(NULL);
    gtk_widget_set_no_show_all(file_indicator, TRUE);        // The widget is not shown by default
    gtk_widget_set_halign(file_indicator, GTK_ALIGN_END);    // The widget goes to the right side
    gtk_container_add(GTK_CONTAINER(top_wrapper), file_indicator);
    hb_bind_file_indicator(file_indicator);

    // Add a save button to the menu bar
    GtkWidget *save_button = gtk_menu_item_new();
    GtkWidget *save_icon = gtk_image_new_from_icon_name("document-save", GTK_ICON_SIZE_MENU);
    gtk_widget_set_tooltip_text(save_button, "Save file");
    gtk_container_add(GTK_CONTAINER(save_button), save_icon);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), save_button);
    g_signal_connect(GTK_MENU_ITEM(save_button), "enter-notify-event", G_CALLBACK(hb_menu_hover), NULL);
    g_signal_connect(GTK_MENU_ITEM(save_button), "leave-notify-event", G_CALLBACK(hb_menu_hover), NULL);
    g_signal_connect(GTK_MENU_ITEM(save_button), "button-press-event", G_CALLBACK(hb_save_file), GTK_WINDOW(window));

    // Add a open button to the menu bar
    GtkWidget *open_button = gtk_menu_item_new();
    GtkWidget *open_icon = gtk_image_new_from_icon_name("document-open", GTK_ICON_SIZE_MENU);
    gtk_widget_set_tooltip_text(open_button, "Open file");
    gtk_container_add(GTK_CONTAINER(open_button), open_icon);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), open_button);
    g_signal_connect(GTK_MENU_ITEM(open_button), "enter-notify-event", G_CALLBACK(hb_menu_hover), NULL);
    g_signal_connect(GTK_MENU_ITEM(open_button), "leave-notify-event", G_CALLBACK(hb_menu_hover), NULL);
    g_signal_connect(GTK_MENU_ITEM(open_button), "button-press-event", G_CALLBACK(hb_open_file), GTK_WINDOW(window));

    // Add a separator to the right of the Save/Open buttons
    GtkWidget *buttons_separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), buttons_separator);

    // Create top level menus

    // Menus at the top of the window
    GtkWidget *file_top = gtk_menu_item_new_with_mnemonic("_File");
    GtkWidget *edit_top = gtk_menu_item_new_with_mnemonic("_Edit");
    GtkWidget *help_top = gtk_menu_item_new_with_mnemonic("_Help");

    // Submenus that popup when the top menus are clicked
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *edit_menu = gtk_menu_new();
    GtkWidget *help_menu = gtk_menu_new();

    // Add the submenus to their respective menus
    {
        GtkWidget *top_level[3] = {file_top, edit_top, help_top};
        GtkWidget *menu[3] = {file_menu, edit_menu, help_menu};
        for (size_t i = 0; i < 3; i++)
        {
            gtk_menu_shell_append(GTK_MENU_SHELL(menubar), top_level[i]);
            gtk_menu_item_set_submenu(GTK_MENU_ITEM(top_level[i]), menu[i]);
        }
    }

    // Add the options to the submenus
    {
        GtkWidget *menu_item, *menu_separator;

        #define NEW_OPTION(name, menu, function) \
            menu_item = gtk_menu_item_new_with_mnemonic(name);\
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);\
            g_signal_connect(GTK_MENU_ITEM(menu_item), "activate", G_CALLBACK(function), GTK_WINDOW(window))
        
        #define NEW_SHORTCUT(key, modifier) \
            gtk_widget_add_accelerator(menu_item, "activate", accel_group, key, modifier, GTK_ACCEL_VISIBLE)
        
        #define NEW_SEPARATOR(menu) \
            menu_separator = gtk_separator_menu_item_new();\
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_separator)

        // File menu
        NEW_OPTION("_New...", file_menu, hb_menu_file_new);
        NEW_SHORTCUT(GDK_KEY_n, GDK_CONTROL_MASK);
        NEW_OPTION("_Open...", file_menu, hb_menu_file_open);
        NEW_SHORTCUT(GDK_KEY_o, GDK_CONTROL_MASK);
        NEW_OPTION("Open _default file", file_menu, hb_menu_file_open_default);
        NEW_SHORTCUT(GDK_KEY_o, GDK_CONTROL_MASK | GDK_SHIFT_MASK);
        NEW_SEPARATOR(file_menu);
        NEW_OPTION("_Save", file_menu, hb_menu_file_save);
        NEW_SHORTCUT(GDK_KEY_s, GDK_CONTROL_MASK);
        NEW_OPTION("Save _as...", file_menu, hb_menu_file_save_as);
        NEW_SHORTCUT(GDK_KEY_a, GDK_CONTROL_MASK);
        NEW_OPTION("Save to default _file", file_menu, hb_menu_file_save_to_default);
        NEW_SHORTCUT(GDK_KEY_s, GDK_CONTROL_MASK | GDK_SHIFT_MASK);
        NEW_SEPARATOR(file_menu);
        // TO DO for a future update:
        // NEW_OPTION("_Backup and restore...", file_menu, placeholder);
        // NEW_SEPARATOR(file_menu);
        NEW_OPTION("E_xit", file_menu, hb_menu_file_exit);
        NEW_SHORTCUT(GDK_KEY_F4, GDK_MOD1_MASK);

        // Edit menu
        NEW_OPTION("_Reload current saved data", edit_menu, hb_menu_edit_reload);
        NEW_SHORTCUT(GDK_KEY_F5, 0);
        NEW_OPTION("_Clear current saved data", edit_menu, hb_menu_edit_clear);
        NEW_SHORTCUT(GDK_KEY_Delete, GDK_SHIFT_MASK);
        NEW_SEPARATOR(edit_menu);

        menu_item = gtk_check_menu_item_new_with_mnemonic("_Dark mode");
        gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), menu_item);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), uses_dark_theme);
        g_signal_connect(GTK_CHECK_MENU_ITEM(menu_item), "toggled", G_CALLBACK(hb_menu_edit_dark_mode), custom_css);
        NEW_SHORTCUT(GDK_KEY_F2, 0);
        
        menu_item = gtk_check_menu_item_new_with_mnemonic("_Automatic reloading");
        hb_automatic_reloading = (hb_config_get("automatic_reloading", CFG_AUTOMATIC_RELOADING)[0] == '0') ? false : true;
        gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), menu_item);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), hb_automatic_reloading);
        g_signal_connect(GTK_CHECK_MENU_ITEM(menu_item), "toggled", G_CALLBACK(hb_edit_automatic_reloading), NULL);
        NEW_SHORTCUT(GDK_KEY_F3, 0);

        // Help menu
        NEW_OPTION("_Help...", help_menu, hb_menu_help_help);
        NEW_SHORTCUT(GDK_KEY_F1, 0);
        NEW_OPTION("_Download page...", help_menu, hb_menu_help_download);
        NEW_SEPARATOR(help_menu);
        NEW_OPTION("_About...", help_menu, hb_menu_help_about);

        #undef NEW_OPTION
        #undef NEW_SEPARATOR
    }

    // ***************************************************
    // Create the chapters tabs and their respective pages
    // ***************************************************

    for (int i = 0; i < CHAPTER_AMOUNT; i++)
    {
        // Create chapter page
        chapter_label[i] = gtk_label_new(hb_chapter[i]);        // Notebook tab for the chapter window
        chapter_page[i] = gtk_scrolled_window_new(NULL, NULL);  // Scrollable window for the grid
        chapter_grid[i] = gtk_grid_new();                       // Grid with the contents of the chapter

        // Add margins and spacing
        // gtk_container_set_border_width(GTK_CONTAINER(chapter_page[i]), PAGE_BORDER);    // Add a margin around the page
        gtk_widget_set_margin_start(chapter_grid[i], PAGE_BORDER);
        gtk_widget_set_margin_end(chapter_grid[i], PAGE_BORDER);
        gtk_grid_set_column_spacing(GTK_GRID(chapter_grid[i]), GRID_COLUMN_SPACING);
        gtk_grid_set_row_spacing(GTK_GRID(chapter_grid[i]), GRID_ROW_SPACING);
        
        // Add the grid to the scrollable window (chapter page)
        gtk_container_add(GTK_CONTAINER(chapter_page[i]), chapter_grid[i]);

        // Add the chapter page to the notebook
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook), chapter_page[i], chapter_label[i]);
    }

    // Add the notebook to the application window
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);  // The tabs are located at the top of the window
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);      // The tabs can be scrolled if they do not fit the window
    gtk_widget_set_vexpand(notebook, TRUE);                         // The notebook fills the remaining horizontal space on the window
    gtk_container_add(GTK_CONTAINER(window_wrapper), notebook);     // Add the notebook to the window wrapper
    
    // Allocate the buffer for the text
    char *restrict text_buffer = malloc( TEXT_BUFFER_SIZE * sizeof(char) );
    if (text_buffer == NULL)
    {
        fprintf(stderr, "Not enough memory to run Heartbound Save Editor.\n");
        exit(EXIT_FAILURE);
    }

    // Create the groups of save entries on each page
    for (size_t i = 0; i < hb_locations_amount; i++)
    {
        // Get the current location from the list
        HeartboundLocation my_location = hb_location_list[i];
        size_t my_world = my_location.world;            // To which page it goes
        size_t my_position = my_location.position * 2;  // Position on the grid

        /* Note:
            Each location occupies two rows and two columns on the grid.
            The label goes to the first row (width of two columns).
            The image goes to the second row, first column.
            The remaining cell (second row of the second column) will be for for the save's variables.
        */

        // Create label with the location's name
        GtkWidget *my_label = gtk_label_new(my_location.name);
        gtk_widget_set_name(my_label, "title-label");            // CSS name for styling the title
        gtk_widget_set_halign(my_label, GTK_ALIGN_START);        // Align the label's text to the left

        // Load texture from file
        GdkPixbuf *my_texture = gdk_pixbuf_new_from_file(my_location.image, NULL);
        
        // Scale down the texture if it is bigger than the maximum size
        // (while keeping the aspect ratio)
        double my_width = (double)gdk_pixbuf_get_width(my_texture);
        double my_height = (double)gdk_pixbuf_get_height(my_texture);
        double my_ratio = my_width / my_height;
        if (my_height > my_width)
        {
            my_height = MIN(my_height, IMAGE_HEIGHT);
            my_width = my_height * my_ratio;
        }
        else
        {
            my_width = MIN(my_width, IMAGE_WIDTH);
            my_height = my_width * (1.0 / my_ratio);
        }

        GdkPixbuf *my_texture_resized = gdk_pixbuf_scale_simple(my_texture, my_width, my_height, GDK_INTERP_BILINEAR);
        g_object_unref(my_texture);
        
        // Create an image from the texture
        GtkWidget *my_image = gtk_image_new_from_pixbuf(my_texture_resized);
        g_object_unref(my_texture_resized);

        // Margin and alignment of the image
        gtk_widget_set_margin_start(my_image, IMAGE_MARGIN);
        gtk_widget_set_valign(my_image, GTK_ALIGN_START);

        // Add the image and labels to the window
        gtk_grid_attach(GTK_GRID(chapter_grid[my_world]), my_label, 0, my_position, 2, 1);
        gtk_grid_attach(GTK_GRID(chapter_grid[my_world]), my_image, 0, my_position+1, 1, 1);

        // Create a box for the save entries of the location
        GtkWidget *my_contents = gtk_box_new(GTK_ORIENTATION_VERTICAL, ENTRY_VERTICAL_SPACING);
        gtk_widget_set_hexpand(my_contents, TRUE);
        gtk_grid_attach(GTK_GRID(chapter_grid[my_world]), my_contents, 1, my_position+1, 1, 1);
    }

    // *********************************
    // Add the save entries to each page
    // *********************************

    for (size_t var = 0; var < NUM_STORY_VARS; var++)
    {
        /*
            Each entry will be wrapped inside a box. This wrapper has a label to the left,
            with the name of the entry, and a flow box to the right, with the options of
            the entry.
            
            The wrapper is packed vertically on its grid cell, and the wrapper contents
            are packed horizontally. The flow box with the options expands horizontally
            to fill the remaining window space.
        */
        
        if (hb_save_data[var].used)
        {
            // Get the chapter and window position
            HeartboundLocation *my_location = hb_get_location(hb_save_data[var].location);
            if (my_location == NULL) continue;  // Skip the entry if its location was not found
            size_t my_chapter = my_location->world;
            size_t my_position = my_location->position * 2 + 1;

            // Get the box on the grid cell where the entry should go
            GtkWidget *my_cell = gtk_grid_get_child_at( GTK_GRID(chapter_grid[my_chapter]), 1, my_position );

            // Create the wrapper box to hold the entry's widgets (name label and options)
            GtkWidget *my_wrapper = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, ENTRY_VERTICAL_SPACING);
            gtk_widget_set_hexpand(my_wrapper, TRUE);

            // Add the wrapper to the cell
            gtk_container_add(GTK_CONTAINER(my_cell), my_wrapper);

            // Copy the entry's name to the text buffer
            if (hb_save_data[var].name != NULL)
            {strncpy(text_buffer, hb_save_data[var].name, TEXT_BUFFER_SIZE);}
            else
            {text_buffer[0] == '\0';}
            
            // If the entry has additional info, append it to the text buffer
            if (hb_save_data[var].info != NULL)
            {
                strncat(text_buffer, " (", TEXT_BUFFER_SIZE-1);
                strncat(text_buffer, hb_save_data[var].info, TEXT_BUFFER_SIZE-1);
                strncat(text_buffer, ")", TEXT_BUFFER_SIZE-1);
            }

            // Append a colon to the label's text
            strncat(text_buffer, ":", TEXT_BUFFER_SIZE-1);
            
            // Create a name label with the string on the text buffer
            GtkWidget *my_name_label = gtk_label_new(text_buffer);
            gtk_widget_set_valign(my_name_label, GTK_ALIGN_START);
            gtk_widget_set_margin_end(my_name_label, ENTRY_HORIZONTAL_SPACING);
            gtk_widget_set_margin_top(my_name_label, 8);    // Added some top margin so the label's text align with the options
                                                            // (for whatever reason, just aligning both to the top was not enough)

            // Add the name label to the entry's box
            gtk_container_add(GTK_CONTAINER(my_wrapper), my_name_label);

            // Add a tooltip to the name label with the variable's index
            snprintf(text_buffer, TEXT_BUFFER_SIZE, "Story Variable #%lu", hb_save_data[var].index);
            gtk_widget_set_tooltip_text(my_name_label, text_buffer);

            // Create a flow box for the options
            GtkWidget *my_options = gtk_flow_box_new();
            gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(my_options), GTK_SELECTION_NONE);  // Prevents the text inside from getting highlighted when you click on them
            gtk_widget_set_valign(my_options, GTK_ALIGN_START);
            gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(my_options), 2);

            // Add the flow box to the wrapper
            gtk_container_add(GTK_CONTAINER(my_wrapper), my_options);
            
            // The type of entry field
            /*  If the '.num_entries' attribute is set to 0, then it means that
                the field accept any value (a text field will be used).
                If '.num_entries' is 2 or more, then the appropriate number of
                radio buttons will be used.
                If the variable does not specify a measurement unit and a number
                of values, then 'No' and 'Yes' radio buttons will be used. */
            if (hb_save_data[var].num_entries == 0 && (hb_save_data[var].unit != NULL || hb_save_data[var].maximum > 0.0))
            {
                // Create the text entry field
                GtkWidget *my_entry_field = gtk_entry_new();
                gtk_widget_set_margin_start(my_entry_field, TEXT_FIELD_MARGIN);
                gtk_container_add(GTK_CONTAINER(my_options), my_entry_field);

                // Set the properties of the text entry field
                snprintf(text_buffer, TEXT_BUFFER_SIZE, "%.0f", hb_save_data[var].value);   // Convert the variable's value to string
                gtk_entry_set_text(GTK_ENTRY(my_entry_field), text_buffer);                 // Add the variable's value (as string) to the field
                gtk_entry_set_width_chars(GTK_ENTRY(my_entry_field), TEXT_FIELD_WIDTH);     // Set the width of the field
                gtk_entry_set_max_length(GTK_ENTRY(my_entry_field), TEXT_FIELD_MAX_CHARS);  // Maximum amount of characters the field can have
                gtk_entry_set_placeholder_text(GTK_ENTRY(my_entry_field), "0");             // Display '0' if the field is empty and unfocused
                
                // Add the measurement unit to the left of the text field
                if (hb_save_data[var].unit != NULL)
                {
                    GtkWidget *my_unit_label = gtk_label_new(hb_save_data[var].unit);
                    gtk_container_add(GTK_CONTAINER(my_options), my_unit_label);
                }

                // Filter the characters and update the storyline variable when the entry's content changes
                // (Only digits from 0 to 9 are accepted)
                g_signal_connect(
                    GTK_ENTRY(my_entry_field),          // The entry field
                    "changed",                          // Event to be listened
                    G_CALLBACK(hb_setvar_text_entry),   // Function to remove non-digits characters and update the storyline variable
                    &hb_save_data[var]                  // Pointer to the storyline variable
                );

                // Add a pointer to the entry field on the variable's data structure
                hb_save_data[var].widget.entry = GTK_ENTRY(my_entry_field);
            }
            else if (hb_save_data[var].num_entries >= 2)
            {
                GtkWidget *my_radio_button = NULL;
                GtkWidget *previous_button = NULL;
                
                // Create radio buttons for each possible value
                for (size_t j = 0; j < hb_save_data[var].num_entries; j++)
                {
                    /*
                        The value of the button is the actual numeric value (as a string) on the save file.
                        The alias of the button is the user friendly name of the value.
                        The text of the button will be the alias, if one is available, otherwise the value string.
                    */
                    char *my_alias = hb_save_data[var].aliases[j].description;
                    char *my_value = hb_save_data[var].aliases[j].header != NULL ? *hb_save_data[var].aliases[j].header : hb_save_headers[COLUMN_OFFSET + j];
                    char *my_text = my_alias != NULL ? my_alias : my_value;
                    
                    // Create a radio button with a text label on the group of the current storyline variable
                    my_radio_button = gtk_radio_button_new_with_label(NULL, my_text);
                    gtk_radio_button_join_group(GTK_RADIO_BUTTON(my_radio_button), GTK_RADIO_BUTTON(previous_button));
                    previous_button = my_radio_button;

                    // Set to active the radio button that corresponds to the variable's value on the save file
                    if (hb_save_data[var].value == atof(my_value))
                    {
                        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(my_radio_button), TRUE);
                    }

                    // Add the radio button to the flow box
                    gtk_container_add(GTK_CONTAINER(my_options), my_radio_button);

                    // Change the storyline variable value when the button is toggled on
                    g_signal_connect(
                        GTK_RADIO_BUTTON(my_radio_button),      // The radio button
                        "toggled",                              // Event to be listened
                        G_CALLBACK(hb_setvar_radio_button),     // Function to change a variable value
                        &hb_save_data[var]                      // Pointer to the storyline variable
                    );
                }

                // Add a pointer to the group of radio buttons to the variable's data structure
                hb_save_data[var].widget.group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(previous_button));
            }
            else    // If nothing else matches, just use No/Yes radio buttons
            {
                GtkWidget *my_radio_button = NULL;
                GtkWidget *previous_button = NULL;

                char *my_text[2] = {"No", "Yes"};
                double my_value[2] = {0.0, 1.0};

                for (size_t i = 0; i < 2; i++)
                {
                    // Create a radio button within the group of the current storyline variable
                    my_radio_button = gtk_radio_button_new_with_label(NULL, my_text[i]);
                    gtk_radio_button_join_group(GTK_RADIO_BUTTON(my_radio_button), GTK_RADIO_BUTTON(previous_button));
                    previous_button = my_radio_button;

                    // Set to active the radio button that corresponds to the variable's value on the save file
                    if (hb_save_data[var].value == my_value[i])
                    {
                        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(my_radio_button), TRUE);
                    }

                    // Add the radio button to the flow box
                    gtk_container_add(GTK_CONTAINER(my_options), my_radio_button);

                    // Change the storyline variable value when the button is toggled on
                    g_signal_connect(
                        GTK_RADIO_BUTTON(my_radio_button),      // The radio button
                        "toggled",                              // Event to be listened
                        G_CALLBACK(hb_setvar_no_yes),           // Function to change a variable value
                        &hb_save_data[var]                      // Pointer to the storyline variable
                    );
                }

                // Add a pointer to the group of radio buttons to the variable's data structure
                hb_save_data[var].widget.group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(previous_button));
            }
        }
    }

    // ************************************************
    // Add the player's attributes to the 'Global' page
    // ************************************************

    {
        // Get the chapter and window position
        HeartboundLocation *my_location = hb_get_location("Lore");
        if (my_location == NULL) goto finish;
        size_t my_chapter = my_location->world;
        size_t my_position = my_location->position * 2 + 1;

        // Create and add a new box with a label (for the name of an option)
        GtkWidget *my_cell = gtk_grid_get_child_at( GTK_GRID(chapter_grid[my_chapter]), 1, my_position );
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
            gtk_container_add(GTK_CONTAINER(my_cell), my_wrapper)
        
        // Create and add flow box for the contents
        GtkWidget *my_flowbox;
        #define NEW_FLOWBOX() \
            my_flowbox = gtk_flow_box_new();\
            gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(my_flowbox), GTK_SELECTION_NONE);\
            gtk_widget_set_valign(my_flowbox, GTK_ALIGN_START);\
            gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(my_flowbox), 2);\
            gtk_container_add(GTK_CONTAINER(my_wrapper), my_flowbox);

        // Create and add the entries for the attributes
        GtkWidget *my_label;
        GtkWidget *my_entry;
        #define NEW_ENTRY(label) \
            my_label = gtk_label_new(label);\
            my_entry = gtk_entry_new();\
            gtk_widget_set_margin_start(my_label, TEXT_FIELD_MARGIN);\
            gtk_entry_set_width_chars(GTK_ENTRY(my_entry), TEXT_FIELD_WIDTH);\
            gtk_entry_set_max_length(GTK_ENTRY(my_entry), TEXT_FIELD_MAX_CHARS);\
            gtk_entry_set_placeholder_text(GTK_ENTRY(my_entry), "0");\
            gtk_container_add(GTK_CONTAINER(my_flowbox), my_label);\
            gtk_container_add(GTK_CONTAINER(my_flowbox), my_entry)
        
        // Create the wrapper box for the room
        NEW_LABEL_BOX("Room :");
        gtk_widget_set_tooltip_text(
            my_name_label,
            "The location where you will spaw.\n"
            "Changing this automatically update the coordinates, "
            "so you do not end up out of bounds or inside walls."
        );
        
        // Create the dropdown list for the room names
        GtkWidget *room_selection = gtk_combo_box_text_new();
        gtk_container_add(GTK_CONTAINER(my_wrapper), room_selection);
        gtk_widget_set_margin_start(room_selection, TEXT_FIELD_MARGIN);

        // Add the rooms to the dropdown list
        for (size_t room = 0; room < hb_rooms_amount; room++)
        {
            // Append the room name to the list
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(room_selection), hb_room_list[room].name);
        }

        // Set the current room as selected in the list
        HeartboundRoom *current_room = hb_get_room(hb_room_id);
        if (current_room != NULL)
        {
            gtk_combo_box_set_active(GTK_COMBO_BOX(room_selection), current_room->index);
        }

        // Update the coordinates entries when a room is selected
        // (so the player do not spaw stuck out of bounds or in walls)
        g_signal_connect(GTK_COMBO_BOX(room_selection), "changed", G_CALLBACK(hb_set_coordinates_from_room), NULL);

        // The list gets updated when another save file is loaded
        hb_bind_widgets(GTK_COMBO_BOX(room_selection), NULL, NULL);
        
        // Create a wrapper box for the coordinates
        NEW_LABEL_BOX("Coordinates :");
        NEW_FLOWBOX();
        gtk_widget_set_tooltip_text(
            my_name_label,
            "The point in the room where you will spaw.\n"
            "(0, 0) being the top left.\n"
            "X grows towards the right, Y grow towards the bottom."
        );

        // Create the entries for the coordinates
        
        NEW_ENTRY("X =");
        snprintf(text_buffer, TEXT_BUFFER_SIZE, "%.0f", hb_x_axis);
        gtk_entry_set_text(GTK_ENTRY(my_entry), text_buffer);
        hb_bind_xy_entries(GTK_ENTRY(my_entry), NULL);  // Bind the X entry to the room selection list        
        
        // Update the variables for the X coordinate when its field change
        g_signal_connect(GTK_ENTRY(my_entry), "changed", G_CALLBACK(hb_setvar_player_attribute), &hb_x_axis);
      
        NEW_ENTRY("Y =");
        snprintf(text_buffer, TEXT_BUFFER_SIZE, "%.0f", hb_y_axis);
        gtk_entry_set_text(GTK_ENTRY(my_entry), text_buffer);
        hb_bind_xy_entries(NULL, GTK_ENTRY(my_entry));  // Bind the Y entry to the room selection list
        
        // Update the variables for the X coordinate when its field change
        g_signal_connect(GTK_ENTRY(my_entry), "changed", G_CALLBACK(hb_setvar_player_attribute), &hb_y_axis);

        // Create a wrapper box for the Hit Points
        NEW_LABEL_BOX("Hit Points :");
        NEW_FLOWBOX();
        gtk_widget_set_tooltip_text(
            my_name_label,
            "Your health in combat.\n"
            "Either by design or by bug, the game seems to ignore these values at some points."
        );
        
        // Create the entries for the Hit Points
        
        NEW_ENTRY("Current =");
        snprintf(text_buffer, TEXT_BUFFER_SIZE, "%.0f", hb_hitpoints_current);
        gtk_entry_set_text(GTK_ENTRY(my_entry), text_buffer);
        g_signal_connect(GTK_ENTRY(my_entry), "changed", G_CALLBACK(hb_setvar_player_attribute), &hb_hitpoints_current);
        hb_bind_hp_entries(GTK_ENTRY(my_entry), NULL);  // Bind the HP fields to each other so the current HP is capped by the maximum
        
        NEW_ENTRY("Maximum =");
        snprintf(text_buffer, TEXT_BUFFER_SIZE, "%.0f", hb_hitpoints_maximum);
        gtk_entry_set_text(GTK_ENTRY(my_entry), text_buffer);
        g_signal_connect(GTK_ENTRY(my_entry), "changed", G_CALLBACK(hb_setvar_player_attribute), &hb_hitpoints_maximum);
        hb_bind_hp_entries(NULL, GTK_ENTRY(my_entry));  // Bind the HP fields to each other so the current HP is capped by the maximum

        // Create wrapper box for the known glyphs
        NEW_LABEL_BOX("Known glyphs :");
        NEW_FLOWBOX();
        gtk_widget_set_tooltip_text(
            my_name_label,
            "Automatically translates the coded text.\n"
            "For example, the conversation between Binder and Barghest."
        );

        // Create the radio buttons for the known glyphs

        {
            // The 3 possible values of the buttons
            char *glyph_labels[3] = {"None", "Guardian", "Guardian and Darksider"};
            
            // Create and add the 3 buttons
            GtkWidget *my_radio_button = NULL;
            GtkWidget *previous_button = NULL;
            
            for (size_t value = 0; value < 3; value++)
            {
                // Create a button with one of the labels
                my_radio_button = gtk_radio_button_new_with_label(NULL, glyph_labels[value]);
                gtk_radio_button_join_group(GTK_RADIO_BUTTON(my_radio_button), GTK_RADIO_BUTTON(previous_button));
                previous_button = my_radio_button;

                // Add the button to the same group as the other buttons
                gtk_container_add(GTK_CONTAINER(my_flowbox), my_radio_button);

                // Set the button as active if it corresponds to the current 'hb_known_glyphs' value
                if (value == hb_known_glyphs)
                {
                    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(my_radio_button), TRUE);
                }

                // Update the 'hb_known_glyphs' variable when the button is toggled
                g_signal_connect(GTK_RADIO_BUTTON(my_radio_button), "toggled", G_CALLBACK(hb_setvar_known_glyphs), &hb_known_glyphs);
            }

            // The glyphs buttons get updated when another save file is loaded
            hb_bind_widgets(NULL, gtk_radio_button_get_group(GTK_RADIO_BUTTON(my_radio_button)), NULL);
        }

        // Create a wrapper for the Game Seed
        NEW_LABEL_BOX("Game Seed :");
        NEW_FLOWBOX();
        gtk_widget_set_tooltip_text(
            my_name_label,
            "Affects the patterns of vegetation and objects on the ground, some flavor texts, "
            "and a few random events (like the notes you find)."
        );

        // Create a text entry for the Game Seed
        my_entry = gtk_entry_new();
        gtk_widget_set_margin_start(my_entry, TEXT_FIELD_MARGIN);
        gtk_entry_set_width_chars(GTK_ENTRY(my_entry), SEED_SIZE-2);
        gtk_entry_set_max_length(GTK_ENTRY(my_entry), SEED_SIZE-2);
        gtk_entry_set_placeholder_text(GTK_ENTRY(my_entry), "random");
        gtk_container_add(GTK_CONTAINER(my_flowbox), my_entry);
        snprintf(text_buffer, sizeof(hb_game_seed), "%s", hb_game_seed);
        gtk_entry_set_text(GTK_ENTRY(my_entry), text_buffer);

        // Update the game seed variable when its field changes
        g_signal_connect(GTK_ENTRY(my_entry), "changed", G_CALLBACK(hb_setvar_game_seed), &hb_game_seed);

        // The game seed entry gets updated when another save file is loaded
        hb_bind_widgets(NULL, NULL, GTK_ENTRY(my_entry));

        finish:
        NULL;
        #undef NEW_LABEL_BOX
        #undef NEW_ENTRY
        #undef NEW_FLOWBOX
    }

    // Update the title of the window with the path of the save file
    hb_update_window_title(GTK_WINDOW(window));

    // Deallocate the text buffer
    free(text_buffer);
    
    // Render the application window and all its children
    gtk_widget_show_all(window);
}

int main ( int argc, char **argv )
{
    // Find the directory of our executable
    char *editor_path = realpath(argv[0], NULL);  // Full path including the executable itself

    // Strip the executable name from the path in order to get its directory
    int path_len = strnlen(editor_path, PATH_BUFFER);  // Length of the program's path
    for (int i = path_len - 1; i >= 0; i--)
    {
        // Move backwards from the end of the path until a backslash or slash character is found
        if ( (editor_path[i] == '/') || (editor_path[i] == '/') )
        {
            editor_path[i] = '\0';
            break;
        }
    }

    // Change the current working directory to the executable directory
    int chdir_status = chdir(editor_path);
    free(editor_path);

    // Check if a file to be opened has been provided as an argument
    char *open_path;
    using_loader = false;  // Whether we are using the loader to open the application
    if (argc >= 2)
    {
        // Check if we are using the loader
        if (argc >= 3)
        {
            // The loader appends the "--loader" to the arguments
            if (strncmp(argv[argc-1], "--loader", 9) == 0)
            {
                // If the last argument is "--loader", then it is flagged as being used
                using_loader = true;
            }
        }

        // Use the first command line argument as the file path, if there is one
        if (strncmp(argv[1], "--loader", 9) != 0)
        {
            // If the first argument is not "--loader", use it as the path
            open_path = argv[1];
        }
        else
        {
            // Otherwise use the default path
            open_path = SAVE_PATH;
        }
    }
    else
    {
        // Use the default file path, if no path was provided
        open_path = SAVE_PATH;
    }
    
    // Open the save file
    hb_open_save(open_path);

    GtkApplication *app;
    int status;

    // Start and register a new instance of Heartbound Save Editor
    // Note: New instances of the application can be opened by just running it
    //       Each instance works independently from each other and has its own memory.
    app = gtk_application_new( "com.github.tbpaolini.hbsaveedit", G_APPLICATION_NON_UNIQUE );
    g_application_register(G_APPLICATION(app), NULL, NULL);
    g_set_application_name(WINDOW_TITLE);
    g_set_prgname(WINDOW_TITLE);
    g_signal_connect( app, "activate", G_CALLBACK(activate), NULL );

    status = g_application_run( G_APPLICATION(app), 1, argv );
    /* Note:
        The value 1 is being passed to the 'argc' argument of 'g_application_run'
        in order to prevent GTK from throwing an error in case there is a command
        line argument given to the application.

        There are proper ways to set up GTK to handle command lines arguments, but
        I found them overly complicated, since all that I want is to use argv[1] as
        the path of the file to be opened (which is handled by my code, not GTK).

        For me it is far simpler to make GTK to "think" that there are no additional
        arguments, than going through a complex chain of callback functions for just
        making GTK to play nice with an argument that it will not use to begin with.

        P.S.: At a futute update I might go through GTK's own system of arguments
        handling, if the program happens to need handling arguments other than the
        file path.
    */

    // Do garbage collection and then exit gracefully :)
    g_object_unref(app);
    hb_close_save();

    return status;
}