The save file structure has been ontained from [https://nav.steets.tech/hbsavestruc](https://nav.steets.tech/hbsavestruc). The structure has been built with valuable information provided by Jason Thor Hall (lead developer of Heartbound); and put together by Steets, Terra Hatvol, and me (Tiago Becerra Paolini).

The Excel (`.xlsx`) file is the unmodified file downloaded from the aforementioned link. This file has been converted to Tabulation Separated Values (`.tsv`). Then I made a Python script that makes some changes to the file, in order to make the text more user friendly. Most notably adding spaces before uppercase characters in the middle of words (CamelCase), in order to make the text more user friendly.

This new `.tsv` file will be parsed by the editor in order to build the user interface.
