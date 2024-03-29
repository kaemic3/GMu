@echo off
REM This sets the cl environment vars
pushd C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\
call vcvarsall.bat x64 &set > NUL 2>&1
popd
pushd
REM NOTE: I think this pipes output to a temp dir??
cd %~dp0
REM NOTE: This REMed stuff below is for reference. Pulled from handmade hero!
REM set CommonCompilerFlags=-MTd -nologo -GR- -EHa- -Od -Oi -WX -W4 -wd 4201 -wd 4100 -wd 4189 -wd 4456 -wd 4505 -D NENJIN_INTERNAL=1 -D NENJIN_W32=1 -D NENJIN_SLOW=1 -FC -Zi -Fm
REM set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

REM For SDL build
REM set CommonCompilerFlags=-MTd -nologo -GR- -EHa- -Od -Oi -WX -W4 -wd 4201 -wd 4100 -wd 4189 -wd 4456 -wd 4505 -FC -Zi -Fm
REM TODO(kaelan): Need to add -WX again, but there are a lot of warnings....
set CommonCompilerFlags=-MTd -nologo -GR- -EHa- -Od -Oi -wd 4201 -wd 4100 -wd 4189 -wd 4456 -wd 4505 -FC -Zi -Fm -D NENJIN_INTERNAL=1 -D NENJIN_SLOW=1 -D NENJIN_SDL=1 /std:c++17 /EHsc 
set CommonLinkerFlags= -incremental:no 
set SDLCompilerFlags= /I ..\dependencies\SDL\Windows\include /I..\dependencies\SDL\Windows\ttf\include\ /I..\dependencies\stb\ /EHsc
set SDLLinkerFlags= -LIBPATH:..\dependencies\SDL\Windows\lib\x64\ -LIBPATH:..\dependencies\SDL\Windows\ttf\lib\x64\ /subsystem:windows SDL2.lib SDL2main.lib SDL2_ttf.lib shell32.lib


REM TODO - can we just build both with one exe?

IF NOT EXIST build mkdir build
pushd build
REM NOTE: This REMed stuff below is for reference. Pulled from handmade hero!
REM 32-bit build
REM cl %CommonCompilerFlags% ..\src\win32_main.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%
REM 64-bit build
REM Optimization switches /O2 /Oi /fp:fast 
REM del *.pdb > NUL 2> NUL
REM echo WAITING FOR PDB > lock.tmp
REM cl %CommonCompilerFlags% ..\src\nenjin.cpp -LD /link -incremental:no -opt:ref -PDB:nenjin_%random%.pdb -EXPORT:NenjinGetSoundSamples -EXPORT:NenjinUpdateAndRender
REM del lock.tmp
REM cl %CommonCompilerFlags% ..\src\win32_main.cpp /link %CommonLinkerFlags%

REM TODO(kaelan): This kinda sucks to compile. Should migrate to single translation unit style compiling!
cl %SDLCompilerFlags% %CommonCompilerFlags% ..\src\sdl_main.cpp /link %SDLLinkerFlags% %CommonLinkerFlags%
popd
