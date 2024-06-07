#pragma once

#include <FastNoise/FastNoise.h>
#include <glm/glm.hpp>
#include <iostream>
#include <array>
#include <mutex>
#include "Chunk.hpp"
#include <memory>
#include <random>

class ColorsGenerator;

class WorldGenerator
{
public:
	virtual uint32_t generateTile(glm::ivec2 pos) const = 0;
	virtual void fillChunk(Chunk& chunk) const = 0;

	static WorldGenerator& getImpl() { return *impl.get(); }

	static void init()
	{
		perlinSourse = FastNoise::New<FastNoise::Perlin>();
		perlin = FastNoise::New<FastNoise::FractalFBm>();
		perlin->SetSource(perlinSourse);
		perlin->SetOctaveCount(5);
	}

	static void setImpl(std::unique_ptr<WorldGenerator> _impl)
	{
		impl = std::move(_impl);
	}

protected:
	inline static std::unique_ptr<WorldGenerator> impl;

	inline static FastNoise::SmartNode<FastNoise::Perlin> perlinSourse;
	inline static FastNoise::SmartNode<FastNoise::FractalFBm> perlin;

	inline static std::mt19937 mt19937;
};

class ColorsGenerator : public WorldGenerator
{
public:
	virtual uint32_t generateTile(glm::ivec2 pos) const override
	{
		double scale = 0.01593591;
		double result0 = perlin->GenSingle2D(pos.x * scale, pos.y * scale, 0);
		double result1 = perlin->GenSingle2D(pos.x * scale, pos.y * scale, 1);
		double result2 = perlin->GenSingle2D(pos.x * scale, pos.y * scale, 2);
		return noiseToTile({ result0, result1, result2 });
	}

	virtual void fillChunk(Chunk& chunk) const override
	{
		double scale = 0.01593591;


		std::array<float, WorldParams::chunkSizeSq> noiseX, noiseY, noiseZ;

		perlin->GenUniformGrid2D(noiseX.data(), chunk.offset.x * WorldParams::chunkSize, chunk.offset.y * WorldParams::chunkSize, WorldParams::chunkSize, WorldParams::chunkSize, scale, 135352);
		perlin->GenUniformGrid2D(noiseY.data(), chunk.offset.x * WorldParams::chunkSize, chunk.offset.y * WorldParams::chunkSize, WorldParams::chunkSize, WorldParams::chunkSize, scale, 0.1f);
		perlin->GenUniformGrid2D(noiseZ.data(), chunk.offset.x * WorldParams::chunkSize, chunk.offset.y * WorldParams::chunkSize, WorldParams::chunkSize, WorldParams::chunkSize, scale, 885894);

		for (int i = 0; i < WorldParams::chunkSizeSq; i++)
			chunk.tiles[i] = noiseToTile({ noiseX[i], noiseY[i], noiseZ[i] });

	}
private:
	static uint32_t noiseToTile(glm::vec3 val)
	{
		return uint32_t(std::min(2.f, std::max(val.x + 1.000000f, 0.f)) / 2.f * 255.f) * 256 +
			uint32_t(std::min(2.f, std::max(val.y + 1.000000f, 0.f)) / 2.f * 255.f) * 256 * 256 +
			uint32_t(std::min(2.f, std::max(val.z + 1.00000f, 0.f)) / 2.f * 255.f) * 256 * 256 * 256;
	}
};

class CavesGenerator : public WorldGenerator
{
public:
	virtual uint32_t generateTile(glm::ivec2 pos) const
	{
		return 0;
	}
	virtual void fillChunk(Chunk& chunk) const
	{
		if (mt19937() % 50 == 1)
		{
			for (int x = 0; x < WorldParams::chunkSize; x++)
				for (int y = 0; y < WorldParams::chunkSize; y++)
				{
					double scale = 0.01593591;
					glm::ivec2 pos = chunk.offset * int(WorldParams::chunkSize) + glm::ivec2{ x, y };
					double result0 = perlin->GenSingle2D(pos.x * scale, pos.y * scale, 0);
					if (result0 > 0.3)
						chunk.tiles[y * WorldParams::chunkSize + x] = 2;
					else
						chunk.tiles[y * WorldParams::chunkSize + x] = 1;
				}
		}
		else
		{
			colors.fillChunk(chunk);
		}

	}
protected:
	ColorsGenerator colors;
};