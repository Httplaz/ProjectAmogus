#pragma once

#include <iostream>
#include <chrono>

class Timer {
public:
	Timer() : start(std::chrono::high_resolution_clock::now()) {}

	~Timer() {
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		std::cout << "Time taken: " << duration.count() << " microseconds" << std::endl;
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> start;
};
