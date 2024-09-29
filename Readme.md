# GMu

**GMu** is a Game Boy emulator. The goal of this project was to teach myself a little bit about computer architecture, and low-level programming.

This is my first programming project, so keep that in mind.

Playable demo is located here: https://kaemic3.github.io/Demos/GMu

## Usage

Currently, Windows and MacOSX are supported. SDL2 is used for the Mac version, so Linux should build if you make a script for it.

Additionally, MBC0 and most MBC1 games are supported.

The emulator will only see ROM files that are put into the data/ROMs directory. For now, file dialog boxes are not working. I am planning on looking into this later on, but due to a lack of understanding on my part, it will take some time.

By default, gb_snek.gb is loaded. 
- To un-pause the emulator, press "P".
- To load another ROM file, you can use the key bind combo "left alt + F". Then use the arrow keys to pick one from the list, then enter to load it.
- "WASD" control up, down, left, and right.
- "F" = B, "G" = A, "H" = select, "J" = start.

## Build
Assuming MSVC is installed, the win32_build.bat file should generate an exe that can run.

If the build fails, you will need to locate the vcvarsall.bat file on your system and change line 3 of the bat file to that directory. 

Build with -O2 for faster performance!

Run the mac_build.sh script for building on Mac. If you have installed SDL2 through brew it should work no problem, but I have had issues with brew installing things into different directories.
If you run into issues with it, make sure that the include and library directories specified in the build script are the same as your system.

## Bugs
There a a lot of bugs at the moment. But most games are playable.

Known:
- Zelda dialogue does not work correctly.
- Top of the screen in Mario has tearing.
- Transparency issues.

## TODO
- Finish UI.
    - Make the ROM menu more robust (adding scrolling or pages?)
        - Need to add fallback code for when files that are not properly structured are loaded?     
    - Make toggles for debug info.
    - Add color palette options.
    - Create a menu bar?
    - Add text that shows the key binds!
- Need to implement STOP opcode, currently resets the system.
- Fully add support for MBC1, MBC2, MBC3, and MBC5.
- Clean up the code, lots of inconsistencies.
- Add save states.
- Fix the PPU bugs.

### Special thanks to Casey Muratori for the handmade hero project, and Javidx9(OneLoneCoder) for the NES emulator series. 
