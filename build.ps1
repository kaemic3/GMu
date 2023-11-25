pushd build
del *.pdb 2> NUL
$CommonCompilerFlags= "-MTd -nologo -GR- -EHa- -Od -Oi -WX -W4 -wd 4201 -wd 4100 -wd 4189 -wd 4456 -D ENGINE_INTERNAL=1 -D ENGINE_W32=1 -D ENGINE_SLOW=1 -FC -Zi -Fm".Split(" ")
$CommonLinkerFlags= "-opt:ref user32.lib Gdi32.lib winmm.lib".Split(" ")
$PDBName = Get-Random
# cl $CommonCompilerFlags ..\src\win32_main.cpp /link -subsystem:Windows,5.1 $CommonLinkerFlags 
cl $CommonCompilerFlags ..\src\nenjin.cpp -LD /link -incremental:no -PDB:nenjin_$PDBName.pdb -EXPORT:EngineUpdateAndRender -EXPORT:EngineGetSoundSamples
cl $CommonCompilerFlags ..\src\win32_main.cpp /link $CommonLinkerFlags 
popd
