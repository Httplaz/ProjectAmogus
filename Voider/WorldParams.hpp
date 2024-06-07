#pragma once
#include <cstdint>

class WorldParams
{
public:
	static constexpr uint32_t chunkSize = 32;
	static constexpr uint32_t chunkSizeSq = chunkSize * chunkSize;
	static constexpr uint32_t gpuChunkMapSide = 32;
	static constexpr uint32_t gpuChunkMapSideSq = gpuChunkMapSide * gpuChunkMapSide;
};
