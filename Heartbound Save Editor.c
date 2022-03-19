#include <stdio.h>
#include <gtk\gtk.h>
#include <unistd.h>
#include <hb_save.h>
// #include <windows.h>

static void activate( GtkApplication* app, gpointer user_data )
{
    GtkWidget *window;
    
    // Create the application window
    window = gtk_application_window_new(app);
    gtk_window_set_icon_from_file(GTK_WINDOW(window), "lib\\icon.png", NULL);
    gtk_window_set_title( GTK_WINDOW(window), "Heartbound Save Editor" );
    gtk_window_set_default_size( GTK_WINDOW(window), 720, 540 );
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    
    // Create a notebook with tabs for each of the game's chapter
    GtkWidget *notebook = gtk_notebook_new();
    GtkWidget *chapter_label[CHAPTER_AMOUNT];
    GtkWidget *chapter_page[CHAPTER_AMOUNT];

    // Create the chapters tabs and their respective pages
    for (int i = 0; i < CHAPTER_AMOUNT; i++)
    {
        chapter_label[i] = gtk_label_new(chapter[i]);
        chapter_page[i] = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook), chapter_page[i], chapter_label[i]);
    }

    // Add the notebook to the application window
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
    gtk_container_add(GTK_CONTAINER(window), notebook);
    
    // Render the application window and all its children
    gtk_widget_show_all(window);
}

int main ( int argc, char **argv )
{
    chdir("C:\\Users\\Tiago\\Desktop\\C\\Projetos\\Heartbound Save Editor\\release");
    open_save();
    close_save();

    GtkApplication *app;
    int status;

    app = gtk_application_new( "com.github.tbpaolini.hbsaveedit", G_APPLICATION_FLAGS_NONE );
    g_signal_connect( app, "activate", G_CALLBACK(activate), NULL );
    status = g_application_run( G_APPLICATION(app), argc, argv );
    g_object_unref(app);

    return status;
}