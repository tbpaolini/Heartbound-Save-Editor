/* Template and data structure for the saved data of Heartbound */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "..\config.h"
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

// Storage of which crops of the Mossback's farm have been destroyed
// Note: Their states are stored as a bitmask across 8 different variables,
//       with 1 meaning destroyed, and 0 meaning not destroyed.
TurtlefarmCrop hb_turtlefarm_layout[TURTLEFARM_HEIGHT][TURTLEFARM_WIDTH];

double hb_turtlefarm_mask[32];  // The first 32 powers of 2, as double precision floats
// Note: Since I am storing all variable values as doubles, I can't just do bitwise operations
//       with the values of the crops, without changing how the rest of the variables work.
//       Since a value using a bitmask is the exception, rather than the rule, then I am
//       just going to add or subtract the powers if two to the bitmask values.

// Whether the structure file has been parsed
static bool save_is_initialized = false;

// Create the data structure for the save file
bool hb_create_save_struct()
{
    // Return if the save structure file has already been parsed
    if (save_is_initialized == true) return false;
    
    // Open the save structure file
    FILE *save_structure = fopen(SAVE_STRUCT_LOC, "r");

    // Display error and exit the program if the structure file is missing
    if (save_structure == NULL)
    {
        NATIVE_ERROR(
            "File %s could not be found.\nPlease download Heartbound Save Editor again.",
            SAVE_STRUCT_LOC,
            1024
        );
    }

    // Buffer for reading the lines of the file
    char *restrict line_buffer = malloc(SAVE_STRUCT_BUFFER);

    // Exit program if there's not enough memory
    if (line_buffer == NULL)
    {
        fprintf(stderr, "Not enough memory to run Heartbound Save Editor.\n");
        exit(EXIT_FAILURE);
    }

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
    
    if (value_buffer == NULL)
    {
        fprintf(stderr, "Not enough memory to run Heartbound Save Editor.\n");
        exit(EXIT_FAILURE);
    }

    // Exit program if there's not enough memory
    if (value_buffer == NULL)
    {
        fprintf(stderr, "Not enough memory to run Heartbound Save Editor.\n");
        exit(EXIT_FAILURE);
    }
    
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
            if (hb_save_headers[column] == NULL)
            {
                fprintf(stderr, "Not enough memory to run Heartbound Save Editor.\n");
                exit(EXIT_FAILURE);
            }
            strcpy_s(hb_save_headers[column++], value_pos, value_buffer);   // Copy the string from the buffer until the terminator (inclusive)
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
        hb_save_data[i].is_bitmask = false;
        hb_save_data[i].used = false;
        hb_save_data[i].def = 0.0;
        hb_save_data[i].maximum = 0.0;
    }

    // Read the structure of each storyline variable

    for (size_t var = 0; var < NUM_STORY_VARS; var++)
    {
        // Read the next line into the line buffer
        char* read_status = fgets(line_buffer, SAVE_STRUCT_BUFFER, save_structure);
        if (read_status == NULL) break;     // Break if there were nothing else to be read (end of file)

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
                if (my_value == NULL)
                {
                    fprintf(stderr, "Not enough memory to run Heartbound Save Editor.\n");
                    exit(EXIT_FAILURE);
                }
                strcpy_s(my_value, value_pos, value_buffer);
                
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
                            // Show an error and exit if the file failed the consistency check
                            NATIVE_ERROR(
                                "File %s is corrupted.\nPlease download Heartbound Save Editor again.",
                                SAVE_STRUCT_LOC,
                                1024
                            );
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

                        // Check if this entry has data of the Mossback's farm crops
                        if (strcmp(my_value, "Crop Values") == 0)
                        {
                            hb_save_data[var].is_bitmask = true;
                        }
                        /* Note:
                            The crops at the Mossback's farm are unique in the sense that their state is
                            stored in bitmasks (0 = not destroyed; 1 =  destroyed), across 8 variables.
                            So we are have a special case for them.
                        */
                        
                        break;
                    
                    case 4:
                        // Description 2 (more detail about the storyline variable)
                        hb_save_data[var].info = my_value;

                        // If the description specifies the maximum, set it to the '.maximum attribute'
                        char *max = strstr(my_value, "Max ");
                        if (max != NULL) hb_save_data[var].maximum = atof(max+3);

                        // If the description specifies the default, set it to the '.def' attribute
                        char *def = strstr(my_value, "Default ");
                        if (def != NULL) hb_save_data[var].def = atof(def+7);

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
                            extra value to the 'hb_save_data[var].aliases[var_pos]' array beyond the allocated
                            memory of '.aliases'. This would happen because the program checks for a tabulation
                            or line break in order to endparsing a cell.
                            
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

        // Keep track of the maximum index of the storyline variables
        // (for the purpose of garbage collection)
        max_var = var;

        // Break if the end of the file has been reached
        if (feof(save_structure)) break;
    }

    // Close the save structure file
    fclose(save_structure);
    
    // Free the memory of the text buffers
    free(value_buffer);
    free(line_buffer);

    // Parse the layout of the Mossback's farm
    turtlefarm_init();
    
    // Flag that the structure file has been parsed
    save_is_initialized = true;
    return true;
}

// Load into memory the layout of the Mossbacks farm
static void turtlefarm_init()
{
    // Try opening the file
    FILE *turtlefarm_file = fopen(TURTLEFARM_STRUCT_LOC, "r");

    // Exit with an error if the file could not be found
    if (turtlefarm_file == NULL)
    {
        NATIVE_ERROR(
            "File %s could not be found.\nPlease download Heartbound Save Editor again.",
            TURTLEFARM_STRUCT_LOC,
            1024
        );
    }

    // Allocate buffers for reading the lines
    char *restrict line_buffer = malloc(TURTLEFARM_STRUCT_BUFFER * sizeof(char));
    if (line_buffer == NULL)
    {
        fprintf(stderr, "Not enough memory to run Heartbound Save Editor.\n");
        exit(EXIT_FAILURE);
    }
    
    char *restrict value_buffer = malloc(TURTLEFARM_STRUCT_BUFFER * sizeof(char));
    if (value_buffer == NULL)
    {
        fprintf(stderr, "Not enough memory to run Heartbound Save Editor.\n");
        exit(EXIT_FAILURE);
    }

    // Coordinates of the crop on the farm layout grid
    size_t x_pos = 0;
    size_t y_pos = 0;

    // Loop through the lines of the file
    while (!feof(turtlefarm_file) && y_pos < TURTLEFARM_HEIGHT)
    {
        // Read a row from the file
        char *line_status = fgets(line_buffer, TURTLEFARM_STRUCT_BUFFER, turtlefarm_file);
        if (line_status == NULL) break;

        // Loop through the columns in the row
        char next_char = '\0';
        char line_pos = 0;
        for (x_pos = 0; x_pos < TURTLEFARM_WIDTH; x_pos++)
        {
            // Keep track if the file is well formatted
            bool success = false;
            
            // Get the text until the next delimiter (tab or colons)
            next_char = line_buffer[line_pos++];
            if (next_char == '\0') break;       // Break if there is no text left
            if (next_char == '\n') continue;    // Move to the next row if at newline
            
            if (next_char == '\t')
            {
                // If the cell is empty, store 0 for both values
                hb_turtlefarm_layout[y_pos][x_pos].var = 0;
                hb_turtlefarm_layout[y_pos][x_pos].bit = 0;
                
                // Move to the next cell
                success = true;
                continue;
            }
            else if (isdigit(next_char))
            {
                // Store the values if the cell is not empty
                
                // Get the storyline variable's number
                char value_pos = 0;
                while (isdigit(next_char))
                {
                    value_buffer[value_pos++] = next_char;
                    next_char = line_buffer[line_pos++];
                }
                
                // Store the storyline variable number
                value_buffer[value_pos] = '\0';  // Null terminate the string
                hb_turtlefarm_layout[y_pos][x_pos].var = atoll(value_buffer);

                if (next_char == ':')
                {
                    // Get the bit position
                    next_char = line_buffer[line_pos++];
                    value_pos = 0;
                    while (isdigit(next_char))
                    {
                        value_buffer[value_pos++] = next_char;
                        next_char = line_buffer[line_pos++];
                    }

                    // Store the bit position
                    value_buffer[value_pos] = '\0';  // Null terminate the string
                    hb_turtlefarm_layout[y_pos][x_pos].bit = atoll(value_buffer);
                    if (value_pos > 0 && hb_turtlefarm_layout[y_pos][x_pos].bit < 32
                        && hb_turtlefarm_layout[y_pos][x_pos].var < NUM_STORY_VARS)
                    {
                        /*
                            The parsing is considered a success if there is at least one digit on both
                            the variable number and bit position. Also they need to be within  their
                            respective ranges, in order to avoid reading or writting beyond their limits.
                        */
                        success = true;
                    }

                    // Store the grid coordinates of the crop
                    hb_turtlefarm_layout[y_pos][x_pos].x = x_pos;
                    hb_turtlefarm_layout[y_pos][x_pos].y = y_pos;
                }
            }

            // Exit with an error if the file is not properly formatted
            if (!success)
            {
                NATIVE_ERROR(
                    "File %s is corrupted.\nPlease download Heartbound Save Editor again.",
                    TURTLEFARM_STRUCT_LOC,
                    1024
                );
            }
        }
        
        // Move to the next row
        y_pos++;
    }

    // Close the file
    fclose(turtlefarm_file);

    // Deallocate the buffers
    free(line_buffer);
    free(value_buffer);

    // Calculate the values of the bitmask
    for (size_t i = 0; i < 32; i++)
    {
        hb_turtlefarm_mask[i] = (double)(1 << i);
    }

    // Determine the maximum value for each of the crop's variables.
    // Since the maximum values are initialized to zero, we just add the value of
    // each crop's bitmask (for all crops) to the maximum of its corresponding variable.
    for (size_t y_pos = 0; y_pos < TURTLEFARM_HEIGHT; y_pos++)
    {
        for (size_t x_pos = 0; x_pos < TURTLEFARM_WIDTH; x_pos++)
        {
            TurtlefarmCrop *crop = &hb_turtlefarm_layout[y_pos][x_pos];         // Pointer to the crop's struct
            if (crop->var == 0) continue;
            hb_save_data[crop->var].maximum += hb_turtlefarm_mask[crop->bit];   // Add the crop's bitmask value to the maximum 
        }
    }
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
        hb_save_data[var].def = 0.0;
        hb_save_data[var].maximum = 0.0;
        hb_save_data[var].num_entries = (size_t)0;
        hb_save_data[var].index = (size_t)0;
        hb_save_data[var].is_bitmask = false;
        hb_save_data[var].used = false;
    }

    // Flag that the save structure is not parsed
    save_is_initialized = false;
}