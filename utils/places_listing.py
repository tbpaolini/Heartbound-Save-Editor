"""
Python script for getting the unique values of the Room/Object column from the
save structure file.
"""

with open("..\structure\save_structure.tsv", "rt") as input_file:
    header = input_file.readline().strip().split("\t")
    my_index = header.index("Room/Object")
    places = {line.split("\t")[my_index] + "\n" for line in input_file}

with open("..\structure\places_list.tsv", "wt") as output_file:
    output_file.write("Room/Object")
    output_file.writelines(sorted(places))