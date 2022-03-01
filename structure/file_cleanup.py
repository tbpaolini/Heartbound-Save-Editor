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
"""

# Regular expression to match uppercased words or numbers that inside another word
# (?<=[a-z])        Look if the previous character is a lowercase letter
# [A-Z]+[a-z]+      One or more uppercase letters followed by one or more lowercase letters
# [0-9]+            Decimal digits
camel_case = re.compile(r"(?<=[a-z])([A-Z]+[a-z]+|[0-9]+)")

# Regular expression to match the 'Rn_' pattert (where n = 1,2,3)
round_n = re.compile(r"R([1-3])_")

# Loop through the file
edited_lines = []
with open("Save File Structure - 1.0.9.55.tsv", "rt") as file:
    for line in file:

        # Change the header 'X (Count)' to just 'X'
        if line.startswith("Row"):
            line = line.replace("X (Count)", "X")

        # Replace 'Rn_' by 'Round n '
        # (working only on lines 350 or 354 because it only happens there)
        if line.startswith("350") or line.startswith("354"):
            line = round_n.sub(r"Round \1 ", line)

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
with open("save_structure.tsv", "wt") as file:
    file.writelines(edited_lines)