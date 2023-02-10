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

#define SCREEN_WIDTH (720)
#define SCREEN_HEIGHT (576)

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

SDL_Rect play_rect; 
SDL_Rect pause_rect; 

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

    /*
    if(audio_len == 0)
    {
        audio_pos = wav_buffer;
        audio_len = wav_length;
    }
    */
}

void play_audio(bool play)
{
    if(play) { SDL_PauseAudioDevice(device_id, 0); }

    else { SDL_PauseAudioDevice(device_id, 1); }
}

void init_audio(std::string wav_path) 
{

    SDL_Init(SDL_INIT_AUDIO);
    if (SDL_LoadWAV(wav_path.c_str(), &wav_spec, &wav_buffer, &wav_length) == nullptr)
    {
        std::cerr << "Could not open " << wav_path << std::endl;
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
}

void init_graphics()
{
    // INITIALIZATION
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Init Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    window = SDL_CreateWindow("WAVE Media Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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

    // clear the screen to dark gray
    SDL_SetRenderDrawColor(renderer, 37, 37, 38, 255);
    SDL_RenderClear(renderer);

    SDL_ShowCursor(SDL_ENABLE);

}

void close_app()
{
    SDL_DestroyWindow(window);
    SDL_CloseAudioDevice(device_id);
    SDL_FreeWAV(wav_buffer);
    SDL_Quit();
}

bool is_on_play(const Mouse_Pos &mouse)
{
    if (mouse.x > play_rect.x && mouse.x < play_rect.x + play_rect.w)
    {
        if (mouse.y > play_rect.y && mouse.y < play_rect.y + play_rect.h) 
        { 
            return true; 
        }
    }
    return false;
}

void draw_play_button(const Mouse_Pos &mouse)
{
    play_rect.w = SCREEN_WIDTH / 8;  // 90
    play_rect.h = SCREEN_HEIGHT / 8; // 72
    play_rect.x = 260;
    play_rect.y = 216;

    int offset = 20;
    int tri_height = play_rect.w - offset;
    int tri_base = play_rect.h - offset;

    if (is_on_play(mouse))
    {
        SDL_SetRenderDrawColor(renderer, 139, 0, 0, 255); 
        SDL_RenderFillRect(renderer, &play_rect);
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); 
    SDL_RenderDrawRect(renderer, &play_rect);
    SDL_RenderDrawLine(renderer, play_rect.x + offset, play_rect.y + tri_base, 
                        play_rect.x + offset, play_rect.y + offset);
    SDL_RenderDrawLine(renderer, play_rect.x + offset, play_rect.y + offset, 
                        play_rect.x + tri_height, play_rect.y + (play_rect.h/2));
    SDL_RenderDrawLine(renderer, play_rect.x + tri_height, play_rect.y + (play_rect.h/2), 
                        play_rect.x + offset, play_rect.y + tri_base);
}

bool is_on_pause(const Mouse_Pos &mouse)
{
    if (mouse.x > pause_rect.x && mouse.x < pause_rect.x + pause_rect.w)
    {
        if (mouse.y > play_rect.y && mouse.y < pause_rect.y + pause_rect.h) 
        { 
            return true; 
        }
    }
    return false;
}

void draw_pause_button(const Mouse_Pos &mouse)
{
    pause_rect.w = SCREEN_WIDTH / 8; 
    pause_rect.h = SCREEN_HEIGHT / 8;
    pause_rect.x = 270 + pause_rect.w;
    pause_rect.y = 216;

    int x_offset = 20;
    int y_offset = 30;
    int line_len = pause_rect.w - x_offset;

    if (is_on_pause(mouse))
    {
        SDL_SetRenderDrawColor(renderer, 139, 0, 0, 255); 
        SDL_RenderFillRect(renderer, &pause_rect);
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); 
    SDL_RenderDrawRect(renderer, &pause_rect);
    SDL_RenderDrawLine(renderer, pause_rect.x + x_offset, pause_rect.y + y_offset, 
                        pause_rect.x + line_len, pause_rect.y + y_offset);
    SDL_RenderDrawLine(renderer, pause_rect.x + x_offset, pause_rect.y + pause_rect.h - y_offset, 
                        pause_rect.x + line_len, pause_rect.y + pause_rect.h - y_offset);
}

void run_app(std::string wav_path)
{
    init_graphics();
    init_audio(wav_path);

    bool quit = false;
    Mouse_Pos mouse;
    SDL_Event event;

    while(!quit)
    {
        SDL_SetRenderDrawColor(renderer, 37, 37, 38, 255); 
        SDL_RenderClear(renderer);

        SDL_GetMouseState(&mouse.x, &mouse.y);

        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    quit = true;
                    break; 
                case SDL_MOUSEBUTTONDOWN:
                    if(is_on_play(mouse)) { play_audio(true); }
                    else if(is_on_pause(mouse)) { play_audio(false); }
                    break;
            }
        }

        draw_play_button(mouse);
        draw_pause_button(mouse);

        SDL_RenderPresent(renderer);
    }

    close_app();
}

std::string read_input_fname(int argc, char* argv[])
{
    std::string wav_path{};
	if (argc < 2)
	{
		std::cerr << "Please provide a path to the Data directory!" << std::endl;
		exit(1);
	} 
	else { wav_path = argv[1]; }

    return wav_path;
}

void write_wav(std::string wav_path)
{
    int num_frequencies = 4;
	float* frequencies = (float*)malloc(sizeof(float) * num_frequencies);

	frequencies[0] = 293.665;	// D  - Octave 4
	frequencies[1] = 369.994;	// F# - Octave 4
	frequencies[2] = 440.000;	// A  - Octave 4
	frequencies[3] = 523.251;	// C  - Octave 5

	int sample_rate = 44100;
    int duration = 10;
	int num_samples = sample_rate*duration;	

	Sound_Sim harmonic(num_frequencies, frequencies, num_samples, sample_rate);
   	harmonic.simulate_data();

    int max = 32767;
    uint16_t* data = harmonic.get_data(max);

    WaveIO wave(wav_path, 1, sample_rate, 16, duration);
    wave.set_data(data);

	wave.write();
}

int main(int argc, char *argv[]) 
{
    std::string wav_path = read_input_fname(argc, argv);
    // write_wav(wav_path);
   
    run_app(wav_path);

	return 0;
}
