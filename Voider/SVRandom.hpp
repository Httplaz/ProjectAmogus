#pragma once

#include <random>
#include <chrono>

class SVRandom {
public:
	SVRandom()
	{
		std::random_device device;
		source.seed(device());
	}

	template<typename T>
	T next(T max)
	{
		if constexpr (std::is_integral_v<T>) {
			std::uniform_int_distribution<T> range(0, max);
			return range(source);
		}
		else if constexpr (std::is_floating_point_v<T>) {
			std::uniform_real_distribution<T> range(0, max);
			return range(source);
		}
		else {
			// Handle unsupported types
			static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>,
				"Unsupported type for returnRandom");
		}
	}

	template<typename T>
	T next(T min, T max)
	{
		return next(max - min) + min;
	}
private:
	std::mt19937 source;
};
