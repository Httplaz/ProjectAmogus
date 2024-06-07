#pragma once
#include "WorldParams.hpp"

class Chunk
{
public:
	glm::ivec2 offset = { INT_MAX, INT_MAX };
	uint32_t tiles[WorldParams::chunkSize * WorldParams::chunkSize];
};