"""
This Python script copies the necessary DLL's from the GTK installation folder to the project folder.
The script works on the output of Process Explorer for the 'Heartbound Save Editor.exe' process.
Process Explorer can be downloaded here: https://docs.microsoft.com/en-us/sysinternals/downloads/process-explorer

Then you should run both Heartbound Save Editor and Process Explorer. On the later, select the Editor's process,
then on the menu 'File > Save as...' to save the information about the process.
Save it as 'Process Explorer - Heartbound Save Editor.exe.txt' on the same folder as this script.

Now you may close both programs and run this script.
"""

import re
from pathlib import Path
from shutil import copyfile

# Read the file into memory
with open("Process Explorer - Heartbound Save Editor.exe.txt", "rt", encoding="utf-8") as process_file:
    process_text = process_file.read()

# Extract from the file the path all DLL's that are on the MinGW's folder
dll_regex = re.compile(r"(C:\\msys64\\mingw64\\bin\\.+\.dll)")
dll_list = dll_regex.findall(process_text)

# Copy the DLL's into the project folder
for dll in dll_list:
    dll_path = Path(dll)
    dll_name = dll_path.name
    dll_destination = Path(r"..\gtk3\bin") / dll_name
    copyfile(dll_path, dll_destination)
    print("Imported:", dll_destination)