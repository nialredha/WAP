#ifndef WAVE_IO_HPP
#define WAVE_IO_HPP

#define BUFFER_SIZE (4)
#define	BITS_PER_BYTE (8)
#define PCM (1)

class WaveIO
{
  public:
	// constructor used for parsing a wave file
	WaveIO(std::string fname);

	// constructor used for writing a wave file
	WaveIO(std::string fname, uint16_t num_channels, uint32_t sample_rate, 
			uint16_t bits_per_sample, uint32_t duration_sec);

	~WaveIO() { free(_data_buffer); }

	void parse();
	void write();
	void print_metadata();

	// getters
	uint16_t* get_data();

	// setters
	void set_fname(std::string fname);
    void set_data(uint16_t* data);

  private:
	std::string _fname{};
	FILE* _fp{};

	uint32_t _chunk_size{};
	uint32_t _fmt_chunk_size{};
	uint16_t _audio_format{PCM}; 
	uint16_t _num_channels{};
	uint32_t _sample_rate{};
	uint32_t _byte_rate{};
	uint16_t _block_align{};
	uint16_t _bits_per_sample{};
	uint16_t _bytes_per_sample{};
	uint32_t _data_chunk_size{};

	uint32_t _duration_sec{};
	uint32_t _num_samples{};
	uint16_t* _data_buffer;

	// little endian format
	void fill_buffer(uint8_t* buffer, uint32_t value);
	void fill_buffer(uint8_t* buffer, uint16_t first_value, uint16_t second_value);

	// big endian format
	void fill_buffer(uint8_t* buffer, std::string name);
};

#endif // WAVE_IP_HPP
