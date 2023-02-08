/* 
	References:
	- https://ccrma.stanford.edu/courses/422-winter-2014/projects/WaveFormat/
	- http://kernelx.weebly.com/bitwise-operators-in-c.html
*/

#include <iostream>
#include "WaveIO.hpp"

WaveIO::WaveIO(std::string fname) : _fname(fname)
{
    _data_buffer = nullptr;

	parse();
}

WaveIO::WaveIO(std::string fname, uint16_t num_channels, 
				uint32_t sample_rate, uint16_t bits_per_sample,
				uint32_t duration_sec) :
				_fname(fname), _num_channels(num_channels),
				_sample_rate(sample_rate), _bits_per_sample(bits_per_sample),
				_duration_sec(duration_sec)
{
	_num_samples = _sample_rate * _duration_sec;
	_bytes_per_sample = _bits_per_sample / BITS_PER_BYTE;
	_byte_rate = _sample_rate * _num_channels * _bytes_per_sample;
	_block_align = num_channels * _bytes_per_sample;
    _data_buffer = nullptr;
}

void WaveIO::parse()
{
	uint8_t buffer[BUFFER_SIZE];

    _fp = fopen(_fname.c_str(), "rb");
	if (!_fp) 
	{ 
		std::cerr << "Couldn't open file..." << std::endl; 
		exit(1);
	}

	// RIFF Chunk Descriptor **************************************************

	// Chunk ID (big endian)
	fread(buffer, 1, 4, _fp);
	if(buffer[0] != 'R' || buffer[1] != 'I' || buffer[2] != 'F' || buffer[3] != 'F')
	{
		std::cerr << "File: " << _fname.c_str() << "is not RIFF!" << std::endl;
		exit(1);
	}

	// Chunk Size (little endian) - size of file minus chunk id and chunk size 
	fread(buffer, 1, 4, _fp);
	_chunk_size = buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0] << 0;

	// std::cout << "Chunk Size = " << chunk_size << std::endl;
	// std::cout << "Data Size = " << data_size << std::endl;

	// Format (big endian)
	fread(buffer, 1, 4, _fp);
	if(buffer[0] != 'W' || buffer[1] != 'A' || buffer[2] != 'V' || buffer[3] != 'E')
	{
		std::cerr << "File: " << _fname.c_str() << "is not WAVE!" << std::endl;
		exit(1);
	}

	// FMT Sub-Chunk **********************************************************

	// Sub-Chunk-1 ID (big endian)
	fread(buffer, 1, 4, _fp);
	if (buffer[0] != 'f' || buffer[1] != 'm' || buffer[2] != 't')
	{
		std::cerr << "Can't find the 'fmt' sub-chunk.." << std::endl;
		exit(1);
	}

	// Sub-Chunk-1 Size (little endian)
	fread(buffer, 1, 4, _fp);

	_fmt_chunk_size = buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0] << 0;

	// std::cout << "FMT Chunk Size = " << _fmt_chunk_size << std::endl;
	
	// Audio Format and Number of Channels (little endian)
	fread(buffer, 1, 4, _fp);

	_audio_format = buffer[1] << 8 | buffer[0] << 0; // expecting 1 a.k.a PCM
	_num_channels = buffer[3] << 8 | buffer[2] << 0;

	// std::cout << "Audio Format = " << _audio_format << std::endl;
	// std::cout << "Number of Channels = " << _num_channels << std::endl;

	if(_audio_format != 1)
	{ 
		std::cerr << "Format is not PCM, meaning there is compression!" << std::endl;
		exit(1);
	}

	// Sample Rate (little endian)
	fread(buffer, 1, 4, _fp);

	_sample_rate = buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0] << 0;

	// std::cout << "Sample Rate = " << _sample_rate << std::endl;

	// Byte Rate (little endian) = sample_rate * num_channels * bytes_per_sample
	fread(buffer, 1, 4, _fp);
	_byte_rate = buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0] << 0;

	// std::cout << "Byte Rate = " << _byte_rate << std::endl;

	// Block Align (num_channels * bytes_per_sample) and Bits Per Sample (little endian)
	fread(buffer, 1, 4, _fp);

	_block_align = buffer[1] << 8 | buffer[0] << 0;
	_bits_per_sample = buffer[3] << 8 | buffer[2] << 0;

	// std::cout << "Block Align = " << _block_align << std::endl;
	// std::cout << "Bits Per Sample = " << _bits_per_sample << std::endl;

	_bytes_per_sample = _bits_per_sample / BITS_PER_BYTE;

	// Data Sub-Chunk *********************************************************

	// Sub-Chunk-2 ID (big endian)
	fread(buffer, 1, 4, _fp);
	if (buffer[0] != 'd' || buffer[1] != 'a' || buffer[2] != 't' || buffer[3] != 'a')
	{
		std::cerr << "Can't find the 'data' sub-chunk.." << std::endl;
		exit(1);
	}

	// Sub-Chunk-2 Size (little endian)
	fread(buffer, 1, 4, _fp);

	_data_chunk_size = buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0] << 0;

	if(_data_chunk_size != _chunk_size - 36) 
	{ 
		std::cerr << "Things aren't looking right..." << std::endl; 
		exit(1);
	} 

	// std::cout << "Data Chunk Size = " << data_chunk_size << std::endl;


	// Data
	_num_samples = _data_chunk_size / (_num_channels * _bytes_per_sample);
	_duration_sec = _num_samples / _sample_rate; // TODO: is this okay to do?

	_data_buffer = (uint16_t*)malloc(sizeof(uint16_t) * _num_samples);
	if (_data_buffer == nullptr) 
	{ 
		std::cerr << "Failed to allocate _data_buffer on heap..." << std::endl; 
		exit(1);
	} 

	fread(_data_buffer, _bytes_per_sample, _num_samples, _fp);

	fclose(_fp);
}

void WaveIO::write()
{
	uint8_t buffer[BUFFER_SIZE];
	std::string name;

    _fp = fopen(_fname.c_str(), "wb");
	if (!_fp) 
	{ 
		std::cerr << "Couldn't open file..." << std::endl; 
		exit(1);
	} 

	// RIFF Chunk Descriptor **************************************************

	// Chunk ID (big endian)
	name = "RIFF";
	fill_buffer(buffer, name);
	fwrite(buffer, 1, 4, _fp);

	// Chunk Size (little endian) - size of file minus chunk id and chunk size 
	fill_buffer(buffer, _chunk_size);
	fwrite(buffer, 1, 4, _fp);

	// Format (big endian)
	name = "WAVE";
	fill_buffer(buffer, name);
	fwrite(buffer, 1, 4, _fp);

	// FMT Sub-Chunk **********************************************************

	// Sub-Chunk-1 ID (big endian)
	name = "fmt ";
	fill_buffer(buffer, name);
	fwrite(buffer, 1, 4, _fp);

	// Sub-Chunk-1 Size (little endian)
	fill_buffer(buffer, _fmt_chunk_size);
	fwrite(buffer, 1, 4, _fp);

	// Audio Format and Number of Channels (little endian)
	fill_buffer(buffer, _audio_format, _num_channels);
	fwrite(buffer, 1, 4, _fp);

	// Sample Rate (little endian)
	fill_buffer(buffer, _sample_rate);
	fwrite(buffer, 1, 4, _fp);

	// Byte Rate (little endian) = sample_rate * num_channels * bytes_per_sample
	fill_buffer(buffer, _byte_rate);
	fwrite(buffer, 1, 4, _fp);

	// Block Align (num_channels * bytes_per_sample) and Bits Per Sample (little endian)
	fill_buffer(buffer, _block_align, _bits_per_sample);
	fwrite(buffer, 1, 4, _fp);

	// Data Sub-Chunk *********************************************************

	// Sub-Chunk-2 ID (big endian)
	name = "data";
	fill_buffer(buffer, name);
	fwrite(buffer, 1, 4, _fp);

	// Sub-Chunk-2 Size (little endian)
	fill_buffer(buffer, _data_chunk_size);
	fwrite(buffer, 1, 4, _fp);

	// Data
	if (_data_buffer == nullptr) 
	{ 
		std::cerr << "No data available to write... "<< std::endl; 
		exit(1);
	} 
	fwrite(_data_buffer, _bytes_per_sample, _num_samples, _fp);

	fclose(_fp);
}

void WaveIO::print_metadata()
{
	std::cout << std::endl;
	std::cout << "*************************************" << std::endl;
	std::cout << "File Name ----------> " <<  _fname << std::endl;
	std::cout << "Number of Channels ->	" << _num_channels << std::endl;
	std::cout << "Sample Rate -------->	" << _sample_rate << std::endl;
	std::cout << "Bit Depth ---------->	" << _bits_per_sample << std::endl;
	std::cout << "Duration ----------->	" << _duration_sec << std::endl;
	std::cout << "Number of Samples -->	" << _num_samples << std::endl;
	std::cout << "Audio Format ------->	PCM" << std::endl;
	std::cout << "*************************************" << std::endl;
	std::cout << std::endl;
}

uint16_t* WaveIO::get_data()
{
	return _data_buffer;
}

void WaveIO::set_fname(std::string fname)
{
	_fname = fname;
}

void WaveIO::set_data(uint16_t* data)
{
    // int num_bytes = _num_samples * _bytes_per_sample;
    // memset(_data_buffer, 0, num_bytes);
    // memcpy(_data_buffer, data, num_bytes);

    _data_buffer = data;
}

void WaveIO::fill_buffer(uint8_t *buffer, uint32_t value)
{
	buffer[0] = value >> 0;
	buffer[1] = value >> 8;
	buffer[2] = value >> 16;
	buffer[3] = value >> 24;
}

void WaveIO::fill_buffer(uint8_t *buffer, uint16_t first_value, uint16_t second_value)
{
	buffer[0] = first_value >> 0;
	buffer[1] = first_value >> 8;
	buffer[2] = second_value >> 0;
	buffer[3] = second_value >> 8;
}

void WaveIO::fill_buffer(uint8_t* buffer, std::string name)
{
	buffer[0] = name[0];
	buffer[1] = name[1];
	buffer[2] = name[2];
	buffer[3] = name[3];
}
