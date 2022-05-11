:: This Windows shell script relies on WSL (Windows Subsystem for Linux), in
:: order to compile both the Windows and Linux builds.
:: It also creates the compressed files of the release builds, and places
:: them in the build\packages\ folder so they can be published on GitHub.
::
:: I ran this script on Windows 10, with WSL version 2 running Ubuntu 20.04.
:: Installation instructions (on Windows):
::     - WSL2: https://docs.microsoft.com/windows/wsl/install
::     - Ubuntu: https://ubuntu.com/wsl
::
:: If you intend to run this script by yourself, you need to have the
:: Heartbound Save Editor's git repository somewhere on the Linux installation,
:: set to the linux branch. The repository on Windows needs to be on the
:: development branch. Also you need to have set up the development environment
:: on both systems (check the README on the project's root for instructions).
::
:: You might also need to edit the path on Linux bellow, in order to match the
:: location of the repository on your system. The paths must be This script
:: assumes that it is being run from the utils folder on the repository at Windows.

@echo off

SET LINUX_PATH=~/hbse
SET VERSION=1.0.0.3

cd ..\
git pull

mingw32-make -B
mingw32-make -B debug
mingw32-make zip
mingw32-make clean

mkdir build\packages
copy "build\windows\Heartbound Save Editor.zip" "build\packages\Heartbound_Save_Editor-v%VERSION%-Windows_10.zip"

wsl -- cd %LINUX_PATH%; git pull; make -B; make -B debug; make tar; make clean
wsl -- cp -rv %LINUX_PATH%/build/linux build/linux
wsl -- cp -v %LINUX_PATH%/build/linux/heartbound-save-editor_release.deb build/packages/Heartbound_Save_Editor-v%VERSION%-Linux_Ubuntu.deb
wsl -- cp -v %LINUX_PATH%/build/linux/heartbound-save-editor.tar.xz build/packages/Heartbound_Save_Editor-v%VERSION%-Linux_binary.tar.xz

cd utils

echo Finished!
pause