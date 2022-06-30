#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <gtk\gtk.h>

#include <hb_arg.h>
#include <hb_save_io.h>
#include <hb_gui_callback.h>

// This button set the values on the save file necessary during the "Deep Thought" puzzle of the ARG:
// https://gopiratesoftware.com/games/Heartbound/CORE_DUMP/MEMORY_ALPHA/CRASH_LOG/BOOT_RECORD/DEEP_THOUGHT/
GtkWidget *hb_arg_remember_button = NULL;

// Check if the user has completed the first step of the "Deep Thought" puzzle of the ARG
bool hb_arg_check_deep_thought()
{
    /* Note:
        We are checking if there is a file called 'painful.memory' at the save folder.
        That means the first step of the puzzle has been completed.
        
        If the player gets the worst possible ending of Heartbound and then choose
        "Forget" at the game's start screen, the file 'painful.memory' is created at
        the save folder.
    */
    
    static bool path_is_initialized = false;        // So we do not build the path more than once
    static char painful_memory_path[PATH_BUFFER];   // Path to the 'painful.memory' file

    if (!path_is_initialized)
    {
        // Build the path, if it was not built yet
        snprintf(painful_memory_path, PATH_BUFFER, "%s\\painful.memory", SAVE_ROOT);
        path_is_initialized = true;
    }

    // Check if a file exists at that path
    FILE *painful_memory_file = fopen(painful_memory_path, "r");
    
    if (painful_memory_file != NULL)
    {
        // Close the file and return 'true' if a file exists
        fclose(painful_memory_file);
        return true;
    }
    else
    {
        // Return 'false' if there is not a file
        return false;
    }
}

// Create the "Remember painful memory" button
void hb_arg_remember_button_init()
{
    static bool button_is_initialized = false;  // So we do not create the button more than once
    
    if (!button_is_initialized)
    {
        // Create the button, if it was not created yet
        hb_arg_remember_button = gtk_button_new_with_label("Remember painful memory");
        
        // Set the tooltip text of the button
        gtk_widget_set_tooltip_markup(
            hb_arg_remember_button,
            "This is part of the Heartbound ARG (Alternate Reality Game).\n"
            "The button sets the values necessary for progressing to the next ARG step, after you got the worst game ending.\n"
            "After you set and saved the values, in the game go to <b>animus_turtlefarm_1</b> and check the bubbles there."
        );
        
        // The button is not remains hidden if the user has not completed the ARG step yet
        bool puzzle_completed = hb_arg_check_deep_thought();
        if (!puzzle_completed)
        {
            gtk_widget_set_no_show_all(hb_arg_remember_button, TRUE);
        }

        // Set the button's functionality
        g_signal_connect(
            hb_arg_remember_button,
            "clicked",
            G_CALLBACK(hb_remember_painful_memory),
            NULL
        );
        
        button_is_initialized = true;
    }
}

// Change the values of storyline variables 400 to 407, as required to proceed on the "Deep Thought" puzzle of the ARG
void hb_remember_painful_memory(GtkWidget *widget)
{
    // Check all checkboxes at the Turtle Farm's grid
    hb_turtlefarm_check_all(NULL);

    // Set the variables to the values that the game expects
    // (those values were found at the 'painful.memory' file)
    hb_save_data[400].value = atof("20091305120914049516247267147776");
    hb_save_data[401].value = atof("325031205051404124160");
    hb_save_data[402].value = atof("61518070520061514865630583380246528");
    hb_save_data[403].value = atof("325031205020507058387025920");
    hb_save_data[404].value = atof("25152111140523202139376542962381595804172288");
    hb_save_data[405].value = atof("25152108010425151453030124617728");
    hb_save_data[406].value = atof("25152103081519050043749574901760");
    hb_save_data[407].value = atof("20091305071815232707362816");

    // Show a dialog to indicate the changes
    GtkWidget *info_dialog = hb_create_dialog_with_title_and_image(
        GTK_WINDOW(gtk_widget_get_toplevel(widget)),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "Deep Thought - ARG (Alternate Reality Game)",
        "dialog-information",
        "Lore has remembered what he did to everyone..."
    );

    gtk_dialog_run(GTK_DIALOG(info_dialog));
    gtk_widget_destroy(info_dialog);
}