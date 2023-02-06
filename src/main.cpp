/* Example use case for WaveIO and SoundSim 
*/ 

#include <cmath>
#include <iostream>

#include "SoundSim.hpp"
#include "WaveIO.hpp"

#ifdef __linux__
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_image.h>
#elif _WIN32
    #include <SDL.h>
    #include <SDL_image.h>
#endif

std::string read_input_fname(int argc, char* argv[])
{
    std::string data_dir{};
	if (argc < 2)
	{
		std::cerr << "Please provide a path to the Data directory!" << std::endl;
		exit(1);
	} 
	else { data_dir = argv[1]; }

    return data_dir;
}

void play_audio(std::string data_dir) 
{
    // SDL Audio Test
    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8* wavBuffer;

	std::string output_fname = data_dir + "/d7_test.wav";

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


    SDL_CloseAudioDevice(deviceId);
    SDL_FreeWAV(wavBuffer);

}

void read_write_wav(std::string data_dir)
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

}

typedef struct {
    int x, y; 
} Mouse_Pos;

void run_gui(std::string data_dir)
{
    // SDL Screen Test
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Surface* surface = nullptr;
    SDL_Texture* texture = nullptr;

    // INITIALIZATION
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Init Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    window = SDL_CreateWindow("Testing", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
    if(window == nullptr)
    {
        std::cerr << "Create Window ERROR: " << SDL_GetError() << std::endl;
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == nullptr)
    {
        std::cerr << "Create Renderer ERROR: " << SDL_GetError() << std::endl;
        exit(1);
    }

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    IMG_Init(IMG_INIT_PNG);

    // LOAD TEXTURE
    std::string crosshair_path = data_dir + "/crosshair.png";

    surface = IMG_Load(crosshair_path.c_str());
    if(surface == nullptr)
    {
        std::cerr << "IMG Load ERROR: " << IMG_GetError() << std::endl;
        exit(1);
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    SDL_ShowCursor(0);

    if(texture == nullptr)
    {
        std::cerr << "Create Texture From Surface ERROR: " << SDL_GetError() << std::endl;
        exit(1);
    }
    SDL_FreeSurface(surface);

    bool quit = false;
    SDL_Event event;
    Mouse_Pos mouse;
    SDL_Rect dest;
    while(!quit)
    {
        SDL_WaitEvent(&event);
        if(event.type == SDL_QUIT)
        {
            quit = true;
        }

        SDL_GetMouseState(&mouse.x, &mouse.y);
        dest.x = mouse.x;
        dest.y = mouse.y;
        SDL_QueryTexture(texture, nullptr, nullptr, &dest.w, &dest.h); 
        dest.x -= dest.w / 2;
        dest.y -= dest.h / 2;

        // clear screen
        SDL_RenderClear(renderer);

        // redner texture to screen
        SDL_RenderCopy(renderer, texture, nullptr, &dest); 

        // update screren 
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char *argv[]) 
{
    std::string data_dir = read_input_fname(argc, argv);
    run_gui(data_dir);

	return 0;
}

