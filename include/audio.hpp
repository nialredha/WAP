#ifdef __linux__
    #include <SDL2/SDL.h>
#elif _WIN32
    #include <SDL.h>
#endif

#define MAX_VOLUME (128)

typedef struct 
{
    std::string wave_path;
    SDL_AudioSpec wave_spec;
    Uint32 wave_len;
    Uint8* wave_buff;
    Uint32 current_len;
    Uint8* current_pos;
    int  volume;
    SDL_AudioDeviceID device_id; 
} Audio;

Audio audio_initialize();

bool audio_load(Audio *a);

void audio_callback(void* userdata, Uint8* stream, int len);

void audio_play(bool play, Audio *a);

void audio_update_pos(float time, Audio *a);

float audio_curr_time(Audio *a);

float audio_total_time(Audio *a);

bool audio_at_end(Audio *a);
