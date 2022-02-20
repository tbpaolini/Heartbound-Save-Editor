/* Template and data structure for the saved data of Heartbound */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SAVE_STRUCT_LOC "lib\\save_structure.tsv"   // Path to the file with the save structure
#define TEXT_BUFFER_SIZE (size_t)500                // Buffer size (in bytes) for each line of the structure file
#define NUM_STORY_VARS (size_t)1000                 // Amount of storyline variables in the save file

// Player attributes
char *game_seed[10];            // Game seed (10 decimal characters long)
char *room_id[30];              // The ID (as a string) of the room the player is
double x_axis, y_axis;          // Coordinates of the player in the room
double hp_current, hp_maximum;  // Current and maximum hit points of the player

// Which glyph sets the player know:
// 0 = None; 1 = Lightbringer; 2 = Lightbringer and Darksider
char known_glyphs;

// The storyline variables
struct StorylineVars
{
    unsigned int value;          // Value of this entry on the save file
    char *location;              // In-game location that this entry applies to
    char *name;                  // Description of the in-game feature this entry refers to
    char *info;                  // Further description of the feature
    char *unit;                  // Measurement unit of the value this entry represents
    unsigned short num_entries;  // Amount of different values that the field accept (0 if it accepts any value)
    char **aliases;              // Associate each numeric value to its meaning (as strings)
} save_data[NUM_STORY_VARS];

// Headers of the save structure
char **save_headers;            // Names of the columns (array of strings)
size_t num_columns;             // Amount of columns in the structure

// Create the data structure for the save file
int open_save()
{
    // Open the save structure file
    FILE *save_structure = fopen(SAVE_STRUCT_LOC, "r");

    // Buffer for reading the lines of the file
    char *line_buffer = malloc(TEXT_BUFFER_SIZE);

    // Get the table header and amount of columns
    fgets(line_buffer, TEXT_BUFFER_SIZE, save_structure);    // Read the first line
    num_columns = 1;
    for (size_t i = 0; i < TEXT_BUFFER_SIZE; i++)
    {
        if (line_buffer[i] == '\t') {num_columns++;}
        else if (line_buffer[i] == '\n') {break;}
    }

    // Store the column names in an array
    save_headers = malloc(num_columns * sizeof(char*));         // Allocate enough memory for one string pointer per column
    char *value_buffer = malloc(TEXT_BUFFER_SIZE + (size_t)1);  // Buffer for the column value
    
    size_t line_pos = (size_t)0;    // Current position of character on the line
    size_t value_pos = (size_t)0;   // Current position of character on the name of the current column
    size_t column = (size_t)0;      // Position of the current column

    while (line_buffer[line_pos] != '\n')    // Loop until the line break
    {
        // Check for buffer overflow
        if (column >= num_columns) break;           // Headers array
        if (line_pos >= TEXT_BUFFER_SIZE) break;    // Line buffer
        if (value_pos >= TEXT_BUFFER_SIZE) break;   // Value buffer
        
        // Add the current character to the name buffer and move to the next character
        value_buffer[value_pos++] = line_buffer[line_pos++];

        // Check if the column has ended (tabulation was found)
        if ( (line_buffer[line_pos] == '\t') || (line_buffer[line_pos] == '\n') )
        {
            value_buffer[value_pos] = '\0';                 // Terminate the string (null terminator)
            save_headers[column] = malloc( ++value_pos );   // Allocate enough memory for the string (including the terminator)
            strcpy(save_headers[column++], value_buffer);   // Copy the string from the buffer until the terminator (inclusive)
            line_pos++;                                     // Move to the next column
            value_pos = (size_t)0;                          // Return to the beginning of the value buffer
        }
    }

    // Close the save structure file
    fclose(save_structure);
    
    // Free the memory of the text buffers
    free(value_buffer);
    free(line_buffer);
    
    return 0;
    // TO DO: Error handling
}

// Free used memory for the save file
void close_save()
{
    // Names of the columns of the save structure
    for (unsigned short i = 0; i < num_columns; i++)
    {
        free(save_headers[i]);
    }
    free(save_headers);
}