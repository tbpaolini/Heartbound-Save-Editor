/*
    Headers for the 'main.c' source file.
    This file defines the configurations of the main application window.
*/

#ifndef _CONFIG
#define _CONFIG

// Window properties
#define WINDOW_WIDTH 720
#define WINDOW_HEIGHT 540
#define WINDOW_MIN_WIDTH 570
#define WINDOW_MIN_HEIGHT 320
#define WINDOW_BORDER 10
#define WINDOW_TITLE "Heartbound Save Editor"
#define WINDOW_ICON "..\\lib\\icon.png"

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

// Macro function to display a native error dialog for fatal errors about missing or corrupted editor files
// Note: Requires the 'windows.h' header
#include <windows.h>
#define NATIVE_ERROR(message, file, buffer_size) \
    char *text = calloc(buffer_size, sizeof(char));\
    snprintf(text, buffer_size, message, file);\
    \
    int button = MessageBoxA(\
        NULL,\
        text,\
        "Heartbound Save Editor - error",\
        MB_ICONERROR | MB_OK\
    );\
    \
    free(text);\
    \
    if (button = IDOK)\
    {\
        ShellExecute(NULL, "open", "https://github.com/tbpaolini/Heartbound-Save-Editor/releases", NULL, NULL, SW_SHOWNORMAL);\
    }\
    \
    exit(EXIT_FAILURE)

#endif