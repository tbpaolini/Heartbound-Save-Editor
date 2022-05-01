/*
    Headers for the 'main.c' source file.
    This file defines the configurations of the main application window.
*/

#ifndef _CONFIG
#define _CONFIG

#include <stdbool.h>

// Window properties
#define WINDOW_WIDTH 720
#define WINDOW_HEIGHT 540
#define WINDOW_MIN_WIDTH 570
#define WINDOW_MIN_HEIGHT 320
#define WINDOW_BORDER 10
#define WINDOW_TITLE "Heartbound Save Editor"
#define WINDOW_ICON "../lib/icon.png"

// Page layout
#define PAGE_BORDER 10
#define MENUBAR_SPACING 2
#define GRID_ROW_SPACING 15
#define GRID_COLUMN_SPACING 5
#define IMAGE_WIDTH 80
#define IMAGE_HEIGHT 80
#define IMAGE_MARGIN 10
#define ENTRY_VERTICAL_SPACING 0
#define ENTRY_HORIZONTAL_SPACING 0

// Text fields properties
#define TEXT_FIELD_MARGIN 10
#define TEXT_FIELD_WIDTH 5
#define TEXT_FIELD_MAX_CHARS 14
#define TEXT_BUFFER_SIZE 1024

// Duration (in milliseconds) of the "file loaded" indicator
#define INDICATOR_TIMEOUT 2600

// Styles for the titles (for Light and Dark themes)

#define CSS_TITLE_LIGHT \
    "label#title-label {"\
    "font-size: 140%;"\
    "font-weight: bold;"\
    "color: #3a6b7c"\
    "}"

#define CSS_TITLE_DARK \
    "label#title-label {"\
    "font-size: 140%;"\
    "font-weight: bold;"\
    "color: #59a5bf"\
    "}"

// Default settings of GTK
#define DEFAULT_SETTINGS_INI \
    "[Settings]\n"\
    "gtk-theme-name=Windows10\n"\
    "gtk-icon-theme-name=Windows10\n"\
    "gtk-font-name=Segoe UI 9\n"\
    "gtk-application-prefer-dark-theme=0"\

// Default editor's options
#define CFG_AUTOMATIC_RELOADING "1"

// Macro function to display a native error dialog for fatal errors about missing or corrupted editor files
// Note: It tries to use the native GTK system of the Linux distro. It also prints a message to 'stderr', just in case.
#define NATIVE_ERROR(message, file, buffer_size) {\
    char *err_text = calloc(buffer_size, sizeof(char));\
    snprintf(err_text, buffer_size, message, file);\
    \
    fprintf(stderr, "Heartbound Save Editor - Error: %s", err_text);\
    GtkWidget *error_dialog = gtk_message_dialog_new(\
        NULL,\
        GTK_DIALOG_DESTROY_WITH_PARENT,\
        GTK_MESSAGE_ERROR,\
        GTK_BUTTONS_OK,\
        err_text\
    );\
    if (error_dialog != NULL)\
    {\
        gtk_window_set_title(GTK_WINDOW(error_dialog), "Heartbound Save Editor - error");\
        gtk_dialog_run(GTK_DIALOG(error_dialog));\
        gtk_widget_destroy(error_dialog);\
    }\
    \
    free(err_text);\
    \
    system("https://github.com/tbpaolini/Heartbound-Save-Editor/releases");\
    \
    exit(EXIT_FAILURE);}

// Whether we are using the loader to open the application
extern bool using_loader;

#endif