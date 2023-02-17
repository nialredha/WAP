/* Really Advanced WAVE Media Player Application (RAWMPA)*/ 

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
#define MAX_LENGTH (3)
#define SIGNIFICANT_DIGITS (5)
#define BUTTON_GAP (10)
#define MAX_VOLUME (128)

enum class State 
{
    IDLE = 0,
    PLAYING = 1,
    TYPING = 2, 
};

enum class Mouse_Signal
{
    CLICKED_LOAD = 0,
    CLICKED_PLAY = 1, 
    CLICKED_PAUSE = 2,
    CLICKED_TEXT_BOX = 3,
    CLICKED_BACKGROUND = 4,
    CLICKED_TIME_MARKER = 5,
    CLICKED_VOLUME_MARKER = 6,
};

typedef struct 
{
    int x, y; 
} Position;

typedef struct
{
    Position p1;
    Position p2;
    int length;
} Line;

typedef struct 
{
    Position p1;
    Position p2;
    Position p3;
    int base;
    int height;
} Triangle;

// TODO: get rid of all these globals
SDL_Window* WINDOW = nullptr;
SDL_Renderer* RENDERER = nullptr;
TTF_Font* LARGE_FONT = nullptr;
TTF_Font* SMALL_FONT = nullptr;

SDL_Color WHITE = {255, 255, 255, 255};
SDL_Color DARK_RED = {139, 0, 0, 255};
SDL_Color DARK_GRAY = {37, 37, 38, 255};

SDL_AudioDeviceID DEVICE_ID = 0; 
SDL_AudioSpec WAVE_SPEC;
Uint32 WAVE_LENGTH;
Uint32 AUDIO_LEN;
Uint8* WAVE_BUFFER;
Uint8* AUDIO_POS;

State APP_STATE;
std::string WAVE_PATH;
int  VOLUME;
bool SOUND_LOADED = false;
bool TIME_MARKER_SELECTED = false;
bool VOLUME_MARKER_SELECTED = false;

void close_app()
{
    SDL_DestroyRenderer(RENDERER);
    SDL_DestroyWindow(WINDOW);

    TTF_CloseFont(LARGE_FONT);
    TTF_CloseFont(SMALL_FONT);

    SDL_CloseAudioDevice(DEVICE_ID);
    SDL_FreeWAV(WAVE_BUFFER);

    TTF_Quit();
    SDL_Quit();
}

void animate_cursor()
{
}

bool on_button(const SDL_Rect* rect, const Position* mouse)
{
    if (mouse->x > rect->x && mouse->x < rect->x + rect->w)
    {
        if (mouse->y > rect->y && mouse->y < rect->y + rect->h) 
        { 
            return true; 
        }
    }
    return false;
}

void draw_triangle(Triangle* tri, SDL_Renderer* rend)
{
    SDL_RenderDrawLine(rend, tri->p1.x, tri->p1.y, tri->p2.x, tri->p2.y);
    SDL_RenderDrawLine(rend, tri->p2.x, tri->p2.y, tri->p3.x, tri->p3.y);
    SDL_RenderDrawLine(rend, tri->p3.x, tri->p3.y, tri->p1.x, tri->p1.y);
}

void draw_button(SDL_Rect* rect, const Position* mouse, 
                    SDL_Color outline, SDL_Color fill, SDL_Color hover_fill, 
                    SDL_Renderer* rend)
{
    if (on_button(rect, mouse))
    {
        SDL_SetRenderDrawColor(rend, hover_fill.r, hover_fill.g, hover_fill.b, fill.a); 
        SDL_RenderFillRect(rend, rect);
    }
    else
    {
        SDL_SetRenderDrawColor(rend, fill.r, fill.g, fill.b, fill.a); 
        SDL_RenderFillRect(rend, rect);
    }

    SDL_SetRenderDrawColor(rend, outline.r, outline.g, outline.b, outline.a); 
    SDL_RenderDrawRect(rend, rect);
}

void draw_rect(SDL_Rect* rect, SDL_Color* outline, SDL_Color* fill, SDL_Renderer* rend)
{
    if (fill != nullptr)
    {
        SDL_SetRenderDrawColor(rend, fill->r, fill->g, fill->b, fill->a); 
        SDL_RenderFillRect(rend, rect);
    }
    if (outline != nullptr)
    {
        SDL_SetRenderDrawColor(rend, outline->r, outline->g, outline->b, outline->a); 
        SDL_RenderDrawRect(rend, rect);
    }
}

SDL_Texture* create_texture(SDL_Rect* rect, std::string text, SDL_Color color, 
                                        TTF_Font* font, SDL_Renderer* rend)
{
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);

    if (surface != nullptr) 
    {
        rect->w = surface->w;
        rect->h = surface->h;
    }
    else 
    {
        rect->w = 0;
        rect->h = 0;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(RENDERER, surface);
    SDL_FreeSurface(surface);

    return texture;
}

float get_curr_time()
{
    SDL_LockAudioDevice(DEVICE_ID);

    int bytes_per_sample = (int)SDL_AUDIO_BITSIZE(WAVE_SPEC.format) / 8;
    float samples_per_byte = 0;
    if (bytes_per_sample != 0)
    { 
        samples_per_byte = 1.0 / (float)bytes_per_sample;
    }
    float sample_num = ((AUDIO_POS - WAVE_BUFFER) * samples_per_byte) / WAVE_SPEC.channels; 

    float curr_time = 0.0;
    if (WAVE_SPEC.freq != 0)
    {
        curr_time = 1.0 / (float)WAVE_SPEC.freq * sample_num;
    }
    
    SDL_UnlockAudioDevice(DEVICE_ID);

    return curr_time;
}

void update_audio_pos(float time)
{
    // AUDIO_POS = (curr_time * freq) * chan / samples_per_byte + WAVE_BUFFER

    SDL_LockAudioDevice(DEVICE_ID);

    int bytes_per_sample = (int)SDL_AUDIO_BITSIZE(WAVE_SPEC.format) / 8;
    float samples_per_byte = 0;
    if (bytes_per_sample != 0)
    { 
        samples_per_byte = 1.0 / (float)bytes_per_sample;
    }

    int sample_num = (int)(time * (float)WAVE_SPEC.freq);
    int total_bytes = sample_num * WAVE_SPEC.channels * bytes_per_sample;

    if (total_bytes > (int)WAVE_LENGTH) { total_bytes = (int)WAVE_LENGTH; }

    AUDIO_POS = WAVE_BUFFER + total_bytes;
    AUDIO_LEN = WAVE_LENGTH - total_bytes;

    SDL_UnlockAudioDevice(DEVICE_ID);
}

float get_total_time()
{
    int bytes_per_sample = (int)SDL_AUDIO_BITSIZE(WAVE_SPEC.format) / 8;
    float samples_per_byte = 0;
    if (bytes_per_sample != 0)
    { 
        samples_per_byte = 1.0 / (float)bytes_per_sample;
    }
    float num_samples = (WAVE_LENGTH * samples_per_byte) / WAVE_SPEC.channels; 

    float total_time = 0.0;
    if (WAVE_SPEC.freq != 0)
    {
        total_time = 1.0 / (float)WAVE_SPEC.freq * (float)num_samples;
    }

    return total_time;
}

void play_audio(bool play)
{
    if (play) { SDL_PauseAudioDevice(DEVICE_ID, 0); }

    else { SDL_PauseAudioDevice(DEVICE_ID, 1); }
}

void audio_callback(void* userdata, Uint8* stream, int len)
{
    memset(stream, 0, len);

    if (AUDIO_LEN == 0) { return; }

    if (len > AUDIO_LEN) 
    { 
        len = AUDIO_LEN; 
    }

    SDL_MixAudioFormat(stream, AUDIO_POS, WAVE_SPEC.format, len, VOLUME);
    // memcpy(stream, AUDIO_POS, len);
    
    AUDIO_POS += len;
    AUDIO_LEN -= len;

    if (AUDIO_LEN == 0)
    {
        AUDIO_POS = WAVE_BUFFER;
        AUDIO_LEN = WAVE_LENGTH;

        play_audio(false);
        APP_STATE = State::IDLE;
    }
}

bool load_audio()
{
    if (WAVE_BUFFER != nullptr)
    {
        SDL_CloseAudioDevice(DEVICE_ID);
        SDL_FreeWAV(WAVE_BUFFER);
    }

    if (SDL_LoadWAV(WAVE_PATH.c_str(), &WAVE_SPEC, &WAVE_BUFFER, &WAVE_LENGTH) == nullptr)
    {
        std::cerr << "LoadWAV Error: "<< SDL_GetError() << std::endl;
        WAVE_BUFFER = nullptr;
        return false;
    }

    AUDIO_POS = WAVE_BUFFER;
    AUDIO_LEN = WAVE_LENGTH;

    WAVE_SPEC.callback = audio_callback;
    WAVE_SPEC.userdata = nullptr;
    WAVE_SPEC.size = 1024;
    DEVICE_ID = SDL_OpenAudioDevice(nullptr, 0, &WAVE_SPEC, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (DEVICE_ID == 0)
    {
        std::cerr << "Sound Device Error: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

void initialize()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        std::cerr << "Init Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    WINDOW = SDL_CreateWindow("WAVE Media Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (WINDOW == nullptr)
    {
        std::cerr << "Create Window ERROR: " << SDL_GetError() << std::endl;
        exit(1);
    }

    RENDERER = SDL_CreateRenderer(WINDOW, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (RENDERER == nullptr)
    {
        std::cerr << "Create Renderer ERROR: " << SDL_GetError() << std::endl;
        exit(1);
    }

    if (TTF_Init() < 0)
    {
        std::cerr << "Error Initializing TTF: " << TTF_GetError() << std::endl;
        exit(1);
    }

    LARGE_FONT = TTF_OpenFont("../assets/Roboto-Regular.ttf", 24);
    if (!LARGE_FONT)
    {
        std::cerr << "Error Loading Font: " << TTF_GetError() << std::endl;
        exit(1);
    }
    SMALL_FONT = TTF_OpenFont("../assets/Roboto-Regular.ttf", 12);
    if (!SMALL_FONT)
    {
        std::cerr << "Error Loading Font: " << TTF_GetError() << std::endl;
        exit(1);
    }

    // clear the screen to dark gray
    SDL_SetRenderDrawColor(RENDERER, DARK_GRAY.r, DARK_GRAY.g, DARK_GRAY.b, DARK_GRAY.a);
    SDL_RenderClear(RENDERER);

    SDL_ShowCursor(SDL_ENABLE);
}

void process_mouse_event(const Mouse_Signal signal)
{
    // for safety, user can only pause sound when playing. 
    if (APP_STATE == State::PLAYING)
    { 
        if (signal == Mouse_Signal::CLICKED_PAUSE)
        {
            play_audio(false);
            APP_STATE = State::IDLE;
        }
        if (signal == Mouse_Signal::CLICKED_VOLUME_MARKER)
        {
            VOLUME_MARKER_SELECTED = true;
        }
        return;
    }

    assert(APP_STATE != State::PLAYING);
    switch (signal)
    {
        case Mouse_Signal::CLICKED_LOAD:
            if (load_audio()) { SOUND_LOADED = true; }
            else { SOUND_LOADED = false; }
            break;
        case Mouse_Signal::CLICKED_PLAY:
            if (SOUND_LOADED)
            { 
                play_audio(true);
                APP_STATE = State::PLAYING;
            }
            break;
        case Mouse_Signal::CLICKED_TEXT_BOX:
            APP_STATE = State::TYPING;
            break;
        case Mouse_Signal::CLICKED_TIME_MARKER:
            TIME_MARKER_SELECTED = true;
            break;
        case Mouse_Signal::CLICKED_VOLUME_MARKER:
            VOLUME_MARKER_SELECTED = true;
            break;
        case Mouse_Signal::CLICKED_BACKGROUND:
            APP_STATE = State::IDLE;
            break;
    }
}

int main(int argc, char *argv[]) 
{
    initialize();

	if (argc > 2)
	{
		std::cerr << "ERROR: too many arguments provided." << std::endl;
        close_app();
        exit(1);
	} 
	else if (argc == 2)
    { 
        WAVE_PATH = argv[1]; 
        if (load_audio())
        {
            SOUND_LOADED = true;
        }
    }

    VOLUME = 128;
    APP_STATE = State::IDLE;
    SDL_Event sdl_event;

    SDL_Texture* load_texture = nullptr;
    SDL_Texture* curr_time_texture = nullptr;
    SDL_Texture* total_time_texture = nullptr;
    SDL_Texture* wave_texture = nullptr;

    Position mouse;
    Mouse_Signal signal; 

    SDL_Rect play_rect; 
    play_rect.w = SCREEN_WIDTH / 8;
    play_rect.h = SCREEN_HEIGHT / 8; 
    play_rect.x = (SCREEN_WIDTH / 2) - play_rect.w - (BUTTON_GAP / 2);
    play_rect.y = (SCREEN_HEIGHT / 2) - (play_rect.h / 2);
    int offset = 20;
    Triangle play_tri;
    play_tri.height = play_rect.w - offset;
    play_tri.base = play_rect.h - offset;
    play_tri.p1.x = play_rect.x + offset;
    play_tri.p1.y = play_rect.y + play_tri.base;
    play_tri.p2.x = play_rect.x + offset;
    play_tri.p2.y = play_rect.y + offset;
    play_tri.p3.x = play_rect.x + play_tri.height;
    play_tri.p3.y = play_rect.y + (play_rect.h/2);

    SDL_Rect pause_rect; 
    pause_rect.w = SCREEN_WIDTH / 8; 
    pause_rect.h = SCREEN_HEIGHT / 8;
    pause_rect.x = (SCREEN_WIDTH / 2) + (BUTTON_GAP / 2);
    pause_rect.y = (SCREEN_HEIGHT / 2) - (play_rect.h / 2);
    int x_offset = 20;
    int y_offset = 30;
    Line pause_top;
    pause_top.length = pause_rect.w - x_offset;
    pause_top.p1.x = pause_rect.x + x_offset;
    pause_top.p1.y = pause_rect.y + y_offset;
    pause_top.p2.x = pause_rect.x + pause_top.length;
    pause_top.p2.y = pause_rect.y + y_offset;
    Line pause_bot;
    pause_bot.length = pause_top.length;
    pause_bot.p1.x = pause_rect.x + x_offset;
    pause_bot.p1.y = pause_rect.y + pause_rect.h - y_offset;
    pause_bot.p2.x = pause_rect.x + pause_bot.length;
    pause_bot.p2.y = pause_rect.y + pause_rect.h - y_offset;

    SDL_Rect load_text_rect;
    load_text_rect.x = BUTTON_GAP;
    load_text_rect.y = BUTTON_GAP;
    SDL_Rect load_rect;
    load_rect.x = load_text_rect.x - TEXT_DISPLAY_BUFFER;
    load_rect.y = load_text_rect.y - TEXT_DISPLAY_BUFFER;
    load_texture = create_texture(&load_text_rect, "Load", WHITE, LARGE_FONT, RENDERER);
    if (load_texture == nullptr)
    {
        std::cerr << "ERROR: Couldn't create texture for load (" << __LINE__ << ")" << std::endl;
        close_app();
        exit(1);
    }
    load_rect.w = load_text_rect.w + TEXT_DISPLAY_BUFFER * 2;
    load_rect.h = load_text_rect.h + TEXT_DISPLAY_BUFFER * 2;

    SDL_Rect text_box_rect;
    text_box_rect.w = 360;
    text_box_rect.h = load_rect.h;
    text_box_rect.x = load_rect.x + load_rect.w;
    text_box_rect.y = load_rect.y;

    SDL_Rect wave_text_rect;
    wave_text_rect.x = text_box_rect.x + TEXT_DISPLAY_BUFFER;
    wave_text_rect.y = text_box_rect.y + TEXT_DISPLAY_BUFFER;

    Line cursor;
    cursor.length = text_box_rect.h - TEXT_DISPLAY_BUFFER * 2;
    cursor.p1.x = wave_text_rect.x + TEXT_DISPLAY_BUFFER;
    cursor.p1.y = wave_text_rect.y;
    cursor.p2.x = cursor.p1.x;
    cursor.p2.y = wave_text_rect.y + cursor.length;

    SDL_Rect time_bar_rect;
    time_bar_rect.w = play_rect.w + pause_rect.w + BUTTON_GAP;
    time_bar_rect.h = 10;
    time_bar_rect.x = play_rect.x;
    time_bar_rect.y = play_rect.y + play_rect.h + BUTTON_GAP;
    float percent_completed = 0.0; 
    SDL_Rect filled_time_bar_rect;
    filled_time_bar_rect.w = 0;
    filled_time_bar_rect.h = time_bar_rect.h;
    filled_time_bar_rect.x = time_bar_rect.x;
    filled_time_bar_rect.y = time_bar_rect.y;

    SDL_Rect time_marker_rect;
    time_marker_rect.w = filled_time_bar_rect.h;
    time_marker_rect.h = filled_time_bar_rect.h;
    time_marker_rect.x = 0;
    time_marker_rect.y = filled_time_bar_rect.y;

    SDL_Rect curr_time_text_rect;
    float curr_time;
    std::string curr_time_str;
    curr_time_text_rect.x = play_rect.x;
    curr_time_text_rect.y = time_bar_rect.y + time_bar_rect.h;

    float total_time;
    std::string total_time_str;
    SDL_Rect total_time_text_rect;
    total_time_text_rect.y = time_bar_rect.y + time_bar_rect.h;


    SDL_Rect volume_rect;
    volume_rect.x = pause_rect.x + pause_rect.w + BUTTON_GAP;
    volume_rect.y = pause_rect.y;
    volume_rect.w = time_bar_rect.h;
    volume_rect.h = pause_rect.h + BUTTON_GAP + time_bar_rect.h;
    SDL_Rect filled_volume_rect;
    filled_volume_rect.w = volume_rect.w;
    filled_volume_rect.h = 0;
    filled_volume_rect.x = volume_rect.x;
    filled_volume_rect.y = volume_rect.y + volume_rect.h;
    SDL_Rect volume_marker_rect;
    volume_marker_rect.w = filled_volume_rect.w;
    volume_marker_rect.h = filled_volume_rect.w;
    volume_marker_rect.x = filled_volume_rect.x;
    volume_marker_rect.y = filled_volume_rect.y;
    float percent_volume = 0.0;

    bool quit = false;
    while (!quit)
    {
        SDL_SetRenderDrawColor(RENDERER, DARK_GRAY.r, DARK_GRAY.g, DARK_GRAY.b, DARK_GRAY.a); 
        SDL_RenderClear(RENDERER);

        SDL_GetMouseState(&mouse.x, &mouse.y);
        SDL_StartTextInput();

        if (SDL_PollEvent(&sdl_event)) 
        { 
            switch (sdl_event.type)
            {
                case SDL_QUIT:
                    quit = true;
                    break; 
                case SDL_MOUSEBUTTONDOWN:
                    if (on_button(&load_rect, &mouse)) { signal = Mouse_Signal::CLICKED_LOAD; }
                    else if (on_button(&play_rect, &mouse)) { signal = Mouse_Signal::CLICKED_PLAY; }
                    else if (on_button(&pause_rect, &mouse)) { signal = Mouse_Signal::CLICKED_PAUSE; }
                    else if (on_button(&text_box_rect, &mouse)) { signal = Mouse_Signal::CLICKED_TEXT_BOX; }
                    else if (on_button(&time_marker_rect, &mouse)) 
                    { 
                        signal = Mouse_Signal::CLICKED_TIME_MARKER; 
                    }
                    else if (on_button(&volume_marker_rect, &mouse)) 
                    { 
                        signal = Mouse_Signal::CLICKED_VOLUME_MARKER; 
                    }
                    else { signal = Mouse_Signal::CLICKED_BACKGROUND; }
                    process_mouse_event(signal);
                    break;
                case SDL_MOUSEBUTTONUP:
                    if (TIME_MARKER_SELECTED) { TIME_MARKER_SELECTED = false; }
                    if (VOLUME_MARKER_SELECTED) { VOLUME_MARKER_SELECTED = false; }
                    break;
                case SDL_TEXTINPUT:
                    if (APP_STATE == State::TYPING) { WAVE_PATH += sdl_event.text.text; }
                    break;
                case SDL_KEYDOWN:
                    if (sdl_event.key.keysym.sym == SDLK_BACKSPACE) 
                    { 
                        if (WAVE_PATH.length() > 0) { WAVE_PATH.pop_back(); }
                    }
                    if (sdl_event.key.keysym.sym == SDLK_RETURN) 
                    { 
                        process_mouse_event(Mouse_Signal::CLICKED_LOAD);
                    }
                    break;
            }
        }

        // play button
        draw_button(&play_rect, &mouse, WHITE, DARK_GRAY, DARK_RED, RENDERER);
        draw_triangle(&play_tri, RENDERER);

        // pause button
        draw_button(&pause_rect, &mouse, WHITE, DARK_GRAY, DARK_RED, RENDERER);
        SDL_RenderDrawLine(RENDERER, pause_top.p1.x, pause_top.p1.y, pause_top.p2.x, pause_top.p2.y);
        SDL_RenderDrawLine(RENDERER, pause_bot.p1.x, pause_bot.p1.y, pause_bot.p2.x, pause_bot.p2.y);

        // load button 
        draw_button(&load_rect, &mouse, WHITE, DARK_GRAY, DARK_RED, RENDERER);
        SDL_RenderCopy(RENDERER, load_texture, nullptr, &load_text_rect);

        // text box
        draw_rect(&text_box_rect, &WHITE, nullptr, RENDERER);

        // wave path
        if (WAVE_PATH != "") 
        { 
            wave_texture = create_texture(&wave_text_rect, WAVE_PATH, WHITE, LARGE_FONT, RENDERER);
            if (wave_texture == nullptr)
            {
                std::cerr << "ERROR: Couldn't create texture for wave path(" << __LINE__ << ")" << std::endl;
                close_app();
                exit(1);
            }

            cursor.length = wave_text_rect.h;
            cursor.p1.x = wave_text_rect.x + wave_text_rect.w + TEXT_DISPLAY_BUFFER;
            cursor.p1.y = wave_text_rect.y;
            cursor.p2.x = cursor.p1.x;
            cursor.p2.y = wave_text_rect.y + wave_text_rect.h;

            SDL_RenderCopy(RENDERER, wave_texture, nullptr, &wave_text_rect);
            SDL_DestroyTexture(wave_texture);
        }

        if (APP_STATE == State::TYPING) 
        {
            SDL_RenderDrawLine(RENDERER, cursor.p1.x, cursor.p1.y, cursor.p2.x, cursor.p2.y);
        }

        if (TIME_MARKER_SELECTED)
        {
            if (mouse.x > time_bar_rect.x && 
                mouse.x < time_bar_rect.x + time_bar_rect.w)
            {
                float new_width = (float)(mouse.x - filled_time_bar_rect.x);
                percent_completed = new_width / time_bar_rect.w;
                float new_time = percent_completed * total_time;
                update_audio_pos(new_time);
            } 
        }

        // time bar
        if (SOUND_LOADED)
        { 
            total_time = get_total_time();
            curr_time = get_curr_time();
            percent_completed = curr_time / total_time; 
        }
        else 
        { 
            curr_time = 0.0;
            total_time = 0.0;
            percent_completed = 0.0; 
        }
        filled_time_bar_rect.w = (int)(percent_completed * time_bar_rect.w);
        draw_rect(&filled_time_bar_rect, nullptr, &DARK_RED, RENDERER);
        draw_rect(&time_bar_rect, &WHITE, nullptr, RENDERER);

        // current time marker
        time_marker_rect.x = filled_time_bar_rect.x + filled_time_bar_rect.w - (time_marker_rect.w / 2);
        draw_button(&time_marker_rect, &mouse, WHITE, DARK_GRAY, WHITE, RENDERER);

        // current time
        curr_time_str = std::to_string(curr_time).substr(0, SIGNIFICANT_DIGITS); 
        curr_time_texture = create_texture(&curr_time_text_rect, curr_time_str, WHITE, SMALL_FONT, RENDERER);
        if (curr_time_texture == nullptr)
        {
            std::cerr << "ERROR: Couldn't create texture for current time (" << __LINE__ << ")" << std::endl;
            close_app();
            exit(1);
        }
        SDL_RenderCopy(RENDERER, curr_time_texture, nullptr, &curr_time_text_rect);
        SDL_DestroyTexture(curr_time_texture);

        // total time
        total_time_str = std::to_string(total_time).substr(0, SIGNIFICANT_DIGITS);
        total_time_texture = create_texture(&total_time_text_rect, total_time_str, WHITE, SMALL_FONT, RENDERER);
        total_time_text_rect.x = pause_rect.x + pause_rect.w - total_time_text_rect.w;
        if (total_time_texture == nullptr)
        {
            std::cerr << "ERROR: Couldn't create texture for total time (" << __LINE__ << ")" << std::endl;
            close_app();
            exit(1);
        }
        SDL_RenderCopy(RENDERER, total_time_texture, nullptr, &total_time_text_rect);
        SDL_DestroyTexture(total_time_texture);

        // volume bar
        if (VOLUME_MARKER_SELECTED)
        {
            if (mouse.y > volume_rect.y && 
                mouse.y < volume_rect.y + volume_rect.h)
            {
                float new_height = (float)(volume_rect.h - (mouse.y - volume_rect.y));
                percent_volume = new_height / volume_rect.h;

                SDL_LockAudioDevice(DEVICE_ID);
                VOLUME = (int)(percent_volume * MAX_VOLUME);
                SDL_UnlockAudioDevice(DEVICE_ID);
            } 
        }
        percent_volume = (float)VOLUME / (float)MAX_VOLUME;
        filled_volume_rect.h = -1 * (int)(percent_volume * volume_rect.h);
        draw_rect(&filled_volume_rect, nullptr, &DARK_RED, RENDERER);
        draw_rect(&volume_rect, &WHITE, nullptr, RENDERER);
        volume_marker_rect.y = filled_volume_rect.y + filled_volume_rect.h - (volume_marker_rect.h / 2);
        draw_button(&volume_marker_rect, &mouse, WHITE, DARK_GRAY, WHITE, RENDERER);


        SDL_RenderPresent(RENDERER);
    }

    close_app();
    return 0;
}

