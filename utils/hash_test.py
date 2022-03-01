from collections import Counter
from secrets import choice, randbelow
from string import ascii_letters

with open("..\structure\save_structure.tsv", "rt") as file:
    file.readline()
    places = {line.split("\t")[2] for line in file}

def hashbrown(key:str, slots:int=300, bits=32) -> int:
    result = 0xA5A5A5A5
    key = key.encode("ascii")
    mask = 1 << (bits - 1)
    size = 1 << bits
    for char in key:
        result = (result + char) % size
        result = (char * result) % size
        msb = (result & mask ) >> (bits - 1)
        result = ((result << 1) % size) | msb
    
    return result % slots

spread1 = Counter()
ref = Counter()
slots = 300
elements = 100
for i in range(elements):
    key = "".join(choice(ascii_letters) for j in range(randbelow(32)))
    spread1.update((hashbrown(key, slots),))
    ref.update((randbelow(slots),))

spread_sum = sum(value[1] for value in spread1.most_common())
ref_sum = sum(value[1] for value in ref.most_common())

print(f"Unique: {len(spread1)} (reference: {len(spread1) * 100 / len(ref):.2f}%)")
print(f"Big O time: {elements / len(spread1):.3f} (reference: {elements / len(ref):.3f})")