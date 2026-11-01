#pragma once

#include "vertex.h"
#include "voxel.h"
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>

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

uint64_t getChunkEnergy(const Chunk& chunk);

void fillFaceRenderList(Chunk& chunk);

void initializeChunk(Chunk& chunk);

VkResult createChunkBuffer(VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, Chunk& chunk);