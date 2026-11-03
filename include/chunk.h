#pragma once

#include "vertex.h"
#include "voxel.h"
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>
#include <cassert>

struct GridCoordinate {
    int i;
    int j;
    int k;
};

struct Chunk {
    std::vector<MaterialType> materials;
    std::vector<uint32_t> energies;
    std::vector<int64_t> deltaArray; 
    std::vector<float> temperatures;
    GridCoordinate chunkPosition;
    glm::vec3 position;
    int voxelsPerEdge;
    float size;
    VkBuffer chunkBuffer;
    VkDeviceMemory chunkMemory;
    std::vector<FaceInstance> faceRenderList;
    void* memoryMap;
};

inline size_t getVoxelIndex(const Chunk& chunk, int i, int j, int k){
    assert(i >= 0 && i < chunk.voxelsPerEdge && "i is out of range.");
    assert(j >= 0 && j < chunk.voxelsPerEdge && "j is out of range.");
    assert(k >= 0 && k < chunk.voxelsPerEdge && "k is out of range.");
    return static_cast<size_t>(i) * chunk.voxelsPerEdge * chunk.voxelsPerEdge + static_cast<size_t>(j) * chunk.voxelsPerEdge + static_cast<size_t>(k);
}

const uint32_t stoneAt300k = 65000000;

void computeHeatFlow(std::vector<Chunk>& chunks, size_t chunkIndex, double dt);

void applyHeatFlow(Chunk& chunk);

void updateTemperatures(Chunk& chunk);

uint64_t getChunkEnergy(const Chunk& chunk);

void fillFaceRenderList(Chunk& chunk);

void initializeChunk(Chunk& chunk);

VkResult createChunkBuffer(VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, Chunk& chunk);