# Heartbound Save Editor
This program allows you to change everything on the save file of [Heartbound](https://store.steampowered.com/app/567380/Heartbound/), like where you are and what you did or didn't.

**Download:** [Heartbound Save Editor - v1.0.0 (Windows 10)](https://github.com/tbpaolini/Heartbound-Save-Editor/releases/download/v1.0.0/Heartbound_Save_Editor-v1.0.0-Windows_10.zip)

The program does not need installation. Just extract the zip file to any place you want, then you can just run the Heartbound Save Editor :)

The editor works with latest version of Heartbound (1.0.9.55). Once the game gets updated, you might need to download a new version of the save editor.

## Screenshots
<img src="https://user-images.githubusercontent.com/85261542/164076804-5d3bc2bb-d81e-4bbe-9340-1562f7648225.png" width=325 /> <img src="https://user-images.githubusercontent.com/85261542/164076838-c28a40d4-5c64-4902-8a79-a7dba8a7cc99.png" width=325 />

## Compilation instructions
It is not really necessary to compile the program by yourself, since you can just download it. But if for any reason you want to build the program from the source, then this instructions should help you.

Heartbound Save Editor was programmed in the C language, using [GTK 3](https://www.gtk.org/) for the user interface. In order to be able to compile the editor, first you need to setup the MingW and GTK frameworks, both of which can be installed using [MSYS2](https://www.msys2.org/) package manager:
* Download and install MSYS2 from [its official download](https://github.com/msys2/msys2-installer/releases/download/2022-03-19/msys2-x86_64-20220319.exe) (we recommend you to keep its default installation folder).
* Open MSYS2 (a command line shell will open, where you type commands).
* Update its package database by running the command `pacman -Syu`
* Once MSYS2 is done updating, close it and then open it again.
* Now updgrade the packages by running the command `pacman -Su`
* Install MingW and GTK by running this command on MSYS2:
```shell
pacman -S --needed base-devel mingw-w64-x86_64-toolchain mingw-w64-x86_64-gtk3
```
* Now you can close MSYS2
* Add the path of MingW's "bin" folder to the PATH environment variable of Windows. If you installed MSYS2 to the its default folder, that MingW's folder is at `C:\msys64\mingw64\bin` . The PATH environment variable can be edited by:
    * In Search, search for and then select: Edit System Environment Variables
    * Click the "Environment Variables" button.
    * Chose the "Path" variable and click on the "Edit" button.
    * Click on the "New" button and input the MingW's folder: `C:\msys64\mingw64\bin`
    * Then click OK to confirm on all dialogs

If you haven't downloaded the source code of Heartbound Save Editor, it can be [downloaded here](https://github.com/tbpaolini/Heartbound-Save-Editor/archive/refs/heads/master.zip). Unzip the project wherever you want. Then you can just run `compile.bat` script in the project folder. It will ask which build you want to compile (Release, Debug, or both). Once the compilation is done, the program will be saved to the `build` folder inside the project.

The debug build is almost the same as the release build, with two differences:
* It prints errors and messages to a terminal window.
* It can be used with [GDB](https://www.sourceware.org/gdb/), or another similar debugging tool, in order to debug the source code line by line.

The source code can be edited with [Visual Studio Code](https://code.visualstudio.com/), or another code editor of your preference.

Alternatively, it is also possible to compile Heartbound Save Editor by opening a Windows Command Prompt on the project's folder, then running one of these commands there:
* `mingw32-make -B` (for the release build)
* `mingw32-make -B debug` (for the debug build)

## Contact
You are welcome to leave comments and suggestions on [Discussions](https://github.com/tbpaolini/Heartbound-Save-Editor/discussions), to report bugs on [Issues](https://github.com/tbpaolini/Heartbound-Save-Editor/issues), and security matters on [Security](https://github.com/tbpaolini/Heartbound-Save-Editor/security).
