@echo off

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" x64

mkdir ..\..\build
pushd ..\..\build
del *.pdb > NUL 2> NUL
echo WAITING FOR PDB > lock.tmp
cl -MTd -nologo -Gm- -O2 -Oi -WX -W4 -wd4201 -wd4505 -wd4100 -wd4189 -wd4701 -DH_INTERNAL -DHANDMADE_SLOW -FC -Zi ..\base\code\handmade.cpp /LD /link /pdb:handmade%random%.pdb /EXPORT:GameUpdateAndRender /EXPORT:GameGetSoundSamples
del lock.tmp
cl -MTd -nologo -Gm- -O2 -Oi -WX -W4 -wd4201 -wd4505 -wd4100 -wd4189 -wd4701 -DH_INTERNAL -DHANDMADE_SLOW -FC -Zi ..\base\code\handmade_win32.cpp user32.lib gdi32.lib winmm.lib

cl -MTd -nologo -Gm- -Od -Oi -WX -W4 -wd4201 -wd4505 -wd4100 -wd4189 -wd4701 -DH_INTERNAL -DHANDMADE_SLOW -FC -Zi -D_CRT_SECURE_NO_WARNINGS ..\base\code\handmade_test_asset_builder.cpp gdi32.lib
popd
