#pragma once

#include <vector>
#include <map>
#include <random>
#include <glm/glm.hpp>
#include "Chunk.hpp"
#include "WorldGenerator.hpp"
#include "Timer.hpp"

class SvWorld
{
public:
	uint32_t colors[32 * 32 * 32];
	struct IVec2Comparator {
		bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
			if (a.x < b.x)
				return true;
			else if (a.x > b.x)
				return false;
			else
				return a.y < b.y;
		}
	};


	std::vector<Chunk> loadedChunks;
	std::map<glm::ivec2, size_t, IVec2Comparator> indicesMap;

	void st()
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<uint32_t> dis;
		for (int i = 0; i < 0; i++)
			colors[i] = dis(gen);
	}

	size_t loadChunk(glm::ivec2 pos)
	{
		//Timer timer;
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<uint32_t> dis;

		auto index = loadedChunks.size();
		Chunk a{ pos };
		WorldGenerator::getImpl().fillChunk(a);
		loadedChunks.push_back(std::move(a));
		indicesMap.insert({ pos, index });
		return index;
	}

	Chunk getChunk(glm::ivec2 pos)
	{
		auto it = indicesMap.find(pos);
		if (it == indicesMap.end())
			return loadedChunks[loadChunk(pos)];
		return loadedChunks[it->second];
	}
};