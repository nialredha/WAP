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

// TODO: get rid of globals
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Surface* surface = nullptr;
SDL_Texture* texture = nullptr;

SDL_AudioSpec wav_spec;
Uint32 wav_length;
Uint8* wav_buffer;
SDL_AudioDeviceID device_id; 

static Uint8* audio_pos;
static Uint32 audio_len;

typedef struct {
    int x, y; 
} Mouse_Pos;

void audio_callback(void* userdata, Uint8* stream, int len)
{
    memset(stream, 0, len);

    if(audio_len == 0) { return; }

    if(len > audio_len) 
    { 
        len = audio_len; 
    }

    memcpy(stream, audio_pos, len);

    audio_pos += len;
    audio_len -= len;

    if(audio_len == 0)
    {
        audio_pos = wav_buffer;
        audio_len = wav_length;
    }
}

void play_pause_audio(bool play)
{
    if(play) { SDL_PauseAudioDevice(device_id, 0); }

    else { SDL_PauseAudioDevice(device_id, 1); }
}

void init_audio(std::string data_dir) 
{

	std::string output_fname = data_dir + "/d7.wav";

    SDL_Init(SDL_INIT_AUDIO);
    if (SDL_LoadWAV(output_fname.c_str(), &wav_spec, &wav_buffer, &wav_length) == nullptr)
    {
        std::cerr << "Could not open " << output_fname << std::endl;
        exit(1);
    }

    wav_spec.callback = audio_callback;
    wav_spec.userdata = nullptr;
    audio_pos = wav_buffer;
    audio_len = wav_length;

    device_id = SDL_OpenAudioDevice(nullptr, 0, &wav_spec, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (device_id == 0)
    {
        std::cerr << "Sound Device Error: " << SDL_GetError() << std::endl;
        exit(1);
    }
    else { std::cout << "Device ID = " << device_id << std::endl; }
}

void init_graphics(std::string data_dir)
{
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
    std::string crosshair_path = data_dir + "/cursor.png";

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
}

void close_app()
{
    SDL_DestroyWindow(window);
    SDL_CloseAudioDevice(device_id);
    SDL_FreeWAV(wav_buffer);
    SDL_Quit();
}

void run_app(std::string data_dir)
{
    init_graphics(data_dir);
    init_audio(data_dir);

    bool quit = false;
    bool play = true;
    Mouse_Pos mouse;
    SDL_Rect dest;
    SDL_Event event;

    while(!quit)
    {
        SDL_WaitEvent(&event);
        switch(event.type)
        {
            case SDL_QUIT:
                quit = true;
                break; 
            case SDL_MOUSEBUTTONDOWN:
                play_pause_audio(play);
                play = !play;
                break;
        }

        SDL_GetMouseState(&mouse.x, &mouse.y);
        dest.x = mouse.x;
        dest.y = mouse.y;

        SDL_QueryTexture(texture, nullptr, nullptr, &dest.w, &dest.h); 
        dest.x -= dest.w / 2;
        dest.y -= dest.h / 2;

        // clear screen
        SDL_RenderClear(renderer);
        // render texture to screen
        SDL_RenderCopy(renderer, texture, nullptr, &dest); 
        // update screren 
        SDL_RenderPresent(renderer);
    }

    close_app();
}

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

void read_write_wav(std::string data_dir)
{
    int num_frequencies = 4;
	float* frequencies = (float*)malloc(sizeof(float) * num_frequencies);

	frequencies[0] = 293.665;	// D  - Octave 4
	frequencies[1] = 369.994;	// F# - Octave 4
	frequencies[2] = 440.000;	// A  - Octave 4
	frequencies[3] = 523.251;	// C  - Octave 5

	int sample_rate = 44100;
    int duration = 1;
	int num_samples = sample_rate*duration;	

	Sound_Sim harmonic(num_frequencies, frequencies, num_samples, sample_rate);
   	harmonic.simulate_data();

    int max = 32767;
    uint16_t* data = harmonic.get_data(max);

	std::string fname = data_dir + "/d7.wav";

	// WaveIO wave(fname); // automatically parses the data
    WaveIO wave(fname, 1, sample_rate, 16, duration);
    wave.set_data(data);

	wave.write();

	// wave.print_metadata();
    wave.read_out_loud();
}

int main(int argc, char *argv[]) 
{
    std::string data_dir = read_input_fname(argc, argv);
    read_write_wav(data_dir);
    run_app(data_dir);

	return 0;
}
