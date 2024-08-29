@echo off

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" x64

mkdir ..\..\build
pushd ..\..\build
del *.pdb > NUL 2> NUL
cl -MTd -nologo -Gm- -Oi -WX -W4 -wd4201 -wd4505 -wd4100 -wd4189 -wd4701 -DH_INTERNAL -DHANDMADE_SLOW -FC -Zi ..\base\code\handmade_win32.cpp user32.lib gdi32.lib winmm.lib
cl -MTd -nologo -Gm- -Oi -WX -W4 -wd4201 -wd4505 -wd4100 -wd4189 -wd4701 -DH_INTERNAL -DHANDMADE_SLOW -FC -Zi ..\base\code\handmade.cpp /LD /link /pdb:handmade%random%.pdb /EXPORT:GameUpdateAndRender /EXPORT:GameGetSoundSamples
popd