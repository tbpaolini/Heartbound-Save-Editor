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
    // Open the structure file
    FILE *save_structure = fopen(SAVE_STRUCT_LOC, "r");

    // Buffer for reading the lines of the file
    char *line_buffer = malloc(TEXT_BUFFER_SIZE * sizeof(char));

    // Get the table header and amount of columns
    fgets(line_buffer, sizeof(line_buffer), save_structure);    // Read the first line
    num_columns = 1;
    for (size_t i = 0; i < TEXT_BUFFER_SIZE; i++)
    {
        if (line_buffer[i] == '\t') {num_columns++;}
        else if (line_buffer[i] == '\n') {break;}
    }

    // Store the column names in an array
    save_headers = malloc(num_columns * sizeof(char*));
    char *name_buffer = malloc(TEXT_BUFFER_SIZE + (size_t)1);   // Buffer for the column name
    size_t pos = (size_t)0;         // Current position of character on the line
    size_t column = (size_t)0;      // Position of the current column
    size_t col_size = (size_t)0;    // Amount of characters on the current column

    while (line_buffer[pos] != '\n')    // Loop until the line break
    {
        if (column >= num_columns) break;       // Prevent overflow of the headers array
        if (pos >= TEXT_BUFFER_SIZE) break;     // Prevent overflow of the text buffers
        
        // Add the current character to the name buffer
        name_buffer[pos] = line_buffer[pos];
        col_size++;

        // Move to the next character and check if the column has ended
        if (line_buffer[++pos] == '\t')
        {
            name_buffer[pos] = '\0';                        // Terminate the string (null terminator)
            save_headers[column] = malloc( ++col_size );    // Allocate enough memory for the string (including the terminator)
            strcpy(save_headers[column], name_buffer);      // Copy the string from the buffer until the terminator (inclusive)
            column++;                                       // Move to the next column
        }
    }
    
    // Free the memory of the line buffer
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