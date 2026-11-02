#include "chunk.h"
#include "voxel.h"
#include <limits>

void initializeChunk(Chunk& chunk){
    chunk.materials.reserve(chunk.voxelsPerEdge * chunk.voxelsPerEdge * chunk.voxelsPerEdge);
    chunk.energies.reserve(chunk.voxelsPerEdge * chunk.voxelsPerEdge * chunk.voxelsPerEdge);
    for(int i = 0; i < chunk.voxelsPerEdge; i++){
        for(int j = 0; j < chunk.voxelsPerEdge; j++){
            for(int k = 0; k < chunk.voxelsPerEdge; k++){
                chunk.materials.push_back(MaterialType::STONE);
                chunk.energies.push_back(65000000);
            }
        }
    }
}

uint64_t getChunkEnergy(const Chunk& chunk){
    uint64_t totalEnergy = 0;
    for(uint64_t energy : chunk.energies){
        totalEnergy += energy;
    }

    return totalEnergy;
}

struct Direction {
    int di;
    int dj;
    int dk;
};

void fillFaceRenderList(Chunk& chunk){
    chunk.faceRenderList.clear();
    chunk.faceRenderList.reserve(6 * (chunk.voxelsPerEdge * chunk.voxelsPerEdge));
    static constexpr std::array<Direction, 6> directions = {{{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}}};
    for(int i = 0; i < chunk.voxelsPerEdge; i++){
        for(int j = 0; j < chunk.voxelsPerEdge; j++){
            for(int k = 0; k < chunk.voxelsPerEdge; k++){
                size_t index = getIndex(chunk, i, j, k);
                if(chunk.materials[index] != MaterialType::AIR){
                    for(uint8_t d = 0; d < 6; d++){
                        int ni = i + directions[d].di;
                        int nj = j + directions[d].dj;
                        int nk = k + directions[d].dk;
                        if(ni < 0 || ni >= chunk.voxelsPerEdge || nj < 0 || nj >= chunk.voxelsPerEdge || nk < 0 || nk >= chunk.voxelsPerEdge || 
                            chunk.materials[getIndex(chunk, ni, nj, nk)] == MaterialType::AIR){
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

void conductHeat(Chunk& chunk, double dt){
    std::vector<int64_t> deltaArray (chunk.energies.size()); 
    static constexpr std::array<Direction, 3> directions = {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
    for(int i = 0; i < chunk.voxelsPerEdge; i++){
        for(int j = 0; j < chunk.voxelsPerEdge; j++){
            for(int k = 0; k < chunk.voxelsPerEdge; k++){
                size_t presentIndex = getIndex(chunk, i, j, k);
                double presentVoxelTemperature = energyToTemperature(chunk.energies[presentIndex], chunk.materials[presentIndex]);
                double presentConductivity = materialProperties[static_cast<size_t>(chunk.materials[presentIndex])].conductivity;
                for(uint8_t d = 0; d < 3; d++){
                    int ni = i + directions[d].di;
                    int nj = j + directions[d].dj;
                    int nk = k + directions[d].dk;
                    if(ni < chunk.voxelsPerEdge && nj < chunk.voxelsPerEdge && nk < chunk.voxelsPerEdge){
                        size_t neighbourIndex = getIndex(chunk, ni, nj, nk);
                        double neighbourVoxelTemperature = energyToTemperature(chunk.energies[neighbourIndex], chunk.materials[neighbourIndex]);
                        double neighbourConductivity = materialProperties[static_cast<size_t>(chunk.materials[neighbourIndex])].conductivity;
                        double conductivity = 2 * (presentConductivity * neighbourConductivity) / (presentConductivity + neighbourConductivity);
                        double flowInWatts = conductivity * voxelEdgeLength * (presentVoxelTemperature - neighbourVoxelTemperature);
                        int64_t flowInUnits = static_cast<int64_t>(flowInWatts * dt / joulesPerEnergyUnit);
                        deltaArray[presentIndex] -= flowInUnits;
                        deltaArray[neighbourIndex] += flowInUnits;
                    }
                }
            }
        } 
    }          
    for(size_t i = 0; i < chunk.energies.size(); i++){
        int64_t newEnergy = chunk.energies[i] + deltaArray[i];
        assert(newEnergy >= 0 && "energy went negative, a flow took more than the voxel had.");
        assert(newEnergy <= std::numeric_limits<uint32_t>::max() && "energy overflowed, energy overflowed uint32, a voxel received more than it can hold.");
        chunk.energies[i] = static_cast<uint32_t>(newEnergy);
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