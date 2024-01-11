# GMu

**GMu** is a Game Boy emulator. The goal of this project was to teach myself a little bit about computer architecture, and low-level programming.

This is my first programming project, so keep that in mind.

## Build
Assuming MSVC is installed, the win32_build.bat file should generate an exe that can run. At this point, the source code has to be changed each time you want to load a new ROM, which is inconvenient, but working on a way to change this while the emulator is running. This change can be made in nenjin.cpp.

If the build fails, you will need to locate the vcvarsall.bat file on your system, and change line 3 of the bat file to that directory.

## TODO
- Add UI again.
    - Add the ability to load new ROMs in without having to recompile.
    - Make toggles for debug info.
    - Add color palette options.
- Need to implement STOP opcode, currently resets the system.
- Fully add support for MBC1, MBC2, MBC3, and MBC5.
- Clean up the code, lots of inconsistencies.
- Add save states.
- Fix the PPU bugs.

### Special thanks to Casey Muratori for the handmade hero project, and Javidx9(OneLoneCoder) for the NES emulator series. 
