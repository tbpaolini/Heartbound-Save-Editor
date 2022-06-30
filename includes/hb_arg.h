/*
    Support for features that help with the Heartbound's ARG (Alternate Reality Game)
*/

#ifndef _HB_ARG
#define _HB_ARG

#include <stdbool.h>
#include <gtk/gtk.h>

// This button set the values on the save file necessary during the "Deep Thought" puzzle of the ARG:
// https://gopiratesoftware.com/games/Heartbound/CORE_DUMP/MEMORY_ALPHA/CRASH_LOG/BOOT_RECORD/DEEP_THOUGHT/
extern GtkWidget *hb_arg_remember_button;

// Check if the user has completed the first step of the "Deep Thought" puzzle of the ARG
bool hb_arg_check_deep_thought();

// Create the "Remember painful memory" button
void hb_arg_remember_button_init();

// Change the values of storyline variables 400 to 407, as required to proceed on the "Deep Thought" puzzle of the ARG
void hb_remember_painful_memory(GtkWidget *widget);

#endif