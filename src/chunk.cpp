#include "chunk.h"
#include "voxel.h"
#include "world.h"
#include <limits>
#include <algorithm>
#include <cmath>

void initializeChunk(Chunk& chunk){
    const size_t voxelCount = static_cast<size_t>(chunk.voxelsPerEdge) * chunk.voxelsPerEdge * chunk.voxelsPerEdge;
    chunk.materials.assign(voxelCount, MaterialType::STONE);
    chunk.energies.assign(voxelCount, stoneAt300k);
    chunk.deltaArray.resize(voxelCount);
    chunk.temperatures.resize(voxelCount);
    for(size_t index = 0; index < voxelCount; index++){
        refreshVoxelTemperature(chunk, index);
    }
    chunk.active.voxelFlags.assign(voxelCount, 0);
    chunk.nextActive.voxelFlags.assign(voxelCount, 0);
}

uint64_t getChunkEnergy(const Chunk& chunk){
    uint64_t totalEnergy = 0;
    for(uint64_t energy : chunk.energies){
        totalEnergy += energy;
    }

    return totalEnergy;
}

void wakeVoxel(ActiveSet& activeSet, uint32_t voxelIndex){
    assert(voxelIndex < activeSet.voxelFlags.size() && "Index out of range.");
    if(activeSet.voxelFlags[voxelIndex] != 1){
        activeSet.voxelIndices.push_back(voxelIndex);
        activeSet.voxelFlags[voxelIndex] = 1;
    }
}

void fillFaceRenderList(Chunk& chunk){
    chunk.faceRenderList.clear();
    chunk.faceRenderList.reserve(6 * (chunk.voxelsPerEdge * chunk.voxelsPerEdge));
    static constexpr std::array<GridCoordinate, 6> directions = {{{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}}};
    for(int i = 0; i < chunk.voxelsPerEdge; i++){
        for(int j = 0; j < chunk.voxelsPerEdge; j++){
            for(int k = 0; k < chunk.voxelsPerEdge; k++){
                size_t index = getVoxelIndex(chunk, i, j, k);
                if(chunk.materials[index] != MaterialType::AIR){
                    for(uint8_t d = 0; d < 6; d++){
                        int ni = i + directions[d].i;
                        int nj = j + directions[d].j;
                        int nk = k + directions[d].k;
                        if(ni < 0 || ni >= chunk.voxelsPerEdge || nj < 0 || nj >= chunk.voxelsPerEdge || nk < 0 || nk >= chunk.voxelsPerEdge || 
                            chunk.materials[getVoxelIndex(chunk, ni, nj, nk)] == MaterialType::AIR){
                            FaceInstance faceInstance;
                            faceInstance.origin = chunk.position + glm::vec3(i, j, k) * (chunk.size / chunk.voxelsPerEdge);
                            faceInstance.faceDirectionIndex = d;
                            faceInstance.temperature = static_cast<float>(energyToTemperature(chunk.energies[index], chunk.materials[index]));
                            chunk.faceRenderList.push_back(faceInstance);
                        }
                    }   
                }
            }
        }
    }
}

void computeHeatFlow(std::vector<Chunk>& chunks, size_t chunkIndex, double dt){
    static constexpr std::array<GridCoordinate, 6> directions = {{{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}}};
    Chunk& chunk = chunks[chunkIndex];
    for(uint32_t presentIndex : chunk.active.voxelIndices){
        GridCoordinate voxelCoordinates = getVoxelCoordinates(chunk, presentIndex);
        double presentVoxelTemperature = chunk.temperatures[presentIndex];
        double presentConductivity = materialProperties[static_cast<size_t>(chunk.materials[presentIndex])].conductivity;
        for(uint8_t d = 0; d < 6; d++){
            int ni = voxelCoordinates.i + directions[d].i;
            int nj = voxelCoordinates.j + directions[d].j;
            int nk = voxelCoordinates.k + directions[d].k;
            size_t neighbourChunkIndex = 0;
            size_t neighbourVoxelIndex = 0;
            if(ni >= 0 && ni < chunk.voxelsPerEdge && nj >= 0 && nj < chunk.voxelsPerEdge && nk >= 0 && nk < chunk.voxelsPerEdge){
                neighbourChunkIndex = chunkIndex;
                neighbourVoxelIndex = getVoxelIndex(chunk, ni, nj, nk);
            } else{
                GridCoordinate neighbourChunkCoordinate;
                neighbourChunkCoordinate.i = chunk.chunkPosition.i + directions[d].i;
                neighbourChunkCoordinate.j = chunk.chunkPosition.j + directions[d].j;
                neighbourChunkCoordinate.k = chunk.chunkPosition.k + directions[d].k;
                if(neighbourChunkCoordinate.i < 0 || neighbourChunkCoordinate.i >= worldSize || 
                   neighbourChunkCoordinate.j < 0 || neighbourChunkCoordinate.j >= worldSize || 
                   neighbourChunkCoordinate.k < 0 || neighbourChunkCoordinate.k >= worldSize){
                    continue;
                }
                neighbourChunkIndex = getChunkIndex(neighbourChunkCoordinate.i, neighbourChunkCoordinate.j, neighbourChunkCoordinate.k);
                if(ni == chunk.voxelsPerEdge){
                    ni = 0;
                } else if(ni == -1){
                    ni = chunk.voxelsPerEdge - 1;
                }
                if(nj == chunk.voxelsPerEdge){
                    nj = 0;
                } else if(nj == -1){
                    nj = chunk.voxelsPerEdge - 1;
                }
                if(nk == chunk.voxelsPerEdge){
                    nk = 0;
                } else if(nk == -1){
                    nk = chunk.voxelsPerEdge - 1;
                }
                neighbourVoxelIndex = getVoxelIndex(chunks[neighbourChunkIndex], ni, nj, nk);
            }
            Chunk& neighbourChunk = chunks[neighbourChunkIndex];
            if(directions[d].i + directions[d].j + directions[d].k < 0 && neighbourChunk.active.voxelFlags[neighbourVoxelIndex] == 1){
                continue;
            }
            double neighbourVoxelTemperature = neighbourChunk.temperatures[neighbourVoxelIndex];
            double temperatureDifference = presentVoxelTemperature - neighbourVoxelTemperature;
            if(std::fabs(temperatureDifference) < sleepThreshold){
                continue;
            }
            double neighbourConductivity = materialProperties[static_cast<size_t>(neighbourChunk.materials[neighbourVoxelIndex])].conductivity;
            double conductivity = 2.0 * (presentConductivity * neighbourConductivity) / (presentConductivity + neighbourConductivity);
            double flowInWatts = conductivity * voxelEdgeLength * temperatureDifference;
            int64_t flowInUnits = static_cast<int64_t>(flowInWatts * dt / joulesPerEnergyUnit);
            chunk.deltaArray[presentIndex] -= flowInUnits;
            neighbourChunk.deltaArray[neighbourVoxelIndex] += flowInUnits;
            wakeVoxel(chunk.nextActive, presentIndex);
            wakeVoxel(neighbourChunk.nextActive, static_cast<uint32_t>(neighbourVoxelIndex));
        } 
    }          
}

void applyHeatFlow(Chunk& chunk){
    for(uint32_t index : chunk.nextActive.voxelIndices){
        int64_t newEnergy = chunk.energies[index] + chunk.deltaArray[index];
        assert(newEnergy >= 0 && "energy went negative, a flow took more than the voxel had.");
        assert(newEnergy <= std::numeric_limits<uint32_t>::max() && "energy overflowed, uint32, a voxel received more than it can hold.");
        chunk.energies[index] = static_cast<uint32_t>(newEnergy);
        chunk.deltaArray[index] = 0;
    }
    for(uint32_t index : chunk.active.voxelIndices){
        chunk.active.voxelFlags[index] = 0;
    }
    std::swap(chunk.active, chunk.nextActive);
    chunk.nextActive.voxelIndices.clear();
}

void updateTemperatures(Chunk& chunk){
    for(uint32_t index : chunk.active.voxelIndices){
        refreshVoxelTemperature(chunk, index);
    }
}

VkResult createChunkBuffer(VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, Chunk& chunk){
    VkBufferCreateInfo bufferCreateInfo {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = sizeof(FaceInstance) * chunk.faceRenderList.size();
    bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult createBufferResult = vkCreateBuffer(*logicalDevice, &bufferCreateInfo, nullptr, &chunk.chunkBuffer);
    if(createBufferResult != VK_SUCCESS){
        return createBufferResult;
    }
    
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties {};
    vkGetPhysicalDeviceMemoryProperties(*physicalDevice, &physicalDeviceMemoryProperties);

    VkMemoryRequirements memoryRequirements {}; 
    vkGetBufferMemoryRequirements(*logicalDevice, chunk.chunkBuffer, &memoryRequirements);

    uint32_t memoryTypeIndex = 0;
    while(!((physicalDeviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && (physicalDeviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) && (memoryRequirements.memoryTypeBits & (1 << memoryTypeIndex)))){
        memoryTypeIndex++;
    }

    VkMemoryAllocateInfo memoryAllocateInfo {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
    VkResult allocateMemoryResult = vkAllocateMemory(*logicalDevice, &memoryAllocateInfo, nullptr, &chunk.chunkMemory);
    if(allocateMemoryResult != VK_SUCCESS){
        return allocateMemoryResult;
    }

    VkResult bindBufferMemoryResult = vkBindBufferMemory(*logicalDevice, chunk.chunkBuffer, chunk.chunkMemory, 0);
    if(bindBufferMemoryResult != VK_SUCCESS){
        return bindBufferMemoryResult;
    }

    VkResult mapMemoryResult = vkMapMemory(*logicalDevice, chunk.chunkMemory, 0, memoryRequirements.size, 0, &chunk.memoryMap);
    if(mapMemoryResult != VK_SUCCESS){
        return mapMemoryResult;
    }

    return bindBufferMemoryResult;
}