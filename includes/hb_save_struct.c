/* Template and data structure for the saved data of Heartbound */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SAVE_STRUCT_LOC "lib\\save_structure.tsv"   // Path to the file with the save structure
#define TEXT_BUFFER_SIZE (size_t)500                // Buffer size (in bytes) for each line of the structure file
#define NUM_STORY_VARS (size_t)1000                 // Amount of storyline variables in the save file
#define COLUMN_OFFSET (size_t)6                     // Amount of columns until the storyline variable values
#define ROW_OFFSET (size_t)7                        // Amount of rows before the first storyline variable

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
size_t num_data;                // Amount of data columns
size_t unit_column;             // Index of the column with the measurement unit's name

// Create the data structure for the save file
int open_save()
{
    // Open the save structure file
    FILE *save_structure = fopen(SAVE_STRUCT_LOC, "r");

    // Buffer for reading the lines of the file
    char *restrict line_buffer = malloc(TEXT_BUFFER_SIZE);

    // Get the table header and amount of columns
    fgets(line_buffer, TEXT_BUFFER_SIZE, save_structure);    // Read the first line
    num_columns = 1;
    for (size_t i = 0; i < TEXT_BUFFER_SIZE; i++)
    {
        if (line_buffer[i] == '\t') {num_columns++;}
        else if (line_buffer[i] == '\n') {break;}
    }
    num_data = num_columns - COLUMN_OFFSET;


    // Store the column names in an array
    save_headers = malloc(num_columns * sizeof(char*));                 // Allocate enough memory for one string pointer per column
    char *restrict value_buffer = malloc(TEXT_BUFFER_SIZE + (size_t)1); // Buffer for the column value
    
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
            if (line_buffer[line_pos] == '\t') line_pos++;  // Move to the next column
            value_pos = (size_t)0;                          // Return to the beginning of the value buffer

            // Check if this is the column of the measurement unit's name
            if ( strcmp(save_headers[column - 1], "X") == 0 ) {unit_column = column - 1;}
        }
    }

    // Skip the next 7 lines (player attributes)
    
    for (size_t i = 0; i < 7; i++)
    {
        fgets(line_buffer, TEXT_BUFFER_SIZE, save_structure);
    }

    // Read the structure of each storyline variable

    for (size_t var = 0; var < NUM_STORY_VARS; var++)
    {
        // Read the next line into the line buffer
        fgets(line_buffer, TEXT_BUFFER_SIZE, save_structure);

        line_pos = (size_t)0;    // Current position of character on the line
        value_pos = (size_t)0;   // Current position of character on the value of the current column
        column = (size_t)0;      // Position of the current column

        while (line_buffer[line_pos] != '\n')
        {
            // Skip empty columns
            while ( (line_buffer[line_pos] == '\t') && (line_pos < TEXT_BUFFER_SIZE) )
            {
                column++;
                line_pos++;
            }

            // Break if we arrived to the end of the line
            if (line_buffer[line_pos] == '\n') break;
            
            // Check for buffer overflow
            if (column >= num_columns) break;           // Headers array
            if (line_pos >= TEXT_BUFFER_SIZE) break;    // Line buffer
            if (value_pos >= TEXT_BUFFER_SIZE) break;   // Value buffer

            // Add the current character to the value buffer and move to the next character
            value_buffer[value_pos++] = line_buffer[line_pos++];

            // Check if the column has ended (tabulation was found)
            if ( (line_buffer[line_pos] == '\t') || (line_buffer[line_pos] == '\n') )
            {
                // Parse the value of the column
                value_buffer[value_pos] = '\0';
                char *my_value = malloc( ++value_pos );
                strcpy(my_value, value_buffer);
                
                // Store the corresponding column value on memory
                switch (column)
                {
                    case 0:
                        // Row
                        while (atoll(my_value) != (var + ROW_OFFSET))
                        {
                            // Skip unused rows
                            var++;
                        }
                        
                        free(my_value);
                        break;
                    
                    case 1:
                        // Storyline Var
                        if (atoll(my_value) != var)
                        {
                            // Consistency checking: Is the program on the variable  where it is expected to be?
                            // TO DO: Show some error dialog instead of just exiting
                            exit(EXIT_FAILURE);
                        }
                        
                        free(my_value);
                        break;
                    
                    case 2:
                        // Room/Object
                        save_data[var].location = my_value;
                        break;
                    
                    case 3:
                        // Description 1
                        save_data[var].name = my_value;
                        break;
                    
                    case 4:
                        // Description 2
                        save_data[var].info = my_value;
                        break;
                    
                    case 5:
                        // Internal State Count
                        if ( strcmp(my_value, "0|X") == 0 || strcmp(my_value, "X") == 0 )
                        {
                            // Storyline variable accepts any value
                            save_data[var].num_entries = (unsigned short)0;
                            save_data[var].aliases = NULL;
                        }
                        else
                        {
                            // Storyline variable accepts an specific amount of values
                            save_data[var].num_entries = (unsigned short)(strspn(my_value, "|")) + 1;
                            save_data[var].aliases = malloc( num_data * sizeof(char*));
                            
                            // Initialize all values of the aliases array to NULL
                            for (size_t i = 0; i < num_data; i++)
                            {
                                save_data[var].aliases[i] = NULL;
                            }
                        }
                        
                        free(my_value);
                        break;
                    
                    default:
                        // Name of the storyline variable value
                        if (column == unit_column)
                        {
                            // Set the measurement unit's name
                            save_data[var].unit = my_value;
                        }
                        else if (save_data[var].num_entries > 1)
                        {
                            // Set the alias for the value
                            save_data[var].aliases[column - COLUMN_OFFSET] = my_value;
                        }
                        else
                        {
                            // Discard the value
                            free(my_value);
                        }

                        break;
                }

                // Move to the next column
                column++;
                if (line_buffer[line_pos] == '\t') line_pos++;  // Skip the tabulation character
                value_pos = (size_t)0;                          // Return to the beginning of the value buffer
            }
        }

        // Break if the end of the file has been reached
        if (feof(save_structure)) break;
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

    // Values of the save structure
    for (size_t i = 0; i < NUM_STORY_VARS; i++)
    {
        free(save_data[i].location);
        free(save_data[i].name);
        free(save_data[i].info);
        free(save_data[i].unit);

        if (save_data[i].num_entries > 1)
        {
            for (size_t j = 0; j < num_data; j++)
            {
                free(save_data[i].aliases[j]);
            }

        free(save_data[i].aliases);
        }
    }
}