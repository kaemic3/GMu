# GMu

**GMu** is a Game Boy emulator. The goal of this project was to teach myself a little bit about computer architecture, and low-level programming.

This is my first programming project, so keep that in mind.

## Usage

Currently, Windows is the only OS that is supported.

The emulator will only see ROM files that are put into the data/ROMs directory. For now, file dialog boxes are not working. I am planning on looking into this later on, but due to a lack of understanding on my part, it will take some time.

By default, gb_snek.gb is loaded. 
- To unpause the emulator, press "P".
- To load another ROM file, you can use the keybind combo "left alt + F". Then use the arrow keys to pick one from the list, then enter to load it.
- "WASD" control up, down, left, and right.
- "F" = B, "G" = A, "H" = select, "J" = start.


## Build
Assuming MSVC is installed, the win32_build.bat file should generate an exe that can run.

If the build fails, you will need to locate the vcvarsall.bat file on your system and change line 3 of the bat file to that directory.

## TODO
- Finish UI.
    - Make the ROM menu more robust (adding scrolling or pages?)
        - Need to add fallback code for when files that are not properly structured are loaded?     
    - Make toggles for debug info.
    - Add color palette options.
    - Create a menu bar?
    - Add text that shows the keybinds!
- Need to implement STOP opcode, currently resets the system.
- Fully add support for MBC1, MBC2, MBC3, and MBC5.
- Clean up the code, lots of inconsistencies.
- Add save states.
- Fix the PPU bugs.

### Special thanks to Casey Muratori for the handmade hero project, and Javidx9(OneLoneCoder) for the NES emulator series. 
