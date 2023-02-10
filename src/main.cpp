/* Really Advanced WAVE Media Player 
*/ 

#include <cmath>
#include <iostream>
#include <assert.h>
#include <string>

#include "SoundSim.hpp"
#include "WaveIO.hpp"

#ifdef __linux__
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_ttf.h>
#elif _WIN32
    #include <SDL.h>
    #include <SDL_ttf.h>
#endif

#define SCREEN_WIDTH (720)
#define SCREEN_HEIGHT (576)
#define TEXT_DISPLAY_BUFFER (5)

enum class State 
{
    IDLE = 0,
    LOADING_SOUND = 1,
    PLAYING_SOUND = 2,
    SOUND_PAUSED = 3,
    TYPING = 4, 
    QUITTING = 5, 
};

enum class Button
{
    LOAD = 0,
    PLAY = 1, 
    PAUSE = 2,
    TYPE = 3,
    BACKGROUND = 4,
};

typedef struct 
{
    int x, y; 
} Mouse_Pos;

// TODO: get rid of all these globals
SDL_Window* WINDOW = nullptr;
SDL_Renderer* RENDERER = nullptr;
SDL_Surface* SURFACE = nullptr;
SDL_Texture* TEXTURE = nullptr;
TTF_Font* FONT = nullptr;

SDL_AudioSpec WAV_SPEC;
Uint32 WAV_LENGTH;
Uint8* WAV_BUFFER;
SDL_AudioDeviceID DEVICE_ID; 
Uint8* AUDIO_POS;
Uint32 AUDIO_LEN;

std::string WAV_PATH;

SDL_Rect PLAY_RECT; 
SDL_Rect PAUSE_RECT; 
SDL_Rect LOAD_RECT;
SDL_Rect TYPE_RECT;
SDL_Rect TIME_RECT;

Mouse_Pos MOUSE;

State APP_STATE;

void close_app()
{
    SDL_DestroyWindow(WINDOW);
    SDL_FreeSurface(SURFACE);
    SDL_DestroyTexture(TEXTURE);
    SDL_DestroyRenderer(RENDERER);
    SDL_CloseAudioDevice(DEVICE_ID);
    SDL_FreeWAV(WAV_BUFFER);
    SDL_Quit();
}

bool is_on_button(const SDL_Rect &rect)
{
    if (MOUSE.x > rect.x && MOUSE.x < rect.x + rect.w)
    {
        if (MOUSE.y > rect.y && MOUSE.y < rect.y + rect.h) 
        { 
            return true; 
        }
    }
    return false;
}

float sample_num_to_time()
{
   int sample_num = ((AUDIO_POS - WAV_BUFFER));
   return 1.0 / WAV_SPEC.freq * (float)(sample_num / 2);
}

void draw_time_bar()
{
    TIME_RECT.w = 180;
    TIME_RECT.h = 40;
    TIME_RECT.x = 260;
    TIME_RECT.y = 290;

    SDL_Color color = {255, 255, 255};
    float audio_time = sample_num_to_time();

    std::string time = std::to_string(audio_time); 
    SURFACE = TTF_RenderText_Solid(FONT, time.c_str(), color);

    SDL_Rect text_rect;
    if(SURFACE != nullptr) 
    {
        text_rect.w = SURFACE->w;
        text_rect.h = SURFACE->h;
    }
    else 
    {
        text_rect.w = 0;
        text_rect.h = 0;
    }
    
    text_rect.x = TIME_RECT.x + TEXT_DISPLAY_BUFFER;
    text_rect.y = TIME_RECT.y + TEXT_DISPLAY_BUFFER;

    TEXTURE = SDL_CreateTextureFromSurface(RENDERER, SURFACE);
    SDL_FreeSurface(SURFACE);
    SDL_RenderCopy(RENDERER, TEXTURE, NULL, &text_rect);
}

void draw_type_box()
{
    TYPE_RECT.w = 360;
    TYPE_RECT.h = 39;
    TYPE_RECT.x = 90;
    TYPE_RECT.y = 20;

    SDL_SetRenderDrawColor(RENDERER, 255, 255, 255, 255); 
    SDL_RenderDrawRect(RENDERER, &TYPE_RECT);
}

void draw_load_button()
{
    LOAD_RECT.w = 70;
    LOAD_RECT.h = 39;
    LOAD_RECT.x = 20;
    LOAD_RECT.y = 20;

    if (is_on_button(LOAD_RECT))
    {
        SDL_SetRenderDrawColor(RENDERER, 139, 0, 0, 255); 
        SDL_RenderFillRect(RENDERER, &LOAD_RECT);
    }

    SDL_SetRenderDrawColor(RENDERER, 255, 255, 255, 255); 
    SDL_RenderDrawRect(RENDERER, &LOAD_RECT);

    SDL_Color color = {255, 255, 255};
    SURFACE = TTF_RenderText_Solid(FONT, "LOAD", color);

    SDL_Rect text_rect;
    if(SURFACE != nullptr) 
    {
        text_rect.w = SURFACE->w;
        text_rect.h = SURFACE->h;
    }
    else 
    {
        text_rect.w = 0;
        text_rect.h = 0;
    }
    
    text_rect.x = LOAD_RECT.x + TEXT_DISPLAY_BUFFER;
    text_rect.y = LOAD_RECT.y + TEXT_DISPLAY_BUFFER;

    TEXTURE = SDL_CreateTextureFromSurface(RENDERER, SURFACE);
    SDL_FreeSurface(SURFACE);
    SDL_RenderCopy(RENDERER, TEXTURE, NULL, &text_rect);
}

void draw_play_button()
{
    PLAY_RECT.w = SCREEN_WIDTH / 8;  // 90
    PLAY_RECT.h = SCREEN_HEIGHT / 8; // 72
    PLAY_RECT.x = 260;
    PLAY_RECT.y = 216;

    int offset = 20;
    int tri_height = PLAY_RECT.w - offset;
    int tri_base = PLAY_RECT.h - offset;

    if (is_on_button(PLAY_RECT))
    {
        SDL_SetRenderDrawColor(RENDERER, 139, 0, 0, 255); 
        SDL_RenderFillRect(RENDERER, &PLAY_RECT);
    }

    SDL_SetRenderDrawColor(RENDERER, 255, 255, 255, 255); 
    SDL_RenderDrawRect(RENDERER, &PLAY_RECT);

    // draw play triangle
    SDL_RenderDrawLine(RENDERER, PLAY_RECT.x + offset, PLAY_RECT.y + tri_base, 
                        PLAY_RECT.x + offset, PLAY_RECT.y + offset);
    SDL_RenderDrawLine(RENDERER, PLAY_RECT.x + offset, PLAY_RECT.y + offset, 
                        PLAY_RECT.x + tri_height, PLAY_RECT.y + (PLAY_RECT.h/2));
    SDL_RenderDrawLine(RENDERER, PLAY_RECT.x + tri_height, PLAY_RECT.y + (PLAY_RECT.h/2), 
                        PLAY_RECT.x + offset, PLAY_RECT.y + tri_base);
}

void draw_pause_button()
{
    PAUSE_RECT.w = SCREEN_WIDTH / 8; 
    PAUSE_RECT.h = SCREEN_HEIGHT / 8;
    PAUSE_RECT.x = 270 + PAUSE_RECT.w;
    PAUSE_RECT.y = 216;

    int x_offset = 20;
    int y_offset = 30;
    int line_len = PAUSE_RECT.w - x_offset;

    if (is_on_button(PAUSE_RECT))
    {
        SDL_SetRenderDrawColor(RENDERER, 139, 0, 0, 255); 
        SDL_RenderFillRect(RENDERER, &PAUSE_RECT);
    }

    SDL_SetRenderDrawColor(RENDERER, 255, 255, 255, 255); 
    SDL_RenderDrawRect(RENDERER, &PAUSE_RECT);
    SDL_RenderDrawLine(RENDERER, PAUSE_RECT.x + x_offset, PAUSE_RECT.y + y_offset, 
                        PAUSE_RECT.x + line_len, PAUSE_RECT.y + y_offset);
    SDL_RenderDrawLine(RENDERER, PAUSE_RECT.x + x_offset, PAUSE_RECT.y + PAUSE_RECT.h - y_offset, 
                        PAUSE_RECT.x + line_len, PAUSE_RECT.y + PAUSE_RECT.h - y_offset);
}

Button which_button()
{
   Button clicked_button; 

   if (is_on_button(LOAD_RECT)) { clicked_button = Button::LOAD; }
   else if (is_on_button(PLAY_RECT)) { clicked_button = Button::PLAY; }
   else if (is_on_button(PAUSE_RECT)) { clicked_button = Button::PAUSE; }
   else if (is_on_button(TYPE_RECT)) { clicked_button = Button::TYPE; }
   else { clicked_button = Button::BACKGROUND; }

   return clicked_button;
}


void play_audio(bool play)
{
    if(play) { SDL_PauseAudioDevice(DEVICE_ID, 0); }

    else { SDL_PauseAudioDevice(DEVICE_ID, 1); }
}

void audio_callback(void* userdata, Uint8* stream, int len)
{
    memset(stream, 0, len);

    if(AUDIO_LEN == 0) { return; }

    if(len > AUDIO_LEN) 
    { 
        len = AUDIO_LEN; 
    }

    memcpy(stream, AUDIO_POS, len);

    AUDIO_POS += len;
    AUDIO_LEN -= len;

    if(AUDIO_LEN == 0)
    {
        AUDIO_POS = WAV_BUFFER;
        AUDIO_LEN = WAV_LENGTH;
        play_audio(false);
    }
}

bool load_audio(std::string WAV_PATH)
{
    if (SDL_LoadWAV(WAV_PATH.c_str(), &WAV_SPEC, &WAV_BUFFER, &WAV_LENGTH) == nullptr)
    {
        std::cerr << "LoadWAV Error: "<< SDL_GetError() << std::endl;
        return false;
    }

    AUDIO_POS = WAV_BUFFER;
    AUDIO_LEN = WAV_LENGTH;

    WAV_SPEC.callback = audio_callback;
    DEVICE_ID = SDL_OpenAudioDevice(nullptr, 0, &WAV_SPEC, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (DEVICE_ID == 0)
    {
        std::cerr << "Sound Device Error: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

void init_graphics()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        std::cerr << "Init Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    WINDOW = SDL_CreateWindow("WAVE Media Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if(WINDOW == nullptr)
    {
        std::cerr << "Create Window ERROR: " << SDL_GetError() << std::endl;
        exit(1);
    }

    RENDERER = SDL_CreateRenderer(WINDOW, -1, SDL_RENDERER_ACCELERATED);
    if(RENDERER == nullptr)
    {
        std::cerr << "Create Renderer ERROR: " << SDL_GetError() << std::endl;
        exit(1);
    }

    if(TTF_Init() < 0)
    {
        std::cerr << "Error Initializing TTF: " << TTF_GetError() << std::endl;
        exit(1);
    }

    FONT = TTF_OpenFont("../assets/Roboto-Regular.ttf", 24);
    if(!FONT)
    {
        std::cerr << "Error Loading Font: " << TTF_GetError() << std::endl;
        exit(1);
    }

    // clear the screen to dark gray
    SDL_SetRenderDrawColor(RENDERER, 37, 37, 38, 255);
    SDL_RenderClear(RENDERER);

    SDL_ShowCursor(SDL_ENABLE);
}

void process_mouse_event()
{
    Button pressed_button = which_button();

    // for safety, user can only pause sound when playing. 
    if(APP_STATE == State::PLAYING_SOUND)
    { 
        if(pressed_button == Button::PAUSE)
        {
            play_audio(false);
            APP_STATE = State::SOUND_PAUSED;
        }

        return;
    }

    assert(APP_STATE != State::PLAYING_SOUND);
    assert(pressed_button != Button::PAUSE);

    switch(pressed_button)
    {
        case Button::LOAD:
            load_audio(WAV_PATH);
            APP_STATE = State::LOADING_SOUND;
            break;
        case Button::PLAY:
            play_audio(true);
            APP_STATE = State::PLAYING_SOUND;
            break;
        case Button::TYPE:
            WAV_PATH.clear();
            APP_STATE = State::TYPING;
            break;
        case Button::BACKGROUND:
            APP_STATE = State::IDLE;
            break;
    }
}

void process_app_event(const SDL_Event &event)
{
    switch(event.type)
    {
        case SDL_QUIT:
            APP_STATE = State::QUITTING;            
            break; 
        case SDL_MOUSEBUTTONDOWN:
            process_mouse_event();
            break;
        case SDL_TEXTINPUT:
            if(APP_STATE == State::TYPING) { WAV_PATH += event.text.text; }
            break;
        case SDL_KEYDOWN:
            if(event.key.keysym.sym == SDLK_BACKSPACE) 
            { 
                if (WAV_PATH.length() > 0) { WAV_PATH.pop_back(); }
            }
            break;
    }
}

void run_app()
{
    init_graphics();
    load_audio(WAV_PATH);

    APP_STATE = State::IDLE;
    SDL_Event event;
    SDL_Color color = {255, 255, 255};

    while(APP_STATE != State::QUITTING)
    {
        SDL_SetRenderDrawColor(RENDERER, 37, 37, 38, 255); 
        SDL_RenderClear(RENDERER);

        SDL_GetMouseState(&MOUSE.x, &MOUSE.y);
        SDL_StartTextInput();

        if(SDL_PollEvent(&event)) { process_app_event(event); }

        draw_play_button();
        draw_pause_button();
        draw_load_button();
        draw_type_box();
        draw_time_bar();

        SURFACE = TTF_RenderText_Solid(FONT, WAV_PATH.c_str(), color);
        SDL_Rect text_rect;
        if(SURFACE != nullptr) 
        {
            text_rect.w = SURFACE->w;
            text_rect.h = SURFACE->h;
        }
        else 
        {
            text_rect.w = 0;
            text_rect.h = 0;
        }

        text_rect.x = TYPE_RECT.x + TEXT_DISPLAY_BUFFER;
        text_rect.y = TYPE_RECT.y + TEXT_DISPLAY_BUFFER;

        TEXTURE = SDL_CreateTextureFromSurface(RENDERER, SURFACE);
        SDL_FreeSurface(SURFACE);
        SDL_RenderCopy(RENDERER, TEXTURE, NULL, &text_rect);

        SDL_RenderPresent(RENDERER);

    }

    close_app();
}

void write_wav()
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

    WaveIO wave(WAV_PATH, 1, sample_rate, 16, duration);
    wave.set_data(data);

	wave.write();
}

std::string read_input_fname(int argc, char* argv[])
{
    std::string WAV_PATH{};
	if (argc < 2)
	{
		std::cerr << "Please provide a path to the Data directory!" << std::endl;
		exit(1);
	} 
	else { WAV_PATH = argv[1]; }

    return WAV_PATH;
}

int main(int argc, char *argv[]) 
{
    WAV_PATH = read_input_fname(argc, argv);
    run_app();

	return 0;
}
