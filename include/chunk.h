#pragma once

#include "vertex.h"
#include "voxel.h"
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>
#include <cassert>
#include <optional>

inline constexpr double initialTemperature = 300.0;

struct GridCoordinate{
    int i = 0;
    int j = 0;
    int k = 0;
};

struct VoxelLocation{
    size_t chunkIndex = 0;
    size_t voxelIndex = 0;
};

struct VoxelSet{
    std::vector<uint32_t> voxelIndices;
    std::vector<uint8_t> voxelFlags;
};

struct Chunk{
    std::vector<MaterialType> materials;
    std::vector<uint32_t> energies;
    std::vector<int64_t> deltaArray; 
    std::vector<float> temperatures;
    VoxelSet active;
    VoxelSet nextActive;
    VoxelSet vacated;
    std::vector<float> velocities;
    std::vector<float> progress;
    std::vector<int32_t> kineticEnergies;
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

inline void landVoxel(std::vector<Chunk>& chunks, const VoxelLocation& location){
    Chunk& chunk = chunks[location.chunkIndex];
    chunk.velocities[location.voxelIndex] = 0.0f;
    chunk.progress[location.voxelIndex] = 0.0f;
    int64_t totalEnergy = chunk.energies[location.voxelIndex] + chunk.kineticEnergies[location.voxelIndex];
    assert(totalEnergy >= 0 && "landing made energy go negative.");
    assert(totalEnergy <= std::numeric_limits<uint32_t>::max() && "landing pushed energy past uint32.");            
    chunk.energies[location.voxelIndex] = static_cast<uint32_t>(totalEnergy);
    chunk.kineticEnergies[location.voxelIndex] = 0;
}

inline bool isVacated(const std::vector<Chunk>& chunks, const VoxelLocation& location){
    return chunks[location.chunkIndex].vacated.voxelFlags[location.voxelIndex] == 1;
}

inline GridCoordinate getVoxelCoordinates(const Chunk& chunk, size_t index){
    assert(index < chunk.energies.size() && "Index out of range.");
    return {
        static_cast<int>(index / (static_cast<size_t>(chunk.voxelsPerEdge) * chunk.voxelsPerEdge)), 
        static_cast<int>((index / static_cast<size_t>(chunk.voxelsPerEdge)) % chunk.voxelsPerEdge), 
        static_cast<int>(index % static_cast<size_t>(chunk.voxelsPerEdge))
    };  
}

inline uint64_t getChunkKineticEnergy(const Chunk& chunk){
    int64_t totalKineticEnergy = 0; //Add values into a int64_t that can hold negatives and cast at the end.
    for(int32_t kineticEnergy : chunk.kineticEnergies){
        totalKineticEnergy += kineticEnergy;
    }
    assert(totalKineticEnergy >= 0 && "Chunk has a negative kinetic energy total.");
    return static_cast<uint64_t>(totalKineticEnergy);
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

void moveVoxel(std::vector<Chunk>& chunks, const VoxelLocation& from, const VoxelLocation& to);

void wakeVoxel(VoxelSet& voxelSet, uint32_t voxelIndex);

void computeHeatFlow(std::vector<Chunk>& chunks, size_t chunkIndex, double dt);

void computeFalling(std::vector<Chunk>& chunks, size_t chunkIndex, uint64_t tick, double dt);

void applyHeatFlow(Chunk& chunk);

void updateTemperatures(Chunk& chunk);

uint64_t getChunkEnergy(const Chunk& chunk);

uint64_t getChunkHeightEnergy(const Chunk& chunk);

void fillFaceRenderList(Chunk& chunk);

void initializeChunk(Chunk& chunk, MaterialType material);

VkResult createChunkBuffer(VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, Chunk& chunk);