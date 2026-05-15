#pragma once

#include "vertex.h"
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>

struct Chunk {
    glm::vec3 position;
    int voxelCount;
    float size;
    VkBuffer chunkBuffer;
    VkDeviceMemory chunkMemory;
    std::vector<Cube> voxels;
    void* memoryMap;
};

void initializeChunk(Chunk& chunk);

VkResult createChunkBuffer(VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, Chunk& chunk);