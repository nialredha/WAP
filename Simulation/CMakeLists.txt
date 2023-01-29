add_library(
	Simulation 
	SoundSim.hpp
	SoundSim.cpp
	)

target_include_directories(Simulation PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/")
