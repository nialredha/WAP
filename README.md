# ExploreAudio

Learning what it would take to make a less than good DAW.

## Build and Run

WINDOWS (x64)

1. [Setup a build system for Windows terminal](https://www.youtube.com/watch?v=Ee3EtYb8d1o) 
2. Download [SDL2-devel-x.x.x-VC](https://github.com/libsdl-org/SDL/releases) and [SDL2\_image-devel-x.x.x-VC](https://github.com/libsdl-org/SDL_image/releases) to somewhere on your hardrive (e.g. C:\SDL2)
3. Move the SDL\_image header and library files to the same folder the SDL2 files are located.
4. Open `win_build.bat` in the base of this repo and change the SDL2 paths to match the location of SDL2 on your machine.
5. `win_build.bat` to build
6. `cd build && main.exe <path-to-data-dir>` to run

NOTE: might have to change `SDL_AUDIODRIVER` environment variable in `win_build.bat`. More information [here](https://stackoverflow.com/questions/22960325/no-audio-with-sdl-c).

Linux (WSL Ubuntu)

1. `sudo apt install libsdl2-dev libsdl2-image-dev clang pulseaudio also-utils`
2. `sh lin_build.sh` to build
3. `./build/Audio <path-to-data-dir>` to run

## Known Bugs

1. Linux Audio doesn't work. Need to look into pulseaudio.
2. Need to add error handling.
3. WaveIO wites corrupt wav files from simulated data.
