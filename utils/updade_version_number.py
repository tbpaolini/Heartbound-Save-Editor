#!/usr/bin/env python

help_text = """
This Python script replaces the version number occurrences on the source files,
by the number specified as the command line argument of the script.

Usage:
    python3 updade_version_number.py N.N.N.N

    Where N is an unsigned integer.
    For example: 1.0.0.4

Note:
    This script is used by the 'dual_compile.bat' script at the utils folder of
    the Windows version. That script will create a git commit on the 'development'
    and 'linux' branches with the new updated version number.
    
    For this reason, this python script does not update the version number at the
    '.bat' script, which should be updated manually if you want to run the '.bat'.
"""

import os, sys, re
from pathlib import Path

# Input validation

def exit_failure() -> None:
    print(help_text)    # Print the help text if the input is incorrect
    sys.exit(1)

if (len(sys.argv) != 2):    # Is there exactly one argument? (besides the file itself)
    exit_failure()

version = sys.argv[1].strip().split(".")  # Store the version number as a list of strings

if (len(version) != 4):     # Do we have exactly 4 valyes on the list?
    exit_failure()

for value in version:
    if (not value.isdecimal()):     # Are all velues decimal numbers?
        exit_failure()

# Create the version strings
version_dots = ".".join(value for value in version)     # Version number separated by dots
version_commas = ",".join(value for value in version)   # Version number separated by commas

# Change the working directory to the project's root
os.chdir(Path(__file__).parents[1])

# Update the files

# Version number at the "About" window
current_path = Path("includes", "hb_gui_callback.c")
about_regex = re.compile(r'(        "version", "Version )(\d+\.\d+\.\d+\.\d+)(",\n)')

with open(current_path, "r+") as file:
    lines = file.readlines()
    file.seek(0)

    for i in range(len(lines)):
        lines[i] = about_regex.sub(fr"\g<1>{version_dots}\g<3>", lines[i])
    
    file.writelines(lines)

# Version numbers at the Windows Resource File (executable's metadata)
current_path = current_path = Path("resources.rc")

if current_path.exists():
    
    res_int_regex = re.compile(r'(FILEVERSION\s+|PRODUCTVERSION\s+)(\d+,\d+,\d+,\d+)(\n)')
    res_str_regex = re.compile(r'(\s+VALUE "(?:FileVersion|ProductVersion)",\s+")(\d+\.\d+\.\d+\.\d+)(\\0"\n)')
    
    with open(current_path, "r+") as file:
        lines = file.readlines()
        file.seek(0)

        for i in range(len(lines)):
            lines[i] = res_int_regex.sub(fr"\g<1>{version_commas}\g<3>", lines[i])
            lines[i] = res_str_regex.sub(fr"\g<1>{version_dots}\g<3>", lines[i])
        
        file.writelines(lines)

# Downloads at the main README file
current_path = current_path = Path("README.md")

readme_regex = re.compile(r'(\* .+ \[Heartbound Save Editor - v)(\d+\.\d+\.\d+\.\d+)( \(.+\)\]\(https://github\.com/tbpaolini/Heartbound-Save-Editor/releases/download/v)(\d+\.\d+\.\d+\.\d+)(/Heartbound_Save_Editor-v)(\d+\.\d+\.\d+\.\d+)(.+\n)')

with open(current_path, "r+") as file:
    lines = file.readlines()
    file.seek(0)

    for i in range(len(lines)):
        lines[i] = readme_regex.sub(fr"\g<1>{version_dots}\g<3>{version_dots}\g<5>{version_dots}\g<7>", lines[i])
    
    file.writelines(lines)

# Version at the Debian package
current_path = Path("packaging", "DEBIAN", "control")

if current_path.exists():
    
    deb_regex = re.compile(r'(Version: )(\d+\.\d+\.\d+\.\d+)(-1\n)')
    
    with open(current_path, "r+") as file:
        lines = file.readlines()
        file.seek(0)

        for i in range(len(lines)):
            lines[i] = deb_regex.sub(fr"\g<1>{version_dots}\g<3>", lines[i])
        
        file.writelines(lines)