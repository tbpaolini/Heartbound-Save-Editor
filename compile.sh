#!/bin/bash

# This Linux bash script compiles the Release and/or Debug builds, according to the user's choice.
# In order for the build to succeed, the following packages need to be installed:
#   build-essential
#   libgtk-3-dev

echo "Please type the number of the build you want to compile."
select choice in "Release" "Debug" "Both"; do
    case $choice in
        Release ) make -B; break;;
        Debug   ) make -B debug; break;;
        Both    ) make -B; make -B debug; break;;
    esac
done

read -n1 -r -p "Press any key to continue..." key;
make clean;
