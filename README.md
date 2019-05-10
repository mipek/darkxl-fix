# DarkXL Fix
This fixes some bugs for the XL engine port of the classic dark forces game from 1995.
I created a topic back in the day on the XL forums but the site is currently down: http://xlengine.com/forums/viewtopic.php?f=19&t=1062

This is a old source from 2015 but it still works at least on Windows 7 and Windows 10 (Version 1809). Have fun!

## Install
1. Download (https://github.com/mipek/darkxl-fix/releases)
2. Extract contents of the archive to the folder where DarkXL.exe resides in
3. Run DarkXL (you should see a text "Alpha 9.50 DarkFX" in the lower right corner of the screen in the main menu if everything worked)

## Configuration
If you experience crashes or don't want to enable a certain patch just open "darkxl_fix.ini" via your favorite text editor. You can use "true" or "false" to enable/disable a certain patch.

## How does it work
This is basically a hack that patches code into DarkXL in order to fix some issues.
The DLL is called d3d9.dll because this is a so called DLL proxy. DarkXL uses DirectX9 (hence it loads d3d9.dll).