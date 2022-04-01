import re

"""
This Python script makes some a few changes on the text of the save structure
in order to make it more user friendly:

- splitting the CascalCase from the save file structure, by putting a space
  before a capital letter in the middle of a word.
- Adding a space before numbers inside words
- Replacing the 'Rn_' abbreviation by 'Round n '.
- Changing the header 'X (Count)' to just 'X', to facilitate the matching
  with the 'Internal State Count' column
- Adding the value of the variable of the Heartbound ARG (Alternate Reality Game).
- Formatting a couple of lines that do not follow the patterns of the others
"""

# Regular expression to match uppercased words or numbers that inside another word
# (?<=[a-z])        Look if the previous character is a lowercase letter
# [A-Z]+[a-z]+      One or more uppercase letters followed by one or more lowercase letters
# [0-9]+            Decimal digits
camel_case = re.compile(r"(?<=[a-z])([A-Z]+[a-z]+|[0-9]+)")

# Regular expression to match the 'Rn_' pattert (where n = 1,2,3)
round_n = re.compile(r"R([1-3])_")

# Regular expression for finding the 5th and 7th columns
# (it will be used for swapping those columns)
# [a-zA-z0-9 \|]  <-- Any alphanumeric character, a pipe, or a space
swap_5_7 = re.compile(r"((?:[a-zA-z0-9 \|]*\t){4})([a-zA-z0-9 \|]*)((?:[a-zA-z0-9 \|]*\t){2})([a-zA-z0-9 \|]*)")

# Loop through the file
edited_lines = []
with open("..\structure\Save File Structure - 1.0.9.55.tsv", "rt") as file:
    for line in file:

        # Change the header 'X (Count)' to just 'X'
        # And add a column for the ARG's variable value
        if line.startswith("Row\t"):
            line = line.replace("X (Count)", "X")
            line = line[:-1] + "\t42\n"
        else:
            line = line[:-1] + "\t\n"

        # Add the value to the ARG variable
        if line.startswith("245\t"):
            line = "245\t238\tThe Truth\t\t\t0|42\tNo\t\t\t\t\t\t\t\t\t\t\t\tYes\n"

        # Replace 'Rn_' by 'Round n '
        # (working only on lines 350 or 354 because it only happens there)
        if line.startswith("350\t") or line.startswith("354\t"):
            line = round_n.sub(r"Round \1 ", line)
        
        # Format the lines that do not follow the pattern of the others
        for row in ("19", "25", "41", "42", "347", "353"):
            if line.startswith(row + "\t"):
                # Swap the 5th and 7th row
                line = swap_5_7.sub(r"\1\4\3\2", line)
            
            # Delete redundant descriptions
            if (row == "347"): line = line.replace("Baskets Filled\tBaskets Filled", "Baskets Filled\t", 1)
            if (row == "353"): line = line.replace("Cartons Filled\tCartons Filled", "Cartons Filled\t", 1)
        
        if line.startswith("386\t"):
            # That is such an oddball line that it deserves its very own special case :-)
            line = "386\t379\tDeer Judge\tFlowers Stepped On\t\t0|X\t\t\t\t\t\t\t\t\t\t\tNumberOfFlowers\t\t\n"

        # Split the cells before the capital letters in the middle,
        # then join the parts by a space
        new_line = " ".join(camel_case.split(line))

        # NOTE: Some extra spaces are added by the above edits,
        #       and will be removed next.

        # Replace double blank spaces by a single space
        new_line = new_line.replace("  ", " ")

        # Replace blank spaces at the end of cells
        new_line = new_line.replace(" \t", "\t")
        new_line = new_line.replace(" \n", "\n")
        
        # Join each cell with a tab, then add the changed line to the list
        edited_lines.append(new_line)

# Save a file with all the changed lines
with open("..\structure\save_structure.tsv", "wt") as file:
    file.writelines(edited_lines)