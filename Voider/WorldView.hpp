#pragma once

#include <map>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "World.hpp"
#include "WorldParams.hpp"
#include "Shader.hpp"
#include "RectangleMesh.hpp"
#include "TextureAtlas.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class WorldView {
public:
	WorldView(SvWorld& viewableWorld);
	~WorldView();

	void setViewMatrix(glm::mat4 mat);

	void update();

	void render();

	void setDefaultChunk(Chunk chunk);

	Shader shader;

private:
	SvWorld& world;
	GLuint tilesSSBO;
	GLuint chunkMapSSBO;
	RectangleMesh mesh;

	Chunk* chunkDataBuffer;
	uint32_t* chunkMapBuffer;

	Chunk defaultChunk;

	glm::mat4 viewMatrix;
};

#include <iostream>

TextureAtlas atlas1;

WorldView::WorldView(SvWorld& viewableWorld)
	: world(viewableWorld), shader("worldView.vert", "worldView.frag"), mesh(-1, -1, 2, 2) {


	atlas1.init(1024, 3);

	auto data = Texture::load("default.png");

	atlas1.place(data, 0, 0, 0, 0);

	glGenBuffers(1, &tilesSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, tilesSSBO);

	// Создаем буфер для данных тайлов
	GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, WorldParams::gpuChunkMapSideSq * sizeof(Chunk), NULL, flags);//::gpuChunkBufferCapacity * WorldParams::chunkSize * WorldParams::chunkSize * sizeof(uint32_t), NULL, flags);

	glGenBuffers(1, &chunkMapSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, chunkMapSSBO);

	// Создаем буфер для карты индексов чанков
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, WorldParams::gpuChunkMapSideSq * sizeof(uint32_t), NULL, flags);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, tilesSSBO);
	chunkDataBuffer = (Chunk*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, WorldParams::gpuChunkMapSideSq * sizeof(Chunk), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

	chunkDataBuffer[0] = { {-1, -1} };

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, chunkMapSSBO);
	chunkMapBuffer = (uint32_t*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, WorldParams::gpuChunkMapSideSq * sizeof(uint32_t), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

WorldView::~WorldView() {
	glDeleteBuffers(1, &tilesSSBO);
	glDeleteBuffers(1, &chunkMapSSBO);
}

void WorldView::setViewMatrix(glm::mat4 mat)
{
	viewMatrix = mat;
	shader.setMat4("cameraMatrix", mat);
}


void WorldView::update() {
	glm::vec2 chunkOffset = viewMatrix * glm::vec4(0, 0, 0, 1) / 2.f;
	glm::ivec2 centerChunk = viewMatrix * glm::vec4(0, 0, 0, 1) / 2.f;

	glm::ivec2 lowLeftChunk = centerChunk - glm::ivec2(WorldParams::gpuChunkMapSide / 2);
	glm::ivec2 highRightChunk = centerChunk + glm::ivec2(WorldParams::gpuChunkMapSide / 2);

	//std::cout << lowLeftChunk.x << " " << lowLeftChunk.y << " " << highRightChunk.x << " " << highRightChunk.y << "\n";

	for (int i = lowLeftChunk.x; i <= highRightChunk.x; i++)
		for (int j = lowLeftChunk.y; j <= highRightChunk.y; j++)
		{
			int cx = i % WorldParams::gpuChunkMapSide, cy = j % WorldParams::gpuChunkMapSide;
			if (chunkDataBuffer[cy * WorldParams::gpuChunkMapSide + cx].offset != glm::ivec2{ i, j })
				chunkDataBuffer[cy * WorldParams::gpuChunkMapSide + cx] = world.getChunk({ i, j });
			//std::cout << cx << " " << cy << "\n";
		}

	//for (int i = 0; i < 32; i++)
		//for (int j = 0; j < 32; j++)
			//chunkDataBuffer[j * 32 + i] = world.getChunk({ i, j });

	shader.setVec2("chunkOffset", chunkOffset);
}


void WorldView::render() {
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, tilesSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, chunkMapSSBO);


	shader.use();
	shader.setInt("textureArray", 0);
	atlas1.use(0);

	shader.setInt("chunkMapSide", WorldParams::gpuChunkMapSide);
	shader.setInt("chunkSize", WorldParams::chunkSize);
	shader.setInt("chunkSizeSq", WorldParams::chunkSizeSq);

	mesh.draw();
}

void WorldView::setDefaultChunk(Chunk chunk)
{
	defaultChunk = chunk;
}
