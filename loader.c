/*
    Loader for the main executable of the Heartbound Save Editor.
    
    Strictly speaking, the loader isn't really necessary. People can just
    open the program directly.
    
    However I am doing a loader because some people might not notice that the
    editor's executable is inside the "bin\" folder. So I am placing the loader
    on the root of the installation folder.

    Note: I cannot change the location of the main executable because GTK expects
    a certain folder structure where the program is installed.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>    // Interface with Microsoft Windows processes
#include <windows.h>    // API of Microsoft Windows

#define BUFFER_SIZE (size_t)1024
static char *HB_SAVE_EDITOR_FOLDER = "bin";
static char *HB_SAVE_EDITOR_NAME = "Heartbound Save Editor.exe";
static char HB_SAVE_EDITOR_PATH[BUFFER_SIZE];
static char HB_SAVE_EDITOR_PATH_QUOTED[BUFFER_SIZE];

intptr_t main ( int argc, char **argv )
{
    // Find the directory of the loader

    char *loader_path = calloc(BUFFER_SIZE, sizeof(char));
    GetModuleFileNameA(NULL, loader_path, BUFFER_SIZE);

    int path_len = strnlen_s(loader_path, BUFFER_SIZE);  // Length of the loader's path
    int path_pos = 0;
    for (int i = path_len - 1; i >= 0; i--)
    {
        // Move backwards from the end of the path until a backslash or slash character is found
        if ( (loader_path[i] == '/') || (loader_path[i] == '/') )
        {
            path_pos = i;   // The position on the string where the loader's directory ends
            break;
        }
    }

    // Store the loader directory
    char *path_dir = calloc(path_pos + 1, sizeof(char));
    memcpy_s(path_dir, path_pos, loader_path, path_pos);
    free(loader_path);

    // Absolute path of the Heartbound Save Editor

    snprintf(
        HB_SAVE_EDITOR_PATH,
        BUFFER_SIZE,
        "%s/%s/%s",
        path_dir,
        HB_SAVE_EDITOR_FOLDER,
        HB_SAVE_EDITOR_NAME
    );

    snprintf(
        HB_SAVE_EDITOR_PATH_QUOTED,
        BUFFER_SIZE,
        "\"%s/%s/%s\"",   // The path is enclosed with quotes in order to handle the blank spaces when used as an argument
        path_dir,
        HB_SAVE_EDITOR_FOLDER,
        HB_SAVE_EDITOR_NAME
    );

    // Build the arguments list to pass to the editor

    // Allocate enough memory for an array of strings that has one more element
    // than the 'argv[]' of this loader. The additional element will be a NULL.
    char **hb_args = malloc( (argc + 2) * sizeof(char*) );
    if (hb_args == NULL) exit(EXIT_FAILURE);

    for (size_t i = 0; i < argc; i++)
    {
        // Each entry can have up to BUFFER_SIZE characters
        hb_args[i] = calloc( BUFFER_SIZE, sizeof(char) );
        if (hb_args[i] == NULL) exit(EXIT_FAILURE);
    }
    
    // Set the Save Editor's path as the first argument
    strncpy_s(hb_args[0], BUFFER_SIZE, HB_SAVE_EDITOR_PATH_QUOTED, BUFFER_SIZE);

    // Set the next arguments to the arguments passed to the loader
    for (size_t i = 1; i < argc; i++)
    {
        snprintf(hb_args[i], BUFFER_SIZE, "\"%s\"", argv[i], BUFFER_SIZE);
    }

    hb_args[argc] = "--loader";
    
    // Set the last argument to NULL
    // (Windows needs this to know that we are done with passing arguments)
    hb_args[argc+1] = NULL;

    // *************************************************
    // Execute the Save Editor with the passed arguments
    // and give it control of the process
    // *************************************************
    intptr_t status = _execv(HB_SAVE_EDITOR_PATH, (const char *const *)hb_args);

    // *******************************
    // Failure to open the Save Editor
    // *******************************

    /* Note:
        If the '_execv()' function has success in opening the program, the control of the process is given to that program.
        That is, the Save Editor will overlay this process. The loader's memory will be replaced with the editor's memory.
        But if opening the program fails, then the following code is executed.
    */
    
    // Shows an error dialog
    int button = MessageBox(
        NULL,
        "Failed to open Heartbound Save Editor.\n\n"
        "Tip: This file should be at the Editor's folder in order to be executed. "
        "If you want to run the program from somewhere else, then you should create "
        "a shortcut to it (Right click > Send to > Desktop), instead of copying this file.\n\n"
        "But if the problem is that Save Editor is corrupted, then please download it again.",
        "Error",
        MB_ICONERROR | MB_OK
    );

    // Open the download page when the dialog is closed
    if (button = IDOK)
    {
        ShellExecute(NULL, "open", "https://github.com/tbpaolini/Heartbound-Save-Editor/releases", NULL, NULL, SW_SHOWNORMAL);
    }
    
    // Free the allocated memory
    
    free(path_dir);

    for (size_t i = 0; i < argc; i++)
    {
        free(hb_args[i]);
    }

    free(hb_args);
    
    return status;
}

#undef BUFFER_SIZE