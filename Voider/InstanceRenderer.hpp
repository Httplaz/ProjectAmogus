#pragma once

#include "Shader.hpp"
#include "Texture.hpp"
#include "TextureAtlas.hpp"

class InstanceRenderer {
public:
	struct Vertex
	{
		glm::vec4 pos;
	};
	struct Instance
	{
		Instance(float scale) : scale(scale)
		{
			static uint32_t globalIdx = 0;
			pos.x = static_cast<float>(globalIdx++) / 4096.f;
			spriteIndex = rand() % 16;
		}
		glm::vec3 pos;
		float scale = 1.f;
		float rotation = 0.f;
		uint32_t spriteIndex = 0; //todo set uint8_t
	};

	struct DrawCommand {
		unsigned count;
		unsigned instanceCount;
		unsigned first;
		unsigned baseInstance;
	};

	struct Mesh
	{
		uint16_t firstVertex = 0;
		uint16_t vertexCount = 0;
		uint16_t firstInstance = 0;
		uint16_t instanceCount = 0;
		bool enabled = false;
		DrawCommand MakeCommand()
		{
			return { vertexCount, instanceCount, firstVertex, firstInstance };
		}
	};

	Shader shader;
	Texture texture;
	GLuint VAO, VBO, instanceVBO, drawCommandBO;

	size_t meshesLimit, verticesLimit, instancesLimit;
	std::vector<Mesh> meshes;
	std::vector<uint16_t> freeMeshIndices;
	Vertex* vertexBuffer;
	Instance* instanceBuffer;
	TextureAtlas atlas;
public:
	InstanceRenderer(uint16_t meshCount, uint16_t vertexCount, uint16_t instanceCount = 1000) : shader("defaultInstanced.vert", "defaultInstanced.frag"), texture("default.png"),
	meshesLimit(meshCount), verticesLimit(vertexCount), instancesLimit(instanceCount){
		atlas.init(4096, 3);

		for (int i = 0; i < 7; i++) {
			auto data = Texture::load("assets/pattern" + std::to_string(i+1) + ".png");

			atlas.place(data, 1, i%4*1024, i/4*1024, 0);
		}
		meshes.resize(meshesLimit);
		freeMeshIndices.resize(meshesLimit);
		initRenderData();
	}

	uint16_t addMesh(std::vector<Vertex>&& meshVertices)
	{
		uint16_t newMeshIndex = freeMeshIndices.back();
		freeMeshIndices.pop_back();
		Mesh& mesh = meshes[newMeshIndex];
		mesh.firstVertex = newMeshIndex * verticesLimit;
		mesh.firstInstance = newMeshIndex * instancesLimit;
		mesh.vertexCount = meshVertices.size();
		std::move(std::make_move_iterator(meshVertices.begin()), std::make_move_iterator(meshVertices.end()),
			vertexBuffer+newMeshIndex*verticesLimit);
		mesh.enabled = true;
		return newMeshIndex;
	}

	uint16_t addInstance(uint16_t meshIndex, Instance&& instance)
	{
		Mesh& mesh = meshes[meshIndex];
		instanceBuffer[meshIndex * instancesLimit + mesh.instanceCount] = std::move(instance);
		return meshIndex* instancesLimit + mesh.instanceCount++;
	}

	Instance& accessInstance(uint16_t instanceIdx)
	{
		return instanceBuffer[instanceIdx];
	}

	void Update()
	{
		std::vector<DrawCommand> commands;
		for (auto mesh : meshes)
		{
			if (!mesh.enabled)
			{
				continue;
			}
			commands.push_back({ mesh.MakeCommand() });
		}
		commands.push_back({ 4, 12, 0, 0 });
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawCommandBO);
		glBufferData(GL_DRAW_INDIRECT_BUFFER, commands.size()*sizeof(DrawCommand), commands.data(), GL_DYNAMIC_DRAW);
	}

	void Draw() {
		shader.use();
		shader.setInt("textureArray", 0);
		atlas.use(0);

		Update();

		glBindVertexArray(VAO);

		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawCommandBO);
		glMultiDrawArraysIndirect(GL_TRIANGLE_STRIP, nullptr, 1, 0);

		glBindVertexArray(0);
	}

private:
	void initRenderData() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &instanceVBO);
		glGenBuffers(1, &drawCommandBO);

		glBindVertexArray(VAO);

		// Load vertices
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferStorage(GL_ARRAY_BUFFER, verticesLimit * sizeof(Vertex), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		vertexBuffer = (Vertex*)glMapBufferRange(GL_ARRAY_BUFFER, 0, verticesLimit * sizeof(Vertex),
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		// Load instance data
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferStorage(GL_ARRAY_BUFFER, instancesLimit * sizeof(Instance), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		instanceBuffer = (Instance*)glMapBufferRange(GL_ARRAY_BUFFER, 0, instancesLimit * sizeof(Instance),
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float) + sizeof(uint32_t), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribDivisor(1, 1);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float) + sizeof(uint32_t), (void*)(sizeof(float)*3));
		glEnableVertexAttribArray(2);
		glVertexAttribDivisor(2, 1);
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float) + sizeof(uint32_t), (void*)(sizeof(float) * 4));
		glEnableVertexAttribArray(3);
		glVertexAttribDivisor(3, 1);
		glVertexAttribPointer(4, 1, GL_INT, GL_FALSE, 5 * sizeof(float) + sizeof(uint32_t), (void*)(sizeof(float) * 5));
		glEnableVertexAttribArray(4);
		glVertexAttribDivisor(4, 1);

		glBindVertexArray(0);
	}
};
