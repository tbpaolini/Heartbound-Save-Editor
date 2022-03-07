"""
This script records the current room's name and coordinates to an output file,
when the 'space' hotkey is pressed during gameplay. It can register the press
even when the script window is minimized.

The script relies on the private Wiki Warriors build of Heartbound, which can
save the game anywhere by pressing space. On the regular public build, only
the last saved room will be recorded.
"""

import os
import keyboard     # Third party module: https://pypi.org/project/keyboard/
from time import sleep

SAVE_PATH = f"{os.getenv('LocalAppData')}\\Heartbound\\heartbound_save8.thor"
OUTPUT_PATH = "..\structure\\room_coordinates-raw.tsv"

def parse_save():

    # Get the room name and coordinates from the save file
    with open(SAVE_PATH, "rt", encoding="utf-8") as save_file:
        save_file.readline()
        room = save_file.readline().strip()
        x_axis = save_file.readline().strip()
        y_axis = save_file.readline().strip()
    
    # Append the room name and coordinates to the output file
    with open(OUTPUT_PATH, "at", encoding="utf-8") as out_file:
        out_text = f"{room}\t{x_axis}\t{y_axis}\n"
        out_file.write(out_text)
        print(f"Mapped:\t{out_text}", end="")


if __name__ == "__main__":

    print("Press space while playing Heartbound to map its current room's coordinates to file...")

    """NOTE
    This script relies on the private Wiki Warriors build of Heartbound, which can save the game anywhere by pressing space.
    On the regular public build of Heartbound, only the last saved room will be recorded.
    """
    
    try:
        while True:
            keyboard.wait("space")  # Block until space is pressed
            sleep(0.1)              # Wait 0.1 seconds so the save file can be updated by the game
            parse_save()            # Parse the room coordinates from save file and append them to the output file

    except KeyboardInterrupt:
        # Exit cleanly after pressing Ctrl+C
        print("Script closed by user.")
        raise SystemExit