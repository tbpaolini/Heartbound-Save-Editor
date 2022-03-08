"""
This Python script prints the rooms that were not yet mapped by the
'room_mapping.py' script
"""

with open("..\\structure\\Save File Structure - Room IDs.tsv", "rt") as ids_file:
    ids_file.readline()
    room_ids = {line.strip() for line in ids_file}

with open("..\\structure\\room_coordinates-raw.tsv") as coords_file:
    mapped = {line.split("\t")[0] for line in coords_file}

for room in sorted(room_ids - mapped):
    print(room)

# input()