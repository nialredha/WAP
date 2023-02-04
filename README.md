# ExploreAudio

## Building and Dependencies

Windows:

1. find where Visual Studio is on your computer, specifically the `vcvarsall.bat` file to configure the C compiler path and all that good stuff so you can use the `cl` command. Once you have found it run: `vcvarsall.bat x64` to compile 64bit. Click [here](https://www.youtube.com/watch?v=Ee3EtYb8d1o) for a video describing how to setup a build system from Windows CMD. Also, feel free to use the Visual Studio GUI.
2. Download SDL2-devel-x.x.x-VC from [here](https://github.com/libsdl-org/SDL/releases) to somewhere on your hardrive (e.g. C:\SDL2)
3. Open `win_build.bat` in base of repo and change SDL2 paths to match the location of SDL2 on your machine.
3. run `set path=C:\SDL2\lib\x64;%path%` to add SDL2 dll to path.
5. run `set SDL_\AUDIODRIVER=dsound` or check [here](https://stackoverflow.com/questions/22960325/no-audio-with-sdl-c) for other options if that doesn't work.
6. run `win_build.bat`

Linux (WSL Ubuntu):

1. `sudo apt install libsdl2-dev clang pulseaudio also-utils`
2. `sh lin_build.sh`

## Known Bugs

1. Linux Audio doesn't work. Need to look into pulseaudio.

## References That Could be Helpful?

https://github.com/microsoft/WSL/issues/4257

