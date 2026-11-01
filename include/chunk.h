#pragma once

#include "vertex.h"
#include "voxel.h"
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>
#include <cassert>

struct Chunk {
    std::vector<MaterialType> materials;
    std::vector<uint32_t> energies;
    glm::vec3 position;
    int voxelsPerEdge;
    float size;
    VkBuffer chunkBuffer;
    VkDeviceMemory chunkMemory;
    std::vector<FaceInstance> faceRenderList;
    void* memoryMap;
};

inline size_t getIndex(const Chunk& chunk, int i, int j, int k){
    assert(i >= 0 && i < chunk.voxelsPerEdge && "i is out of range.");
    assert(j >= 0 && j < chunk.voxelsPerEdge && "j is out of range.");
    assert(k >= 0 && k < chunk.voxelsPerEdge && "k is out of range.");
    return static_cast<size_t>(i) * chunk.voxelsPerEdge * chunk.voxelsPerEdge + static_cast<size_t>(j) * chunk.voxelsPerEdge + static_cast<size_t>(k);
}

uint64_t getChunkEnergy(const Chunk& chunk);

void fillFaceRenderList(Chunk& chunk);

void initializeChunk(Chunk& chunk);

VkResult createChunkBuffer(VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, Chunk& chunk);