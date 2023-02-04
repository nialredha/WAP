/* Example use case for WaveIO and SoundSim 
*/ 

#include <cmath>
#include <iostream>

#include "SoundSim.hpp"
#include "WaveIO.hpp"

#ifdef __linux__
    #include <SDL2/SDL.h>
#elif _WIN32
    #include <SDL.h>
#endif

void run(std::string data_dir)
{
    int num_frequencies = 4;
	float* frequencies = (float*)malloc(sizeof(float) * num_frequencies);

	frequencies[0] = 293.665;	// D  - Octave 4
	frequencies[1] = 369.994;	// F# - Octave 4
	frequencies[2] = 440.000;	// A  - Octave 4
	frequencies[3] = 523.251;	// C  - Octave 5

	int sample_rate = 44100;
	int num_samples = sample_rate*10;	// 3 second duration

	Sound_Sim harmonic(num_frequencies, frequencies, num_samples, sample_rate);
   	harmonic.simulate_data();

	std::string input_fname = data_dir + "/d7.wav";
	WaveIO wave(input_fname); // automatically parses the data
	
	wave.print_metadata();

	std::string output_fname = data_dir + "/d7_test.wav";
	wave.set_fname(output_fname);
	wave.write();

	wave.print_metadata();
    
    // SDL Screen Test
    SDL_Window* window = nullptr;
    SDL_Surface* screen_surface = nullptr;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Init Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    window = SDL_CreateWindow("Testing", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
    screen_surface = SDL_GetWindowSurface(window);
    // fill surface white
    SDL_FillRect(screen_surface, nullptr, SDL_MapRGB(screen_surface->format, 0xFF, 0xFF, 0xFF));
    // update the surface 
    SDL_UpdateWindowSurface(window);

    // SDL Audio Test
    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8* wavBuffer;

    SDL_Init(SDL_INIT_AUDIO);
    if (SDL_LoadWAV(output_fname.c_str(), &wavSpec, &wavBuffer, &wavLength) == nullptr)
    {
        std::cerr << "Could not open " << output_fname << std::endl;
        exit(1);
    }

    SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(nullptr, 0, &wavSpec, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (deviceId == 0)
    {
        std::cerr << "Sound Device Error: " << SDL_GetError() << std::endl;
        exit(1);
    }
    std::cout << "Device ID = " << deviceId << std::endl;

    // play audio
    int status = SDL_QueueAudio(deviceId, wavBuffer, wavLength);
    if (status < 0)
    {
        std::cout << "Error Queueing Audio: " << SDL_GetError() << std::endl;
        exit(1);
    }
    std::cout << "Status = " << status << std::endl;

    SDL_PauseAudioDevice(deviceId, 0);

    bool quit = false;
    SDL_Event event;
    while(!quit)
    {
        SDL_WaitEvent(&event);
        if(event.type == SDL_QUIT)
        {
            quit = true;
        }
    }

    SDL_DestroyWindow(window);
    SDL_CloseAudioDevice(deviceId);
    SDL_FreeWAV(wavBuffer);
    SDL_Quit();



}

int main(int argc, char *argv[]) 
{
    std::string data_dir{};
	if (argc < 2)
	{
		std::cerr << "Please provide a path to the Data directory!" << std::endl;
		exit(1);
	} 
	else { data_dir = argv[1]; }

    run(data_dir);
	return 0;

}

/*
int WinMain(int argc, char *argv[]) 
{
	std::string data_dir{};
	if (argc < 2)
	{
		std::cerr << "Please provide a path to the Data directory!" << std::endl;
		exit(1);
	} 
	else { data_dir = argv[1]; }

    std::cin.get();
	return 0;
}
*/
