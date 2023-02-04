@echo off

if not exist "build\" mkdir build\
if not exist "lib\" mkdir lib\

cd build/

cl /c ..\src\SoundSim.cpp ..\src\WaveIO.cpp /I ..\include
lib /OUT:..\lib\sound.lib SoundSim.obj WaveIO.obj

cl ..\src\main.cpp /I ../include /I C:\SDL2\include /link /LIBPATH:C:\SDL2\lib\x64\ /LIBPATH:..\lib user32.lib SDL2main.lib SDL2.lib sound.lib shell32.lib /SUBSYSTEM:CONSOLE

cd ..