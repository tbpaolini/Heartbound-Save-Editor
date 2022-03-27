#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <gtk\gtk.h>
#include <unistd.h>
#include <hb_save.h>
// #include <windows.h>

#define WINDOW_WIDTH 720
#define WINDOW_HEIGHT 540
#define WINDOW_BORDER 10
#define WINDOW_TITLE "Heartbound Save Editor"
#define WINDOW_ICON "..\\lib\\icon.png"

#define PAGE_BORDER 10
#define GRID_ROW_SPACING 15
#define GRID_COLUMN_SPACING 5
#define IMAGE_WIDTH 80
#define IMAGE_HEIGHT 80
#define IMAGE_MARGIN 10

#define TEXT_BUFFER_SIZE 1024

static void activate( GtkApplication* app, gpointer user_data )
{
    GtkWidget *window;
    
    // Create the application window
    window = gtk_application_window_new(app);
    gtk_window_set_icon_from_file(GTK_WINDOW(window), WINDOW_ICON, NULL);
    gtk_window_set_title( GTK_WINDOW(window), WINDOW_TITLE );
    gtk_window_set_default_size( GTK_WINDOW(window), WINDOW_WIDTH, WINDOW_HEIGHT );
    gtk_container_set_border_width(GTK_CONTAINER(window), WINDOW_BORDER);
    
    // Create a notebook with tabs for each of the game's chapter
    GtkWidget *notebook = gtk_notebook_new();
    GtkWidget *chapter_label[CHAPTER_AMOUNT];
    GtkWidget *chapter_page[CHAPTER_AMOUNT];
    GtkWidget *chapter_grid[CHAPTER_AMOUNT];

    // Create the chapters tabs and their respective pages
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
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
    gtk_container_add(GTK_CONTAINER(window), notebook);

    // Allocate the buffer for the text
    char *restrict text_buffer = malloc( TEXT_BUFFER_SIZE * sizeof(char) );

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
        GtkWidget *my_label = gtk_label_new(NULL);
        snprintf(
            text_buffer,        // Buffer where to copy the label text
            TEXT_BUFFER_SIZE,
            "<span size='140%' weight='bold' color='#3a6b7c'>"  // Format the location's name
            "%s"
            "</span>",
            my_location.name    // Location's name
        );
        gtk_label_set_markup(GTK_LABEL(my_label), text_buffer);  // Set the label from the text with formatting markup
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

        // Add a flow box to the right of the image
        GtkWidget *my_flowbox = gtk_flow_box_new();
        gtk_orientable_set_orientation(GTK_ORIENTABLE(my_flowbox), GTK_ORIENTATION_HORIZONTAL);
        gtk_grid_attach(GTK_GRID(chapter_grid[my_world]), my_flowbox, 1, my_position+1, 1, 1);
    }

    // Add the save entries to each page
    for (size_t i = 0; i < NUM_STORY_VARS; i++)
    {
        StorylineVars storyline_variable = hb_save_data[i];
        
        if (storyline_variable.used)
        {
            // Get the chapter and window position
            HeartboundLocation *my_location = hb_get_location(storyline_variable.location);
            if (my_location == NULL) continue;  // Skip the entry if its location was not found
            size_t my_chapter = my_location->world;
            size_t my_position = my_location->position * 2 + 1;

            // Get the flow box where the entry should go
            GtkWidget *my_flowbox = gtk_grid_get_child_at( GTK_GRID(chapter_grid[my_chapter]), 1, my_position );

            // Create a box to hold the entry's widgets (name label, text field, radio buttons)
            GtkWidget *myself = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
            gtk_container_add(GTK_CONTAINER(my_flowbox), myself);     // Add to the flow box

            // Copy the entry's name to the text buffer
            strcpy_s(text_buffer, TEXT_BUFFER_SIZE, storyline_variable.name);
            
            // If the entry has additional info, append it to the text buffer
            if (storyline_variable.info != NULL)
            {
                strcat_s(text_buffer, TEXT_BUFFER_SIZE, " (");
                strcat_s(text_buffer, TEXT_BUFFER_SIZE, storyline_variable.info);
                strcat_s(text_buffer, TEXT_BUFFER_SIZE, ")");
            }
            
            // Create a name label with the string on the text buffer
            GtkWidget *my_name_label = gtk_label_new(text_buffer);

            // Add the name label to the entry's box
            gtk_box_pack_start(GTK_BOX(myself), my_name_label, FALSE, FALSE, 0);
            // gtk_container_add(GTK_CONTAINER(myself), my_name_label);
            
            // The type of entry field
            /*  If the '.num_entries' attribute is set to 0, then it means that
                the field accept any value (a text field will be used).
                If '.num_entries' is 2 or more, then the appropriate number of
                radio buttons will be used. */
            if (storyline_variable.num_entries == 0)
            {
                /* code */
            }
            else if (storyline_variable.num_entries >= 2)
            {
                GtkWidget *my_radio_button = NULL;
                GtkWidget *previous_button = NULL;
                
                for (size_t j = 0; j < storyline_variable.num_entries; j++)
                {
                    char *my_alias = storyline_variable.aliases[j].description;
                    char *my_value = storyline_variable.aliases[j].header != NULL ? *storyline_variable.aliases[j].header : hb_save_headers[COLUMN_OFFSET + j];
                    char *my_text = my_alias != NULL ? my_alias : my_value;
                    
                    my_radio_button = gtk_radio_button_new_with_label(NULL, my_text);
                    gtk_radio_button_join_group(GTK_RADIO_BUTTON(my_radio_button), GTK_RADIO_BUTTON(previous_button));
                    previous_button = my_radio_button;

                    if (storyline_variable.value == atof(my_value))
                    {
                        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(my_radio_button), TRUE);
                    }

                    gtk_box_pack_start(GTK_BOX(myself), my_radio_button, FALSE, FALSE, 0);
                }
            }
        }
    }

    // Deallocate the text buffer
    free(text_buffer);
    
    // Render the application window and all its children
    gtk_widget_show_all(window);
}

int main ( int argc, char **argv )
{
    // Find the directory of our executable
    size_t path_len = strnlen_s(argv[0], 1024);  // Length of the program's path
    size_t path_pos = 0;
    for (size_t i = path_len - 1; i >= 0; i--)
    {
        // Move backwards from the end of the path until a backslash or slash character is found
        if ( (argv[0][i] == '\\') || (argv[0][i] == '/') )
        {
            path_pos = i;   // The position on the string where the program's directory ends
            break;
        }
    }

    // Change the current working directory to the executable directory
    char *path_dir = calloc(path_pos + 1, sizeof(char));
    memcpy(path_dir, argv[0], path_pos);
    chdir(path_dir);
    free(path_dir);

    hb_open_save();

    GtkApplication *app;
    int status;

    app = gtk_application_new( "com.github.tbpaolini.hbsaveedit", G_APPLICATION_FLAGS_NONE );
    g_signal_connect( app, "activate", G_CALLBACK(activate), NULL );
    status = g_application_run( G_APPLICATION(app), argc, argv );
    g_object_unref(app);
    hb_close_save();

    return status;
}