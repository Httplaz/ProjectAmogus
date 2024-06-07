#pragma once

#include <map>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

template <typename Vertex, typename Instance>
class Vertexpool {
public:
    Vertexpool(size_t vertexSize, size_t instanceSize);
    ~Vertexpool();

    size_t addMesh(std::vector<Vertex>&& vertices);
    size_t addInstance(size_t meshRef, Instance&& inst);
    void removeMesh(size_t meshRef);
    void removeInstance(size_t instRef);

    void render();

    Shader shader;

private:
    GLuint verticesVBO;
    GLuint instancesVBO;
    GLuint VAO;

    size_t vertexBufferSize;
    size_t instanceBufferSize;

    std::map<size_t, std::pair<size_t, size_t>> meshMap; // Maps meshRef to a pair of start index and count
    std::map<size_t, size_t> instanceMap; // Maps instRef to index in instance buffer

    Vertex* vertexBuffer;
    Instance* instanceBuffer;

    size_t nextMeshRef;
    size_t nextInstRef;

    void initializeBuffers();
    void updateVertexBuffer();
    void updateInstanceBuffer();
};

template <typename Vertex, typename Instance>
Vertexpool<Vertex, Instance>::Vertexpool(size_t vertexSize, size_t instanceSize)
    : vertexBufferSize(vertexSize), instanceBufferSize(instanceSize), nextMeshRef(0), nextInstRef(0),
    shader("default.vert", "default.frag")
{
    initializeBuffers();
}

template <typename Vertex, typename Instance>
Vertexpool<Vertex, Instance>::~Vertexpool() {
    glDeleteBuffers(1, &verticesVBO);
    glDeleteBuffers(1, &instancesVBO);
    glDeleteVertexArrays(1, &VAO);
}

template <typename Vertex, typename Instance>
void Vertexpool<Vertex, Instance>::initializeBuffers() {
    // Generate and bind VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Vertex attribute
    glBindBuffer(GL_ARRAY_BUFFER, verticesVBO);
    glVertexAttribIPointer(0, 2, GL_INT, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Instance attribute
    glBindBuffer(GL_ARRAY_BUFFER, instancesVBO);
    glVertexAttribIPointer(1, 2, GL_INT, sizeof(Instance), (void*)0);
    glEnableVertexAttribArray(1);

    // Tell OpenGL that the instance attribute should advance once per instance instead of per vertex
    glVertexAttribDivisor(1, 1);

    // Unbind VAO to prevent further modification
    glBindVertexArray(0);

    // Map buffers persistently
    glBindBuffer(GL_ARRAY_BUFFER, verticesVBO);
    vertexBuffer = (Vertex*)glMapBufferRange(GL_ARRAY_BUFFER, 0, vertexBufferSize * sizeof(Vertex),
        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

    glBindBuffer(GL_ARRAY_BUFFER, instancesVBO);
    instanceBuffer = (Instance*)glMapBufferRange(GL_ARRAY_BUFFER, 0, instanceBufferSize * sizeof(Instance),
        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

    // Unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

template <typename Vertex, typename Instance>
size_t Vertexpool<Vertex, Instance>::addMesh(std::vector<Vertex>&& vertices) {
    size_t meshRef = nextMeshRef++;
    size_t start = meshMap.empty() ? 0 : meshMap.rbegin()->second.first + meshMap.rbegin()->second.second;
    size_t count = vertices.size();

    // Copy vertices to the vertex buffer
    std::copy(vertices.begin(), vertices.end(), vertexBuffer + start);
    updateVertexBuffer();

    // Store mesh information
    meshMap[meshRef] = std::make_pair(start, count);

    return meshRef;
}

template <typename Vertex, typename Instance>
size_t Vertexpool<Vertex, Instance>::addInstance(size_t meshRef, Instance&& inst) {
    if (meshMap.find(meshRef) == meshMap.end()) {
        // Mesh reference not found
        return -1;
    }

    size_t instRef = nextInstRef++;
    size_t index = instanceMap.empty() ? 0 : instanceMap.rbegin()->second + 1;

    // Copy instance to the instance buffer
    instanceBuffer[index] = inst;
    updateInstanceBuffer();

    // Store instance information
    instanceMap[instRef] = index;

    return instRef;
}

template <typename Vertex, typename Instance>
void Vertexpool<Vertex, Instance>::removeMesh(size_t meshRef) {
    auto it = meshMap.find(meshRef);
    if (it != meshMap.end()) {
        // Remove mesh from map
        meshMap.erase(it);
        // Note: This does not actually remove the mesh from the buffer
        // You would need to compact the buffer and update all affected mesh references
    }
}

template <typename Vertex, typename Instance>
void Vertexpool<Vertex, Instance>::removeInstance(size_t instRef) {
    auto it = instanceMap.find(instRef);
    if (it != instanceMap.end()) {
        // Remove instance from map
        instanceMap.erase(it);
        // Note: This does not actually remove the instance from the buffer
        // You would need to compact the buffer and update all affected instance references
    }
}

template <typename Vertex, typename Instance>
void Vertexpool<Vertex, Instance>::render() {
    shader.use();
    glBindVertexArray(VAO);

    // Assuming that each mesh has its own draw call
    for (const auto& pair : meshMap) {
        size_t start = pair.second.first;
        size_t count = pair.second.second;

        // Draw instances
        glDrawArraysInstanced(GL_TRIANGLES, start, count, instanceMap.size());
    }

    glBindVertexArray(0);
}

template <typename Vertex, typename Instance>
void Vertexpool<Vertex, Instance>::updateVertexBuffer() {
    // Since the buffer is persistently mapped, there's no need to call glBufferSubData
    // The memory is already updated. However, you might want to use a fence for synchronization.
}

template <typename Vertex, typename Instance>
void Vertexpool<Vertex, Instance>::updateInstanceBuffer() {
    // Since the buffer is persistently mapped, there's no need to call glBufferSubData
    // The memory is already updated. However, you might want to use a fence for synchronization.
}

// ... (rest of the code)
