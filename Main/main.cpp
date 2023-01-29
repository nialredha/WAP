/* 
	Purpose Goes Here...
*/

#include <cmath>
#include <iostream>

#include "SoundSim.hpp"
#include "WaveIO.hpp"

int main(int argc, char *argv[]) 
{
	std::string data_dir{};
	if (argc < 2)
	{
		std::cerr << "No path to Data directory provided!" << std::endl;
		exit(1);
	} 
	else { data_dir = argv[1]; }


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
	std::cout << input_fname << std::endl;
	WaveIO wave(input_fname); // automatically parses the data

	std::string output_fname = data_dir + "/d7_test.wav";
	wave.set_fname(output_fname);
	wave.write();


	return 0;
}
