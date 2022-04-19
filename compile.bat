:: This Windows batch script is here as a convenience for compiling Heartbound Save Editor
:: In order for it to work, you need to have installed MSYS2, and then use it to install MingW and Gtk3
:: Also, MingW's "bin" folder needs to be added PATH environment variable.
:: For convenience, this script is temporarily adding its default folder to PATH, but you should do so
:: permanently on Windows settings if you want to work with MingW.

:: Detailed instructions for setting up all those things: https://www.gtk.org/docs/installations/windows/

@echo off

path C:\msys64\mingw64\bin;%PATH%

:START
    echo Please type the number of the build you want to compile.
    set /p CHOICE="(1) Release - (2) Debug - (3) both: "

if %CHOICE% == 1 goto RELEASE
if %CHOICE% == 2 goto DEBUG
if %CHOICE% == 3 (goto BOTH) else (goto START)

:RELEASE
    mingw32-make -B
    goto END

:DEBUG
    mingw32-make -B debug
    goto END

:BOTH
    mingw32-make -B
    mingw32-make -B debug
    goto END

:END
    pause
    mingw32-make clean