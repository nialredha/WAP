#include <iostream>
#include <string>
#include "audio.hpp"

Audio audio_initialize()
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        std::cerr << "Init Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    Audio a = {};

    return a;
}

bool audio_load(Audio *a)
{
    if (a->wave_buff != nullptr)
    {
        SDL_CloseAudioDevice(a->device_id);
        SDL_FreeWAV(a->wave_buff);
    }

    if (SDL_LoadWAV(a->wave_path.c_str(), &a->wave_spec, &a->wave_buff, &a->wave_len) == nullptr)
    {
        std::cerr << "LoadWAV Error: "<< SDL_GetError() << std::endl;
        a->wave_buff = nullptr;
        return false;
    }

    a->current_pos = a->wave_buff;
    a->current_len = a->wave_len;

    a->wave_spec.callback = audio_callback;
    a->wave_spec.userdata = nullptr;
    // a->wave_spec.size = 1024;
    a->device_id = SDL_OpenAudioDevice(nullptr, 0, &a->wave_spec, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (a->device_id == 0)
    {
        std::cerr << "Sound Device Error: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

void audio_play(bool play, Audio *a)
{
    if (play) { SDL_PauseAudioDevice(a->device_id, 0); }

    else { SDL_PauseAudioDevice(a->device_id, 1); }
}

void audio_update_pos(float time, Audio *a)
{
    // a->current_pos = (curr_time * freq) * chan / samples_per_byte + a->wave_buff

    SDL_LockAudioDevice(a->device_id);

    if (a->wave_buff == nullptr)
    {
        std::cerr << "Audio.cpp. Error on line " << __LINE__ << ": passed a null pointer" << std::endl;
        exit(1);
    }

    int bytes_per_sample = (int)SDL_AUDIO_BITSIZE(a->wave_spec.format) / 8;
    float samples_per_byte = 0;
    if (bytes_per_sample != 0)
    { 
        samples_per_byte = 1.0 / (float)bytes_per_sample;
    }

    int sample_num = (int)(time * (float)a->wave_spec.freq);
    int total_bytes = sample_num * a->wave_spec.channels * bytes_per_sample;

    if (total_bytes > (int)a->wave_len) { total_bytes = (int)a->wave_len; }

    a->current_pos = a->wave_buff + total_bytes;
    a->current_len = a->wave_len - total_bytes;

    SDL_UnlockAudioDevice(a->device_id);
}

float audio_curr_time(Audio *a)
{
    SDL_LockAudioDevice(a->device_id);

    if (a->current_pos == nullptr | a->wave_buff == nullptr)
    {
        std::cerr << "Audio.cpp. Error on line " << __LINE__ << ": passed a null pointer" << std::endl;
        exit(1);
    }

    int bytes_per_sample = (int)SDL_AUDIO_BITSIZE(a->wave_spec.format) / 8;
    float samples_per_byte = 0;
    if (bytes_per_sample != 0)
    { 
        samples_per_byte = 1.0 / (float)bytes_per_sample;
    }
    float sample_num = ((a->current_pos - a->wave_buff) * samples_per_byte) / a->wave_spec.channels; 

    float curr_time = 0.0;
    if (a->wave_spec.freq != 0)
    {
        curr_time = 1.0 / (float)a->wave_spec.freq * sample_num;
    }
    
    SDL_UnlockAudioDevice(a->device_id);

    return curr_time;
}


float audio_total_time(Audio *a)
{
    int bytes_per_sample = (int)SDL_AUDIO_BITSIZE(a->wave_spec.format) / 8;
    float samples_per_byte = 0;
    if (bytes_per_sample != 0)
    { 
        samples_per_byte = 1.0 / (float)bytes_per_sample;
    }
    float num_samples = (a->wave_len * samples_per_byte) / a->wave_spec.channels; 

    float total_time = 0.0;
    if (a->wave_spec.freq != 0)
    {
        total_time = 1.0 / (float)a->wave_spec.freq * (float)num_samples;
    }

    return total_time;
}

bool audio_at_end(Audio *a)
{
    SDL_LockAudioDevice(a->device_id);
    if (a->current_pos == nullptr | a->wave_buff == nullptr)
    {
        std::cerr << "Audio.cpp. Error on line " << __LINE__ << ": passed a null pointer" << std::endl;
        exit(1);
    }
    if (a->current_pos == a->wave_buff + a->wave_len)
    {
        a->current_pos = a->wave_buff;
        a->current_len = a->wave_len;

        SDL_UnlockAudioDevice(a->device_id);
        return true;
    }
    SDL_UnlockAudioDevice(a->device_id);
    return false;

}
