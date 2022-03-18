#include <stdio.h>
#include <gtk\gtk.h>
#include <unistd.h>
#include <hb_save.h>
// #include <windows.h>

static void activate( GtkApplication* app, gpointer user_data )
{
    GtkWidget *window;
    
    window = gtk_application_window_new(app);
    gtk_window_set_icon_from_file(GTK_WINDOW(window), "lib\\icon.png", NULL);
    gtk_window_set_title( GTK_WINDOW(window), "Heartbound Save Editor" );
    gtk_window_set_default_size( GTK_WINDOW(window), 720, 540 );
    gtk_widget_show(window);
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