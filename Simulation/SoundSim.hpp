#ifndef SOUND_SIM_HPP
#define SOUND_SIM_HPP
/* 
	Module to generate simulated wave data of varying frequencies 
*/


class Sound_Sim 
{
  public:
	Sound_Sim(int num_frequencies, 
				float* frequencies, 
				int num_samples, 
				int sample_rate);

	void simulate_data();

	float* get_data();

	uint32_t* get_data(int max_value);

  private:
	int _num_frequencies;
	float* _frequencies;

	int _num_samples;
	int _sample_rate;

	float* _data;

	void normalize_data(double* value); 
};

#endif // SOUND_SIM_HPP
