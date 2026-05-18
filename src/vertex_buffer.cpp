#include "vertex_buffer.h"
#include "vertex.h"

VkResult createVertexBuffer(VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, VkBuffer* vertexBuffer, VkDeviceMemory* vertexDeviceMemory){
    VkBufferCreateInfo bufferCreateInfo {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = sizeof(voxel);
    bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult createBufferResult = vkCreateBuffer(*logicalDevice, &bufferCreateInfo, nullptr, vertexBuffer);
    if(createBufferResult != VK_SUCCESS){
        return createBufferResult;
    }
    
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties {};
    vkGetPhysicalDeviceMemoryProperties(*physicalDevice, &physicalDeviceMemoryProperties);

    VkMemoryRequirements memoryRequirements {}; 
    vkGetBufferMemoryRequirements(*logicalDevice, *vertexBuffer, &memoryRequirements);

    uint32_t memoryTypeIndex = 0;
    while(!((physicalDeviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && (physicalDeviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) && (memoryRequirements.memoryTypeBits & (1 << memoryTypeIndex)))){
        memoryTypeIndex++;
    }

    VkMemoryAllocateInfo memoryAllocateInfo {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
    VkResult allocateMemoryResult = vkAllocateMemory(*logicalDevice, &memoryAllocateInfo, nullptr, vertexDeviceMemory);
    if(allocateMemoryResult != VK_SUCCESS){
        return allocateMemoryResult;
    }

    VkResult bindBufferMemoryResult = vkBindBufferMemory(*logicalDevice, *vertexBuffer, *vertexDeviceMemory, 0);
    if(bindBufferMemoryResult != VK_SUCCESS){
        return bindBufferMemoryResult;
    }

    void* memoryMap;
    VkResult mapMemoryResult = vkMapMemory(*logicalDevice, *vertexDeviceMemory, 0, memoryRequirements.size, 0, &memoryMap);
    if(mapMemoryResult != VK_SUCCESS){
        return mapMemoryResult;
    }
    memcpy(memoryMap, &voxel, sizeof(voxel));
    vkUnmapMemory(*logicalDevice, *vertexDeviceMemory);

    return bindBufferMemoryResult;
}

VkResult createIndexBuffer(VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, VkBuffer* indexBuffer, VkDeviceMemory* indexDeviceMemory){
    VkBufferCreateInfo indexBufferCreateInfo {};
    indexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    indexBufferCreateInfo.size = sizeof(indexArray);
    indexBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    indexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult createIndexBufferResult = vkCreateBuffer(*logicalDevice, &indexBufferCreateInfo, nullptr, indexBuffer);
    if(createIndexBufferResult != VK_SUCCESS){
        return createIndexBufferResult;
    }
    
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties {};
    vkGetPhysicalDeviceMemoryProperties(*physicalDevice, &physicalDeviceMemoryProperties);

    VkMemoryRequirements memoryRequirements {}; 
    vkGetBufferMemoryRequirements(*logicalDevice, *indexBuffer, &memoryRequirements);

    uint32_t memoryTypeIndex = 0;
    while(!((physicalDeviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && (physicalDeviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) && (memoryRequirements.memoryTypeBits & (1 << memoryTypeIndex)))){
        memoryTypeIndex++;
    }

    VkMemoryAllocateInfo memoryAllocateInfo {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
    VkResult allocateMemoryResult = vkAllocateMemory(*logicalDevice, &memoryAllocateInfo, nullptr, indexDeviceMemory);
    if(allocateMemoryResult != VK_SUCCESS){
        return allocateMemoryResult;
    }

    VkResult bindBufferMemoryResult = vkBindBufferMemory(*logicalDevice, *indexBuffer, *indexDeviceMemory, 0);
    if(bindBufferMemoryResult != VK_SUCCESS){
        return bindBufferMemoryResult;
    }

    void* memoryMap;
    VkResult mapMemoryResult = vkMapMemory(*logicalDevice, *indexDeviceMemory, 0, memoryRequirements.size, 0, &memoryMap);
    if(mapMemoryResult != VK_SUCCESS){
        return mapMemoryResult;
    }
    memcpy(memoryMap, &indexArray, sizeof(indexArray));
    vkUnmapMemory(*logicalDevice, *indexDeviceMemory);

    return bindBufferMemoryResult;
}

VkResult createInstanceBuffer(VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, VkBuffer* instanceBuffer, VkDeviceMemory* instanceDeviceMemory, void** memoryMap){
    VkBufferCreateInfo bufferCreateInfo {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = sizeof(Voxel) * voxels.size();
    bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult createBufferResult = vkCreateBuffer(*logicalDevice, &bufferCreateInfo, nullptr, instanceBuffer);
    if(createBufferResult != VK_SUCCESS){
        return createBufferResult;
    }
    
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties {};
    vkGetPhysicalDeviceMemoryProperties(*physicalDevice, &physicalDeviceMemoryProperties);

    VkMemoryRequirements memoryRequirements {}; 
    vkGetBufferMemoryRequirements(*logicalDevice, *instanceBuffer, &memoryRequirements);

    uint32_t memoryTypeIndex = 0;
    while(!((physicalDeviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && (physicalDeviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) && (memoryRequirements.memoryTypeBits & (1 << memoryTypeIndex)))){
        memoryTypeIndex++;
    }

    VkMemoryAllocateInfo memoryAllocateInfo {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
    VkResult allocateMemoryResult = vkAllocateMemory(*logicalDevice, &memoryAllocateInfo, nullptr, instanceDeviceMemory);
    if(allocateMemoryResult != VK_SUCCESS){
        return allocateMemoryResult;
    }

    VkResult bindBufferMemoryResult = vkBindBufferMemory(*logicalDevice, *instanceBuffer, *instanceDeviceMemory, 0);
    if(bindBufferMemoryResult != VK_SUCCESS){
        return bindBufferMemoryResult;
    }

    VkResult mapMemoryResult = vkMapMemory(*logicalDevice, *instanceDeviceMemory, 0, memoryRequirements.size, 0, memoryMap);
    if(mapMemoryResult != VK_SUCCESS){
        return mapMemoryResult;
    }

    return bindBufferMemoryResult;
}
