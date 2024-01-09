@echo off
REM This sets the cl environment vars
pushd C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\
call vcvarsall.bat x64 &set > NUL 2>&1
popd
pushd
REM NOTE: I think this pipes output to a temp dir??
cd %~dp0
REM NOTE: This REMed stuff below is for reference. Pulled from handmade hero!
set CommonCompilerFlags=-MTd -nologo -GR- -EHa- -Od -Oi  -WX -W4 -wd 4201 -wd 4100 -wd 4189 -wd 4456 -wd 4505 -D NENJIN_INTERNAL=1 -D NENJIN_W32=1 -D NENJIN_SLOW=1 -Zi /std:c++14 /EHsc 

set CommonLinkerFlags= /subsystem:windows -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib shell32.lib coremessaging.lib advapi32.lib 

IF NOT EXIST build mkdir build
pushd build
REM NOTE: This REMed stuff below is for reference. Pulled from handmade hero!
REM 32-bit build
REM cl %CommonCompilerFlags% ..\src\win32_main.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%
REM 64-bit build
REM Optimization switches /O2 /Oi /fp:fast 
REM cl %CommonCompilerFlags% ..\src\nenjin.cpp -LD /I ..\dependencies\stb\ /link -incremental:no -opt:ref -PDB:nenjin_%random%.pdb -EXPORT:NenjinUpdateAndRender
cl %CommonCompilerFlags% ..\src\win32_main.cpp /I ..\dependencies\stb\ /link %CommonLinkerFlags%
REM cl %CommonCompilerFlags% ..\src\win32_main.cpp /I ..\dependencies\stb\ ..\src\Bus.cpp ..\src\SM83.cpp ..\src\DMG_PPU.cpp ..\src\Cartridge.cpp ..\src\Mapper.cpp ..\src\Mapper_00.cpp ..\src\Mapper_01.cpp ..\src\Sprite.cpp ..\src\FG_Fetcher.cpp ..\src\BG_Fetcher.cpp /link %CommonLinkerFlags%
popd
