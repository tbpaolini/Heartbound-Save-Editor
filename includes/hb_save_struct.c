/* Template and data structure for the saved data of Heartbound */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <hb_save_struct.h>

// Lookup table of storyline variables
StorylineVars hb_save_data[NUM_STORY_VARS];

// Headers of the save structure
char **hb_save_headers;                // Names of the columns (array of strings)
static size_t num_columns;             // Amount of columns in the structure
static size_t num_data;                // Amount of data columns
static size_t unit_column;             // Index of the column with the measurement unit's name
static size_t var_pos;                 // Index of the value of a storyline variable
static size_t max_var;                 // Index of the last storyline variable on the structure file

// Whether the structure file has been parsed
static bool save_is_initialized = false;

// Create the data structure for the save file
int hb_create_save_struct()
{
    // Return if the save structure file has already been parsed
    if (save_is_initialized == true) return 0;
    
    // Open the save structure file
    FILE *save_structure = fopen(SAVE_STRUCT_LOC, "r");

    // Buffer for reading the lines of the file
    char *restrict line_buffer = malloc(SAVE_STRUCT_BUFFER);

    // Get the table header and amount of columns
    fgets(line_buffer, SAVE_STRUCT_BUFFER, save_structure);    // Read the first line
    num_columns = 1;
    for (size_t i = 0; i < SAVE_STRUCT_BUFFER; i++)
    {
        if (line_buffer[i] == '\t') {num_columns++;}
        else if (line_buffer[i] == '\n') {break;}
    }
    num_data = num_columns - COLUMN_OFFSET;


    // Store the column names in an array
    hb_save_headers = malloc(num_columns * sizeof(char*));                 // Allocate enough memory for one string pointer per column
    char *restrict value_buffer = malloc(SAVE_STRUCT_BUFFER + (size_t)1); // Buffer for the column value
    
    size_t line_pos = (size_t)0;    // Current position of character on the line
    size_t value_pos = (size_t)0;   // Current position of character on the name of the current column
    size_t column = (size_t)0;      // Position of the current column

    while (line_buffer[line_pos] != '\n')    // Loop until the line break
    {
        // Check for buffer overflow
        if (column >= num_columns) break;           // Headers array
        if (line_pos >= SAVE_STRUCT_BUFFER) break;    // Line buffer
        if (value_pos >= SAVE_STRUCT_BUFFER) break;   // Value buffer
        
        // Add the current character to the name buffer and move to the next character
        value_buffer[value_pos++] = line_buffer[line_pos++];

        // Check if the column has ended (tabulation was found)
        if ( (line_buffer[line_pos] == '\t') || (line_buffer[line_pos] == '\n') )
        {
            value_buffer[value_pos] = '\0';                 // Terminate the string (null terminator)
            hb_save_headers[column] = malloc( ++value_pos );   // Allocate enough memory for the string (including the terminator)
            strcpy(hb_save_headers[column++], value_buffer);   // Copy the string from the buffer until the terminator (inclusive)
            if (line_buffer[line_pos] == '\t') line_pos++;  // Move to the next column
            value_pos = (size_t)0;                          // Return to the beginning of the value buffer

            // Check if this is the column of the measurement unit's name
            if ( strcmp(hb_save_headers[column - 1], "X") == 0 ) {unit_column = column - 1;}
        }
    }

    // Skip the next 7 lines (player attributes)
    
    for (size_t i = 0; i < 7; i++)
    {
        fgets(line_buffer, SAVE_STRUCT_BUFFER, save_structure);
    }

    // Initialize the 'used' flags of the storyline variables to 'false', and the pointers to NULL
    
    for (size_t i = 0; i < NUM_STORY_VARS; i++)
    {
        hb_save_data[i].location = NULL;
        hb_save_data[i].name = NULL;
        hb_save_data[i].info = NULL;
        hb_save_data[i].unit = NULL;
        hb_save_data[i].aliases = NULL;
        hb_save_data[i].used = false;
        hb_save_data[i].maximum = 0.0;
    }

    // Read the structure of each storyline variable

    for (size_t var = 0; var < NUM_STORY_VARS; var++)
    {
        // Read the next line into the line buffer
        fgets(line_buffer, SAVE_STRUCT_BUFFER, save_structure);

        line_pos = (size_t)0;    // Current position of character on the line
        value_pos = (size_t)0;   // Current position of character on the value of the current column
        column = (size_t)0;      // Position of the current column

        while (line_buffer[line_pos] != '\n')
        {
            // Skip empty columns
            while ( (line_buffer[line_pos] == '\t') && (line_pos < SAVE_STRUCT_BUFFER) )
            {
                column++;
                line_pos++;
                
                // Keep track of the current value column of the storyline variable
                if (column == COLUMN_OFFSET) var_pos = 0;
            }

            // Break if we arrived to the end of the line
            if (line_buffer[line_pos] == '\n') break;
            
            // Check for buffer overflow
            if (column >= num_columns) break;           // Headers array
            if (line_pos >= SAVE_STRUCT_BUFFER) break;    // Line buffer
            if (value_pos >= SAVE_STRUCT_BUFFER) break;   // Value buffer

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
                        
                        hb_save_data[var].used = true;  // Flag the variable as 'used'
                        hb_save_data[var].index = var; // Store the number of the variable (for debugging purposes)
                        free(my_value);
                        break;
                    
                    case 2:
                        // Room/Object (where or for what the storyline variable applies to)
                        hb_save_data[var].location = my_value;
                        break;
                    
                    case 3:
                        // Description 1 (name of the storyline variable)
                        hb_save_data[var].name = my_value;
                        break;
                    
                    case 4:
                        // Description 2 (more detail about the storyline variable)
                        hb_save_data[var].info = my_value;

                        // If the description specifies the maximum, set it to the '.maximum attribute'
                        char *max = strstr(my_value, "Max ");
                        if (max != NULL) hb_save_data[var].maximum = atof(max+3);

                        break;
                    
                    case 5:
                        // Internal State Count
                        /*
                            This column stores all the possible states for the storyline variable.
                            If the variable accepts some arbitrary number instead, then the value of this
                            column is either '0|X' or 'X'. In that case, there will be no aliases for the
                            values, just the measurement unit that the value represents (like seconds, tries..)
                            Also if a maximum value is given, then the variable is also considered to accept
                            any arbitrary value.
                        */
                        if ( strcmp(my_value, "0|X") == 0 || strcmp(my_value, "X") == 0 || hb_save_data[var].maximum > 0.0)
                        {
                            // Storyline variable accepts any value
                            hb_save_data[var].num_entries = (size_t)0;
                            hb_save_data[var].aliases = NULL;
                        }
                        else
                        {
                            // Storyline variable accepts an specific amount of values

                            // Count how many values
                            hb_save_data[var].num_entries = (size_t)1;
                            size_t v_pos = 0;
                            while (my_value[v_pos] != '\0')
                            {
                                if (my_value[v_pos++] == '|') hb_save_data[var].num_entries += 1;
                            }

                            hb_save_data[var].aliases = malloc( hb_save_data[var].num_entries * sizeof(ValueAlias));
                            
                            // Initialize all values of the aliases array to NULL
                            for (size_t i = 0; i < hb_save_data[var].num_entries; i++)
                            {
                                hb_save_data[var].aliases[i] = (ValueAlias){NULL, NULL};
                            }
                        }
                        
                        free(my_value);
                        break;
                    
                    default:
                        // Storyline variable value
                        /*
                            The switch statement will end up on this clause, once it gets to the columns that
                            store the possible values for the current variable.

                            Among these columns, is the 'X' column that stores the measurement unit for the
                            variables that accept any value (like elapsed time, number of tries, etc.)
                            We will check if we are on the unit's column to add the unit's name to the
                            appropriate place.
                            
                            The index where the values columns begin is on the 'COLUMN_OFFSET' macro. Each
                            time the column counter is increased, the program checks if we are at that index.
                            If it is, then the 'value_pos' variable is set to zero. That variable is used to
                            keep track of the current index on the array of value's aliases.

                            If we are not on the unit column, then the cell counts as a regular value for
                            the storyline variable.
                        */

                        // Break if the string is empty
                        /*  
                            In case the file does not end in a line break, the last column of the last line
                            will return an empty string, which would lead the program to attempt writting an
                            extra value to the 'hb_save_data' array beyond its allocated memory. This would
                            happen because the program checks for a tabulation or line break in order to end
                            parsing a cell.
                            
                            Adding this check for an empty string here seems simpler than reworking the
                            entire file parsing system just for this case.
                        */
                        if (my_value[0] == '\0')
                        {
                            free(my_value);
                            break;
                        }
                        
                        // We are on the unit's column
                        if (column == unit_column)
                        {
                            // Set the measurement unit's name
                            hb_save_data[var].unit = my_value;
                        }

                        // There are multiple values
                        else if (hb_save_data[var].num_entries > 0)
                        {
                            // Set the alias for the current value and move to the next value
                            // (The value's description is linked to the name of its respective column)
                            hb_save_data[var].aliases[var_pos++] = (ValueAlias){
                                 .header = &(hb_save_headers[column]), // Pointer to the name of the header (which is the value itself)
                                 .description = my_value            // What the value does (as a string)
                            };
                        }
                        
                        // The variable has no specified amount of values (can accept any value)
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

                // Keep track of the current value column of the storyline variable
                if (column == COLUMN_OFFSET) var_pos = 0;
            }
        }

        // Break if the end of the file has been reached
        if (feof(save_structure))
        {
            max_var = var;
            break;
        }
    }

    // Close the save structure file
    fclose(save_structure);
    
    // Free the memory of the text buffers
    free(value_buffer);
    free(line_buffer);
    
    // Flag that the structure file has been parsed
    save_is_initialized = true;
    return true;

    // TO DO: Error handling
}

// Free the used memory by the save data structure
void hb_destroy_save_struct()
{
    // Names of the columns of the save structure
    for (size_t i = 0; i < num_columns; i++)
    {
        free(hb_save_headers[i]);
    }
    free(hb_save_headers);

    // Deallocate the values of the save structure for each storyline variable
    for (size_t var = 0; var <= max_var; var++)
    {
        
        // Deallocate the memory from the attributes
        free(hb_save_data[var].location);
        free(hb_save_data[var].name);
        free(hb_save_data[var].info);
        free(hb_save_data[var].unit);

        // Deallocate the aliases array
        if (hb_save_data[var].num_entries > 0)
        {
            for (size_t j = 0; j < hb_save_data[var].num_entries; j++)
            {
                // Deallocate the description string of each value
                free(hb_save_data[var].aliases[j].description);
            }

        // Deallocate the array of ValueAlias structs
        free(hb_save_data[var].aliases);
        }
        
        // Set the statically allocated values to zero
        hb_save_data[var].value = 0.0;
        hb_save_data[var].maximum = 0.0;
        hb_save_data[var].num_entries = (size_t)0;
        hb_save_data[var].index = (size_t)0;
    }

    // Flag that the save structure is not parsed
    save_is_initialized = false;
}