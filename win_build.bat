@echo off

set path=C:\SDL2\lib\x64;%path%
set SDL_AUDIODRIVER=dsound

if not exist "build\" mkdir build\

cd build/

cl ..\src\main.cpp /EHsc /I ../include /I C:\SDL2\include /link /LIBPATH:C:\SDL2\lib\x64\ user32.lib SDL2main.lib SDL2.lib SDL2_ttf.lib shell32.lib /SUBSYSTEM:CONSOLE 

cd ..
