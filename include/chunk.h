#pragma once

#include "vertex.h"
#include "voxel.h"
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>

struct Chunk {
    glm::vec3 position;
    int voxelsPerEdge;
    float size;
    VkBuffer chunkBuffer;
    VkDeviceMemory chunkMemory;
    std::vector<StorageVoxel> voxelDataList;
    std::vector<FaceInstance> faceRenderList;
    void* memoryMap;
};

void fillFaceRenderList(Chunk& chunk);

void initializeChunk(Chunk& chunk);

VkResult createChunkBuffer(VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, Chunk& chunk);