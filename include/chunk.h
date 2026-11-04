#pragma once

#include "vertex.h"
#include "voxel.h"
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>
#include <cassert>

struct GridCoordinate{
    int i = 0;
    int j = 0;
    int k = 0;
};

struct ActiveSet{
    std::vector<uint32_t> voxelIndices;
    std::vector<uint8_t> voxelFlags;
};

struct Chunk{
    std::vector<MaterialType> materials;
    std::vector<uint32_t> energies;
    std::vector<int64_t> deltaArray; 
    std::vector<float> temperatures;
    ActiveSet active;
    ActiveSet nextActive;
    GridCoordinate chunkPosition = {};
    glm::vec3 position = {};
    int voxelsPerEdge = 0;
    float size = 0.0f;
    VkBuffer chunkBuffer = VK_NULL_HANDLE;
    VkDeviceMemory chunkMemory = VK_NULL_HANDLE;
    std::vector<FaceInstance> faceRenderList;
    void* memoryMap = nullptr;
};

inline size_t getVoxelIndex(const Chunk& chunk, int i, int j, int k){
    assert(i >= 0 && i < chunk.voxelsPerEdge && "i is out of range.");
    assert(j >= 0 && j < chunk.voxelsPerEdge && "j is out of range.");
    assert(k >= 0 && k < chunk.voxelsPerEdge && "k is out of range.");
    return static_cast<size_t>(i) * chunk.voxelsPerEdge * chunk.voxelsPerEdge + static_cast<size_t>(j) * chunk.voxelsPerEdge + static_cast<size_t>(k);
}

inline GridCoordinate getVoxelCoordinates(const Chunk& chunk, size_t index){
    assert(index < chunk.energies.size() && "Index out of range.");
    return {
        static_cast<int>(index / (static_cast<size_t>(chunk.voxelsPerEdge) * chunk.voxelsPerEdge)), 
        static_cast<int>((index / static_cast<size_t>(chunk.voxelsPerEdge)) % chunk.voxelsPerEdge), 
        static_cast<int>(index % static_cast<size_t>(chunk.voxelsPerEdge))
    };  
}

inline void refreshVoxelTemperature(Chunk& chunk, size_t index){
    assert(index < chunk.temperatures.size() && "Index out of range.");
    chunk.temperatures[index] = static_cast<float>(energyToTemperature(chunk.energies[index], chunk.materials[index]));
}

inline void validateVoxelIndexing([[maybe_unused]] const Chunk& chunk) {
    #ifndef NDEBUG
    for(int i = 0; i < chunk.voxelsPerEdge; i++){
        for(int j = 0; j < chunk.voxelsPerEdge; j++){
            for(int k = 0; k < chunk.voxelsPerEdge; k++){
                size_t voxelIndex = getVoxelIndex(chunk, i, j, k); 
                GridCoordinate voxelCoordinates = getVoxelCoordinates(chunk, voxelIndex);
                assert(voxelCoordinates.i == i && voxelCoordinates.j == j && voxelCoordinates.k == k && "Inconsistency in the round trip.");
            }
        }
    }
    #endif
}

void wakeVoxel(ActiveSet& activeSet, uint32_t voxelIndex);

inline constexpr uint32_t stoneAt300k = 65000000;

void computeHeatFlow(std::vector<Chunk>& chunks, size_t chunkIndex, double dt);

void applyHeatFlow(Chunk& chunk);

void updateTemperatures(Chunk& chunk);

uint64_t getChunkEnergy(const Chunk& chunk);

void fillFaceRenderList(Chunk& chunk);

void initializeChunk(Chunk& chunk);

VkResult createChunkBuffer(VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, Chunk& chunk);