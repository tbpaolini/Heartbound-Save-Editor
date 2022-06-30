# Heartbound Save Editor
This program allows you to change everything on the save file of [Heartbound](https://store.steampowered.com/app/567380/Heartbound/), like where you are and what you did or didn't.

**Downloads:**
* *Windows:* [Heartbound Save Editor - v1.0.0.4 (Windows 10)](https://github.com/tbpaolini/Heartbound-Save-Editor/releases/download/v1.0.0.4/Heartbound_Save_Editor-v1.0.0.4-Windows_10.zip) - standalone executable, requires no installation.
* *Linux (Debian/Ubuntu):* [Heartbound Save Editor - v1.0.0.4 (Debian/Ubuntu package)](https://github.com/tbpaolini/Heartbound-Save-Editor/releases/download/v1.0.0.4/Heartbound_Save_Editor-v1.0.0.4-Linux_Ubuntu.deb) - installation package, should work on Debian/Ubuntu and distros based on those.
* *Linux (binary):* [Heartbound Save Editor - v1.0.0.4 (Linux binary)](https://github.com/tbpaolini/Heartbound-Save-Editor/releases/download/v1.0.0.4/Heartbound_Save_Editor-v1.0.0.4-Linux_binary.tar.xz) - just the binary, in order to run you first need to install the `libgtk-3-0` package.

The editor works with latest version of Heartbound (1.0.9.57). Once the game gets updated, you might need to download a new version of the save editor.

## Screenshots
<img src="https://user-images.githubusercontent.com/85261542/167233908-f98e048d-9f43-4d27-b955-51b85a712893.png" width=325 /> <img src="https://user-images.githubusercontent.com/85261542/167233948-84a4e34a-b4b3-4b1f-b132-1179e3fa43ac.png" width=325 />

## Compilation instructions
It is not really necessary to compile the program by yourself, since you can just download it. But if for any reason you want to build the program from the source, then this instructions should help you.

### Windows

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
* Has the GTK Inspector enabled (Menu: Help > Interactive debugging).
* It prints errors and messages to a terminal window.
* It can be used with [GDB](https://www.sourceware.org/gdb/), or another similar debugging tool, in order to debug the source code line by line.

The source code can be edited with [Visual Studio Code](https://code.visualstudio.com/), or another code editor of your preference.

Alternatively, it is also possible to compile Heartbound Save Editor by opening a Windows Command Prompt on the project's folder, then running one of these commands there:
* `mingw32-make -B` (for the release build)
* `mingw32-make -B debug` (for the debug build)

### Linux

First, be sure that you are using our Linux codebase, which is not on the [linux branch](https://github.com/tbpaolini/Heartbound-Save-Editor/tree/linux) of this repository ([download](https://github.com/tbpaolini/Heartbound-Save-Editor/archive/refs/heads/linux.zip)).

In order to compile Heartbound Save Editor on Linux, you need these packages: GTK 3 Development (`libgtk-3-dev`), the GNU Compiler Collection (`gcc`), the makefile build system (`make`), and `pkg-config`.

The instructions for installing Linux package should vary depending on the Linux distro, so you should look up for the shell command for installing packages on your distro, and then get the aforementioned packages.

On Ubuntu, the command is:
```sh
sudo apt install libgtk-3-dev build-essential
```
(the `build-essential` package already includes quite a few tools for compiling and packaging programs)

Then open a terminal on the directory where is the source code of Heartbound Save Editor, and run one of these commands:
* `make -B` (for the release build)
* `make -B debug` (for the debug build)

The `-B` flag forces the entire project to be rebuilt from scratch, which for this project should be very quick. Then you can run `make clean` to remove the temporary files that were created during compilation.

Alternatively, you can instead just run the script `compile.sh` that will ask you which build you want to make and then clean the temporary files after it is finished.

The Release and Debug builds are almost the same, with the differences that the latter includes GTK Inspector (Menu: Help > Interactive debugging), debug symbols and display some additional messages on the terminal. This build can be used with some debugging tool, like the `gdb` package, in order to debug the program line by line.

## Contact
You are welcome to leave comments and suggestions on [Discussions](https://github.com/tbpaolini/Heartbound-Save-Editor/discussions), to report bugs on [Issues](https://github.com/tbpaolini/Heartbound-Save-Editor/issues), and security matters on [Security](https://github.com/tbpaolini/Heartbound-Save-Editor/security).
