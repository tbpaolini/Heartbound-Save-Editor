"""
Python script for sorting alphabetically the rooms that were mapped by 'room mapping.py'.
"""

with open("..\\structure\\room_coordinates-raw.tsv", "rt") as input_file:
    rows = [line.split("\t") for line in input_file]

with open("..\\structure\\room_coordinates.tsv", "wt") as output_file:
    output_file.write("Index\tRoom\tx-axis\ty-axis\n")
    lines = [f"{index}\t" + "\t".join(room) for index, room in enumerate(sorted(rows))]
    output_file.writelines(lines)