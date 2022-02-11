import re

"""
This Python script splits the CascalCase from the save file structure,
by putting a space before a capital letter in the middle of a word.
"""

# Regular expression to match uppercased words that inside another word
# (?<=\B)           Look if the previous character is not a word boundary
# ([A-Z]+[a-z]+)    One or more uppercase letters followed by one or more lowercase letters
camel_case = re.compile(r"(?<=\B)([A-Z]+[a-z]+)")

# Loop through the file
edited_lines = []
with open("Save File Structure - 1.0.9.55.tsv", "rt") as file:
    for line in file:

        # Split the cells before the capital letters in the middle,
        # then join the parts by a space
        new_line = " ".join(camel_case.split(line))
        
        # Join each cell with a tab, then add the changed line to the list
        edited_lines.append(new_line)

# Save a file with all the changed lines
with open("Save File Structure - 1.0.9.55 - new.tsv", "wt") as file:
    file.writelines(edited_lines)