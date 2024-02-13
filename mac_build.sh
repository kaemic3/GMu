#!/bin/sh
pushd build
# NOTE: If your SDL library is NOT install in /opt/homebrew/opt/sdl2 then you will need to change the include and library directories to build!
clang++ -Wno-everything -std=c++17  -I/opt/homebrew/opt/sdl2/include/SDL2 -I../dependencies/stb ../src/sdl_main.cpp -D NENJIN_SLOW=1 -D NENJIN_SDL=1 -o sdl_GMu.o -L/opt/homebrew/opt/sdl2/lib/ -lSDL2
popd