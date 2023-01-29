#include <cmath>
#include <iostream>

#include "SoundSim.hpp"

Sound_Sim::Sound_Sim(int num_frequencies, float* frequencies, int num_samples, int sample_rate) : 
		_num_frequencies(num_frequencies), _frequencies(frequencies),
		_num_samples(num_samples), _sample_rate(sample_rate) 
{
	_data = (float*)malloc(sizeof(float) * _num_samples);
}

void Sound_Sim::simulate_data() 
{
	double time, amplitude;

	for(int n = 0; n < _num_samples; n++) 
	{
		time = (double)n / (double)_sample_rate;	// sample / (samples/sec)
		amplitude = 0.0;
		
		for(int i = 0; i < _num_frequencies; i++) 
		{
			amplitude += sin(2.0 * M_PI * (double)(_frequencies[i]) * time);
		}

		if(_num_frequencies > 1) 
		{
			// no real reason to normalize here, but it could come in handy
			normalize_data(&amplitude);
		}

		_data[n] = (float)amplitude;
	}
}

void Sound_Sim::normalize_data(double* amplitude)
{
	double max = (double)_num_frequencies;
	double min = (double)(-1.0 * _num_frequencies);
	double range = max - min;

	*amplitude = (*amplitude - min) / range;
}

float* Sound_Sim::get_data() 
{ 
	return _data;
}

uint32_t* Sound_Sim::get_data(int max_value) 
{ 
	uint32_t* wav_data = (uint32_t*)malloc(sizeof(uint32_t) * _num_samples);
	for(int n = 0; n < _num_samples; n++) 
	{
		wav_data[n] = (uint32_t)(_data[n] * max_value);
	}

	return wav_data;
}
