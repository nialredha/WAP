# ExploreAudio

## Building and Dependencies

**Windows:**

1. Configure C compiler path and all that good stuff
    - find where Visual Studio is on your computer, specifically the `vcvarsall.bat` file. 
    - run `vcvarsall.bat x64` to compile 64bit code. 
    - click [here](https://www.youtube.com/watch?v=Ee3EtYb8d1o) for more info on setting up a build system from Windows CMD. Also, feel free to use the Visual Studio GUI.
2. Download `SDL2-devel-x.x.x-VC` from [here](https://github.com/libsdl-org/SDL/releases) to somewhere on your hardrive (e.g. C:\SDL2)
3. Open `win_build.bat` in the base of this repo and change the SDL2 lib paths to match the location of SDL2 on your machine.
3. run `set path=C:\SDL2\lib\x64;%path%` to add SDL2 dll to your path.
5. run `set SDL_AUDIODRIVER=dsound` or check [here](https://stackoverflow.com/questions/22960325/no-audio-with-sdl-c) for other options if that doesn't work.
6. run `win_build.bat` to build

**Linux (WSL Ubuntu):**

1. `sudo apt install libsdl2-dev clang pulseaudio also-utils`
2. `sh lin_build.sh`

## Known Bugs

1. Linux Audio doesn't work. Need to look into pulseaudio.
2. Need to add error handling.
