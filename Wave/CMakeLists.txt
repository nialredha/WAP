add_library(
	Wave 
	WaveIO.hpp
	WaveIO.cpp
	)

target_include_directories(Wave PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/")
