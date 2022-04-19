help_text = """
This Python script is meant to be used by makefile in order to copy the needed
GTK files into the build directory.

The script copies only the files that are newer than the ones in the destination,
and it excludes from copying the icons NOT specified on the command line arguments.
Considering that the GTK3 folder of this project has over 14k icons, and the
project only makes use of a handful of them.

Usage:
    python gtk_copy.py src="source folder" dst="destination folder" icons="icon names to include"

    The folders must be relative to the project's root, the same directory where is makefile.
    This script should be in the 'utils' folder of the project.
    The icon names are separated by commas.
"""

from distutils.command.build import build
import os, sys, shutil
from pathlib import Path
from time import perf_counter

# # Used for debugging
# sys.argv = ['gtk_copy.py', 'src=gtk3', 'dst=build\\test', 'icons=document-save,document-open,dialog-error,dialog-warning,image-loading,image-missing']

def help_exit():
    """Prints help text and exit when the script's arguments are malftormated."""
    print(help_text)
    os.system("pause")
    sys.exit(1)

# Do we have 4 command line arguments?
if len(sys.argv) != 4:
    help_exit()

# Parse the arguments
source_folder = destination_folder = icon_names = None

for arg in sys.argv[1:]:
    arg_value = arg.split("=")
    
    if arg_value[0] == "src":
        source_folder = Path(arg_value[1])
    
    if arg_value[0] == "dst":
        destination_folder = Path(arg_value[1])
    
    if arg_value[0] == "icons":
        icon_names = [name + "." for name in arg_value[1].split(",")]

# Exit if any of the arguments was missing
if (source_folder is None) or (destination_folder is None) or (icon_names is None):
    help_exit()

# Get the root folder of the project, and change the current working directory to it
root_folder = Path(__file__).parents[1]
os.chdir(root_folder)

# Number of copied files
copied_num = 0

def ingored_files(folder:str, files:list[str]) -> list[str]:
    """Build a list of files to be ignored by the 'shutil.copytree()' function."""

    global copied_num
    
    # List of files to be ignored
    ignored = ["README.md"]     # The Readme file is ignored by default
    
    # Get the folder's contents
    with os.scandir(folder) as contents:
        
        # Loop through the folder's contents
        for entry in contents:
            
            # Is the entry a file?
            if entry.is_file():

                # Ignore the Readme files
                if entry.name == "README.md": continue

                # Is the file part of the icons folder?
                file_path = Path(entry.path)
                if "share\\icons" in entry.path and (entry.name.endswith(".png") or entry.name.endswith(".svg")):
                    # The file is an icon
                    
                    # Loop through the specified icon names
                    for name in icon_names:
                        
                        # Does the name of the file begins with one of the specified icon names?
                        if entry.name.startswith(name):
                            
                            # Build the path of the destination file
                            file_destination = root_folder / destination_folder / file_path.relative_to(source_folder)
                            
                            # Do not ignore the icon if it does not exist on the destination
                            if (not file_destination.exists()):
                                copied_num += 1
                                print(f"Copied {entry.path}")
                                break
                            
                            # Is the file on the destination more recent or have the same date?
                            if (entry.stat().st_mtime <= os.path.getmtime(file_destination)):
                                # Ignore the file when the destination is not older
                                ignored.append(entry.name)
                                break
                            
                            # No need to check the other names once the name already matched
                            copied_num += 1
                            print(f"Copied {entry.path}")
                            break
                    
                    else:
                        # Ignore the icon if it does not matches any of the names
                        ignored.append(entry.name)
                
                else:
                    # The file is not an icon
                    
                    # Build the path of the destination file
                    file_destination = root_folder / destination_folder / file_path.relative_to(source_folder)

                    # Do not ignore the file if it does not exist on the destination
                    if (not file_destination.exists()):
                        copied_num += 1
                        print(f"Copied {entry.path}")
                        continue

                    # Is the file on the destination more recent or have the same date?
                    if (entry.stat().st_mtime <= os.path.getmtime(file_destination)):
                        # Ignore the file when the destination is not older
                        ignored.append(entry.name)
                    else:
                        copied_num += 1
                        print(f"Copied {entry.path}")
    
    # Returns the list of ignored files
    return ignored

start_time = perf_counter()
shutil.copytree(
    src=source_folder,
    dst=destination_folder,
    ignore=ingored_files,
    dirs_exist_ok=True
)
total_time = perf_counter() - start_time

print(f"Copied {copied_num} new files in {total_time:.1f} seconds.")