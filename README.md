# Waveform Audio Player

Enjoy this original, state-of-the-art music player.

![WAP](https://github.com/nialredha/WAP/blob/master/assets/WAP.png)

## Build and Run

WINDOWS (x64)

1. [Setup a build system for Windows terminal](https://www.youtube.com/watch?v=Ee3EtYb8d1o) 
2. Download [SDL2-devel-x.x.x-VC](https://github.com/libsdl-org/SDL/releases) and [SDL2\_ttf-devel-x.x.x-VC](https://github.com/libsdl-org/SDL_ttf/releases) to somewhere on your hardrive (e.g. C:\SDL2)
3. Move the SDL\_ttf header and library files to the same folder the SDL2 files are located.
4. Open `win_build.bat` in the base of this repo and change the SDL2 paths to match the location of SDL2 on your machine.
5. `win_build.bat` to build
6. `cd build && main.exe` to run

NOTE: might have to change `SDL_AUDIODRIVER` environment variable in `win_build.bat`. More information [here](https://stackoverflow.com/questions/22960325/no-audio-with-sdl-c).

Linux (WSL Ubuntu)

1. `sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev clang`
2. `sh lin_build.sh` to build
3. `./build/wap` to run
