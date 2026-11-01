#include "chunk.h"
#include "voxel.h"

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