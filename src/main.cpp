/* Really Advanced WAVE Media Player Application (RAWMPA)*/ 

#include <cmath>
#include <iostream>
#include <assert.h>
#include <string>

#include "audio.hpp"
#include "graphics.hpp"

#ifdef __linux__
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_ttf.h>
#elif _WIN32
    #include <SDL.h>
    #include <SDL_ttf.h>
#endif

#define TEXT_DISPLAY_BUFFER (5)
#define SIGNIFICANT_DIGITS (5)
#define BUTTON_GAP (10)

enum class State 
{
    IDLE = 0,
    PLAYING = 1,
    UNINITIALIZED = 2,
};

enum class Mouse_Signal
{
    LOAD = 0,
    PLAY = 1, 
    PAUSE = 2,
    TYPE = 3,
    UNFOCUS = 4,
    SCRUB = 5,
    MIX = 6,
    NONE = 7,
};

Graphics graphics = {};
Audio audio = {};
State app_state = State::UNINITIALIZED;

SDL_Color white = {255, 255, 255, 255};
SDL_Color dark_red = {139, 0, 0, 255};
SDL_Color dark_gray = {37, 37, 38, 255};

void close_app();

int main(int argc, char *argv[]) 
{
    graphics = graphics_initialize();
    audio = audio_initialize();

    audio.volume = 128;
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
    load_texture = graphics_create_texture(&load_text_rect, "Load", white, 
                                            graphics.large_font, graphics.renderer);
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


    // clear the screen to dark gray
    SDL_SetRenderDrawColor(graphics.renderer, dark_gray.r, dark_gray.g, dark_gray.b, dark_gray.a);
    SDL_RenderClear(graphics.renderer);

    
	if (argc > 2)
	{
		std::cerr << "ERROR: too many arguments provided." << std::endl;
        close_app();
        exit(1);
	} 
	else if (argc == 2)
    { 
        audio.wave_path = argv[1]; 
        signal = Mouse_Signal::LOAD;
    }

    bool quit = false;
    while (!quit)
    {
        SDL_SetRenderDrawColor(graphics.renderer, dark_gray.r, dark_gray.g, dark_gray.b, dark_gray.a); 
        SDL_RenderClear(graphics.renderer);
        SDL_SetRenderDrawColor(graphics.renderer, white.r, white.g, white.b, white.a); 

        SDL_GetMouseState(&mouse.x, &mouse.y);

        if (SDL_PollEvent(&sdl_event)) 
        { 
            switch (sdl_event.type)
            {
                case SDL_QUIT:
                    quit = true;
                    break; 
                case SDL_MOUSEBUTTONDOWN:
                    if (graphics_on_button(&load_rect, &mouse)) 
                    { 
                        signal = Mouse_Signal::LOAD; 
                    }
                    else if (graphics_on_button(&play_rect, &mouse)) 
                    { 
                        signal = Mouse_Signal::PLAY; 
                    }
                    else if (graphics_on_button(&pause_rect, &mouse)) 
                    { 
                        signal = Mouse_Signal::PAUSE; 
                    }
                    else if (graphics_on_button(&text_box_rect, &mouse)) 
                    { 
                        signal = Mouse_Signal::TYPE; 
                    }
                    else if (graphics_on_button(&time_marker_rect, &mouse)) 
                    { 
                        signal = Mouse_Signal::SCRUB; 
                    }
                    else if (graphics_on_button(&volume_marker_rect, &mouse)) 
                    { 
                        signal = Mouse_Signal::MIX; 
                    }
                    else 
                    { 
                        signal = Mouse_Signal::UNFOCUS; 
                    }
                    // process_mouse_event(signal);
                    break;
                case SDL_MOUSEBUTTONUP:
                    if (signal == Mouse_Signal::SCRUB | signal == Mouse_Signal::MIX)
                    {
                        signal = Mouse_Signal::NONE; 
                    }
                    break;
                case SDL_TEXTINPUT:
                    if (signal == Mouse_Signal::TYPE) 
                    { 
                        audio.wave_path += sdl_event.text.text; 
                    }
                    break;
                case SDL_KEYDOWN:
                    if (signal == Mouse_Signal::TYPE)
                    {
                        switch(sdl_event.key.keysym.sym)
                        {
                            case SDLK_BACKSPACE: 
                                if (audio.wave_path.length() > 0) 
                                { 
                                    audio.wave_path.pop_back(); 
                                }
                                break;
                            case SDLK_RETURN:
                                // process_mouse_event(Mouse_Signal::LOAD);
                                signal = Mouse_Signal::LOAD;
                                break;
                        }
                    }
                    break;
            }
        }

        switch (signal)
        {
            case Mouse_Signal::LOAD:
                if (app_state == State::PLAYING)
                {
                    audio_play(false, &audio);
                }
              
                if (audio_load(&audio)) 
                { 
                    total_time = audio_total_time(&audio);
                    app_state = State::IDLE; }
                else 
                { 
                    total_time = 0.0;
                    app_state = State::UNINITIALIZED; 
                }
                signal = Mouse_Signal::NONE;
                break;
            case Mouse_Signal::PLAY:
                if (app_state == State::PLAYING)
                {
                    break;
                }
                else if (app_state != State::UNINITIALIZED)
                { 
                    audio_play(true, &audio);
                    app_state = State::PLAYING;
                }
                signal = Mouse_Signal::NONE;
                break;
            case Mouse_Signal::PAUSE:
                if (app_state == State::PLAYING)
                {
                    audio_play(false, &audio);
                    app_state = State::IDLE;
                }
                signal = Mouse_Signal::NONE;
                break;
            case Mouse_Signal::TYPE:
                // std::cout << animate_cursor() << std::endl;
                if (graphics_blink())
                {
                    SDL_RenderDrawLine(graphics.renderer, cursor.p1.x, cursor.p1.y, cursor.p2.x, cursor.p2.y);
                }
                break;
            case Mouse_Signal::SCRUB:
                if (mouse.x > time_bar_rect.x && 
                    mouse.x < time_bar_rect.x + time_bar_rect.w)
                {
                    float new_width = (float)(mouse.x - filled_time_bar_rect.x);
                    percent_completed = new_width / time_bar_rect.w;
                    float new_time = percent_completed * total_time;
                    audio_update_pos(new_time, &audio);
                } 
                break;
            case Mouse_Signal::MIX:
                if (mouse.y > volume_rect.y && 
                    mouse.y < volume_rect.y + volume_rect.h)
                {
                    float new_height = (float)(volume_rect.h - (mouse.y - volume_rect.y));
                    percent_volume = new_height / volume_rect.h;
                    SDL_LockAudioDevice(audio.device_id);
                    audio.volume = (int)(percent_volume * MAX_VOLUME);
                    SDL_UnlockAudioDevice(audio.device_id);
                } 
                break;
            case Mouse_Signal::UNFOCUS:
                if (app_state != State::PLAYING)
                {
                    app_state = State::IDLE;
                    signal = Mouse_Signal::NONE;
                }
                break;
        }

        // play button
        graphics_draw_button(&play_rect, &mouse, white, dark_gray, dark_red, graphics.renderer);
        graphics_draw_triangle(&play_tri, graphics.renderer);

        // pause button
        graphics_draw_button(&pause_rect, &mouse, white, dark_gray, dark_red, graphics.renderer);
        SDL_RenderDrawLine(graphics.renderer, pause_top.p1.x, pause_top.p1.y, pause_top.p2.x, pause_top.p2.y);
        SDL_RenderDrawLine(graphics.renderer, pause_bot.p1.x, pause_bot.p1.y, pause_bot.p2.x, pause_bot.p2.y);

        // load button 
        graphics_draw_button(&load_rect, &mouse, white, dark_gray, dark_red, graphics.renderer);
        SDL_RenderCopy(graphics.renderer, load_texture, nullptr, &load_text_rect);

        // text box
        graphics_draw_rect(&text_box_rect, &white, nullptr, graphics.renderer);

        // wave path
        if (audio.wave_path != "") 
        { 
            wave_texture = graphics_create_texture(&wave_text_rect, audio.wave_path, 
                                                    white, graphics.large_font, graphics.renderer);
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

            SDL_RenderCopy(graphics.renderer, wave_texture, nullptr, &wave_text_rect);
            SDL_DestroyTexture(wave_texture);
        }

        // time bar
        if (app_state != State::UNINITIALIZED)
        { 
            curr_time = audio_curr_time(&audio);
            percent_completed = curr_time / total_time; 
        }
        else 
        { 
            curr_time = 0.0;
            percent_completed = 0.0; 
        }

        filled_time_bar_rect.w = (int)(percent_completed * time_bar_rect.w);
        graphics_draw_rect(&filled_time_bar_rect, nullptr, &dark_red, graphics.renderer);
        graphics_draw_rect(&time_bar_rect, &white, nullptr, graphics.renderer);

        // current time marker
        time_marker_rect.x = filled_time_bar_rect.x + filled_time_bar_rect.w - (time_marker_rect.w / 2);
        graphics_draw_button(&time_marker_rect, &mouse, white, dark_gray, white, graphics.renderer);

        // current time
        curr_time_str = std::to_string(curr_time).substr(0, SIGNIFICANT_DIGITS); 
        curr_time_texture = graphics_create_texture(&curr_time_text_rect, curr_time_str, 
                                                    white, graphics.small_font, graphics.renderer);
        if (curr_time_texture == nullptr)
        {
            std::cerr << "ERROR: Couldn't create texture for current time (" << __LINE__ << ")" << std::endl;
            close_app();
            exit(1);
        }
        SDL_RenderCopy(graphics.renderer, curr_time_texture, nullptr, &curr_time_text_rect);
        SDL_DestroyTexture(curr_time_texture);

        // total time
        total_time_str = std::to_string(total_time).substr(0, SIGNIFICANT_DIGITS);
        total_time_texture = graphics_create_texture(&total_time_text_rect, total_time_str, 
                                                    white, graphics.small_font, graphics.renderer);
        total_time_text_rect.x = pause_rect.x + pause_rect.w - total_time_text_rect.w;
        if (total_time_texture == nullptr)
        {
            std::cerr << "ERROR: Couldn't create texture for total time (" << __LINE__ << ")" << std::endl;
            close_app();
            exit(1);
        }
        SDL_RenderCopy(graphics.renderer, total_time_texture, nullptr, &total_time_text_rect);
        SDL_DestroyTexture(total_time_texture);

        // volume bar
        percent_volume = (float)audio.volume / (float)MAX_VOLUME;
        filled_volume_rect.h = -1 * (int)(percent_volume * volume_rect.h);
        graphics_draw_rect(&filled_volume_rect, nullptr, &dark_red, graphics.renderer);
        graphics_draw_rect(&volume_rect, &white, nullptr, graphics.renderer);
        volume_marker_rect.y = filled_volume_rect.y + filled_volume_rect.h - (volume_marker_rect.h / 2);
        graphics_draw_button(&volume_marker_rect, &mouse, white, dark_gray, white, graphics.renderer);


        SDL_RenderPresent(graphics.renderer);
    }

    close_app();
    return 0;
}

void audio_callback(void* userdata, Uint8* stream, int len)
{
    memset(stream, 0, len);

    if (audio.current_len == 0) { return; }

    if (len > audio.current_len) { len = audio.current_len; }

    SDL_MixAudioFormat(stream, audio.current_pos, audio.wave_spec.format, len, audio.volume);
    
    audio.current_pos += len;
    audio.current_len -= len;

    if (audio.current_len == 0)
    {
        audio.current_pos = audio.wave_buff;
        audio.current_len = audio.wave_len;

        audio_play(false, &audio);
        app_state = State::IDLE;
    }
}

void close_app()
{
    SDL_DestroyRenderer(graphics.renderer);
    SDL_DestroyWindow(graphics.window);

    TTF_CloseFont(graphics.large_font);
    TTF_CloseFont(graphics.small_font);

    SDL_CloseAudioDevice(audio.device_id);
    SDL_FreeWAV(audio.wave_buff);

    TTF_Quit();
    SDL_Quit();
}


