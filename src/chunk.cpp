#include "chunk.h"
#include "voxel.h"
#include "world.h"
#include <limits>
#include <algorithm>
#include <cmath>

static constexpr GridCoordinate downDirection = {0, 1, 0};
static constexpr GridCoordinate upDirection = {0, -1, 0};
static constexpr std::array<GridCoordinate, 10> wakeDirections = {{{0, -1, 0}, {0, 1, 0}, {1, 0, 0}, {-1, 0, 0}, {0, 0, 1}, {0, 0, -1}, {1, -1, 0}, {-1, -1, 0}, {0, -1, 1}, {0, -1, -1}}};
static constexpr std::array<GridCoordinate, 4> sideDirections = {{{1, 0, 0}, {-1, 0, 0}, {0, 0, 1}, {0, 0, -1}}};
static constexpr std::array<GridCoordinate, 6> faceDirections = {{{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}}};

void initializeChunk(Chunk& chunk, MaterialType material){
    const size_t voxelCount = static_cast<size_t>(chunk.voxelsPerEdge) * chunk.voxelsPerEdge * chunk.voxelsPerEdge;
    chunk.materials.assign(voxelCount, material);
    chunk.energies.assign(voxelCount, temperatureToEnergy(initialTemperature, material));
    chunk.deltaArray.resize(voxelCount);
    chunk.temperatures.resize(voxelCount);
    for(size_t index = 0; index < voxelCount; index++){
        refreshVoxelTemperature(chunk, index);
    }
    chunk.active.voxelFlags.assign(voxelCount, 0);
    chunk.nextActive.voxelFlags.assign(voxelCount, 0);
    chunk.vacated.voxelFlags.assign(voxelCount, 0);
    chunk.velocities.resize(voxelCount);
    chunk.progress.resize(voxelCount);
    chunk.kineticEnergies.resize(voxelCount);
}

int getCellHeight(const Chunk& chunk, size_t voxelIndex){
    GridCoordinate voxelCoordinates = getVoxelCoordinates(chunk, voxelIndex);
    const int j = voxelCoordinates.j;
    const int globalJ = chunk.chunkPosition.j * chunk.voxelsPerEdge + j;
    const int height = (worldSize * voxelsPerChunk - 1) - globalJ;
    assert(height >= 0 && "Height is negative.");
    return height;
}

uint64_t getChunkEnergy(const Chunk& chunk){
    uint64_t totalEnergy = 0;
    for(uint64_t energy : chunk.energies){
        totalEnergy += energy;
    }

    return totalEnergy;
}

uint64_t getChunkHeightEnergy(const Chunk& chunk){
    uint64_t totalEnergy = 0;
    for(size_t index = 0; index < chunk.materials.size(); index++){
        totalEnergy += static_cast<uint64_t>(heightEnergy(chunk.materials[index], getCellHeight(chunk, index)));
    }

    return totalEnergy;
}

void wakeVoxel(VoxelSet& voxelSet, uint32_t voxelIndex){
    assert(voxelIndex < voxelSet.voxelFlags.size() && "Index out of range.");
    if(voxelSet.voxelFlags[voxelIndex] != 1){
        voxelSet.voxelIndices.push_back(voxelIndex);
        voxelSet.voxelFlags[voxelIndex] = 1;
    }
}

void fillFaceRenderList(Chunk& chunk){
    chunk.faceRenderList.clear();
    chunk.faceRenderList.reserve(6 * (chunk.voxelsPerEdge * chunk.voxelsPerEdge));
    const float voxelSize = chunk.size / static_cast<float>(chunk.voxelsPerEdge);
    for(int i = 0; i < chunk.voxelsPerEdge; i++){
        for(int j = 0; j < chunk.voxelsPerEdge; j++){
            for(int k = 0; k < chunk.voxelsPerEdge; k++){
                size_t index = getVoxelIndex(chunk, i, j, k);
                if(chunk.materials[index] != MaterialType::AIR){
                    for(uint8_t d = 0; d < 6; d++){
                        int ni = i + faceDirections[d].i;
                        int nj = j + faceDirections[d].j;
                        int nk = k + faceDirections[d].k;
                        if(ni < 0 || ni >= chunk.voxelsPerEdge || nj < 0 || nj >= chunk.voxelsPerEdge || nk < 0 || nk >= chunk.voxelsPerEdge || 
                            chunk.materials[getVoxelIndex(chunk, ni, nj, nk)] == MaterialType::AIR){
                            FaceInstance faceInstance;
                            faceInstance.origin = chunk.position + glm::vec3(i, j, k) * voxelSize;
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

std::optional<VoxelLocation> findNeighbour(const std::vector<Chunk>& chunks, size_t chunkIndex, GridCoordinate voxelCoordinates, GridCoordinate direction){
    const Chunk& chunk = chunks[chunkIndex];
    assert(direction.i >= -1 && direction.i <= 1 && "Direction i must be -1, 0 or 1.");
    assert(direction.j >= -1 && direction.j <= 1 && "Direction j must be -1, 0 or 1.");
    assert(direction.k >= -1 && direction.k <= 1 && "Direction k must be -1, 0 or 1.");    
    int ni = voxelCoordinates.i + direction.i;
    int nj = voxelCoordinates.j + direction.j;
    int nk = voxelCoordinates.k + direction.k;
    VoxelLocation location;
    if(ni >= 0 && ni < chunk.voxelsPerEdge && nj >= 0 && nj < chunk.voxelsPerEdge && nk >= 0 && nk < chunk.voxelsPerEdge){
        location.chunkIndex = chunkIndex;
        location.voxelIndex = getVoxelIndex(chunk, ni, nj, nk);
        return location;
    } else{
        GridCoordinate chunkStep;
        if(ni == chunk.voxelsPerEdge){
            ni = 0;
            chunkStep.i = 1;
        } else if(ni == -1){
            ni = chunk.voxelsPerEdge - 1;
            chunkStep.i = -1;
        }
        if(nj == chunk.voxelsPerEdge){
            nj = 0;
            chunkStep.j = 1;
        } else if(nj == -1){
            nj = chunk.voxelsPerEdge - 1;
            chunkStep.j = -1;
        }
        if(nk == chunk.voxelsPerEdge){
            nk = 0;
            chunkStep.k = 1;
        } else if(nk == -1){
            nk = chunk.voxelsPerEdge - 1;
            chunkStep.k = -1;
        }
        GridCoordinate neighbourChunkCoordinate;
        neighbourChunkCoordinate.i = chunk.chunkPosition.i + chunkStep.i;
        neighbourChunkCoordinate.j = chunk.chunkPosition.j + chunkStep.j;
        neighbourChunkCoordinate.k = chunk.chunkPosition.k + chunkStep.k;
        if(neighbourChunkCoordinate.i < 0 || neighbourChunkCoordinate.i >= worldSize || 
            neighbourChunkCoordinate.j < 0 || neighbourChunkCoordinate.j >= worldSize || 
            neighbourChunkCoordinate.k < 0 || neighbourChunkCoordinate.k >= worldSize){
            return std::nullopt;
        }
        location.chunkIndex = getChunkIndex(neighbourChunkCoordinate.i, neighbourChunkCoordinate.j, neighbourChunkCoordinate.k);
        location.voxelIndex = getVoxelIndex(chunks[location.chunkIndex], ni, nj, nk);
        return location;
    }
}

void moveVoxel(std::vector<Chunk>& chunks, const VoxelLocation& from, const VoxelLocation& to){
    Chunk& fromChunk = chunks[from.chunkIndex];
    Chunk& toChunk = chunks[to.chunkIndex];
    int fromHeight = getCellHeight(fromChunk, from.voxelIndex);
    int toHeight = getCellHeight(toChunk, to.voxelIndex);
    int64_t before = heightEnergy(fromChunk.materials[from.voxelIndex], fromHeight) + heightEnergy(toChunk.materials[to.voxelIndex], toHeight); 
    std::swap(fromChunk.materials[from.voxelIndex], toChunk.materials[to.voxelIndex]);
    std::swap(fromChunk.energies[from.voxelIndex], toChunk.energies[to.voxelIndex]);
    std::swap(fromChunk.temperatures[from.voxelIndex], toChunk.temperatures[to.voxelIndex]);
    std::swap(fromChunk.velocities[from.voxelIndex], toChunk.velocities[to.voxelIndex]);
    std::swap(fromChunk.progress[from.voxelIndex], toChunk.progress[to.voxelIndex]);
    std::swap(fromChunk.kineticEnergies[from.voxelIndex], toChunk.kineticEnergies[to.voxelIndex]);
    int64_t after = heightEnergy(fromChunk.materials[from.voxelIndex], fromHeight) + heightEnergy(toChunk.materials[to.voxelIndex], toHeight); 
    toChunk.kineticEnergies[to.voxelIndex] += static_cast<int32_t>(before - after);
    wakeVoxel(fromChunk.vacated, static_cast<uint32_t>(from.voxelIndex));
    wakeVoxel(toChunk.nextActive, static_cast<uint32_t>(to.voxelIndex));
    double emptyCellDensity = materialProperties[static_cast<size_t>(fromChunk.materials[from.voxelIndex])].density;
    for(const GridCoordinate& direction : wakeDirections){ 
        std::optional<VoxelLocation> nearby = findNeighbour(chunks, from.chunkIndex, getVoxelCoordinates(fromChunk, from.voxelIndex), direction);
        if(!nearby){
            continue;
        }
        double nearbyDensity = materialProperties[static_cast<size_t>(chunks[nearby->chunkIndex].materials[nearby->voxelIndex])].density;
        if(nearbyDensity > emptyCellDensity){
            wakeVoxel(chunks[nearby->chunkIndex].nextActive, static_cast<uint32_t>(nearby->voxelIndex));
        }
    }
}

void computeHeatFlow(std::vector<Chunk>& chunks, size_t chunkIndex, double dt){
    Chunk& chunk = chunks[chunkIndex];
    for(uint32_t presentIndex : chunk.active.voxelIndices){
        GridCoordinate voxelCoordinates = getVoxelCoordinates(chunk, presentIndex);
        double presentVoxelTemperature = chunk.temperatures[presentIndex];
        double presentConductivity = materialProperties[static_cast<size_t>(chunk.materials[presentIndex])].conductivity;
        for(uint8_t d = 0; d < 6; d++){
            std::optional<VoxelLocation> location = findNeighbour(chunks, chunkIndex, voxelCoordinates, faceDirections[d]);
            if(!location){ //If location has no value
                continue;
            }
            size_t neighbourChunkIndex = location->chunkIndex;
            size_t neighbourVoxelIndex = location->voxelIndex;
            Chunk& neighbourChunk = chunks[neighbourChunkIndex];
            if(faceDirections[d].i + faceDirections[d].j + faceDirections[d].k < 0 && neighbourChunk.active.voxelFlags[neighbourVoxelIndex] == 1){
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

void computeFalling(std::vector<Chunk>& chunks, size_t chunkIndex, uint64_t tick, double dt){
    Chunk& chunk = chunks[chunkIndex];
    for(uint32_t presentIndex : chunk.active.voxelIndices){
        GridCoordinate voxelCoordinates = getVoxelCoordinates(chunk, presentIndex);
        double presentVoxelDensity = materialProperties[static_cast<size_t>(chunk.materials[presentIndex])].density;
        std::optional<VoxelLocation> target = findNeighbour(chunks, chunkIndex, voxelCoordinates, downDirection);
        if(!target){ //If target has no value
            continue;
        }
        if(isVacated(chunks, {chunkIndex, presentIndex})){
            continue;
        }
        double downDensity = materialProperties[static_cast<size_t>(chunks[target->chunkIndex].materials[target->voxelIndex])].density;
        if(presentVoxelDensity <= downDensity){ //Below blocked
            landVoxel(chunks, {chunkIndex, presentIndex});
            target = std::nullopt;
            std::optional<VoxelLocation> above = findNeighbour(chunks, chunkIndex, voxelCoordinates, upDirection);
            if(above && (materialProperties[static_cast<size_t>(chunks[above->chunkIndex].materials[above->voxelIndex])].density >= presentVoxelDensity
                || isVacated(chunks, *above))){
                continue;
            }
            size_t start = (tick + presentIndex) % sideDirections.size();
            for(size_t n = 0; n < sideDirections.size(); n++){
                GridCoordinate direction = sideDirections[(start + n) % sideDirections.size()];
                std::optional<VoxelLocation> side = findNeighbour(chunks, chunkIndex, voxelCoordinates, direction);
                std::optional<VoxelLocation> below = findNeighbour(chunks, chunkIndex, voxelCoordinates, {direction.i, 1, direction.k});
                if(!side || !below){
                    continue;
                }
                if(isVacated(chunks, *side) || isVacated(chunks, *below)){
                    continue;
                }
                double sideVoxelDensity = materialProperties[static_cast<size_t>(chunks[side->chunkIndex].materials[side->voxelIndex])].density;
                double belowVoxelDensity = materialProperties[static_cast<size_t>(chunks[below->chunkIndex].materials[below->voxelIndex])].density;
                if(sideVoxelDensity < presentVoxelDensity && belowVoxelDensity < presentVoxelDensity){
                    target = side;
                    break;
                }
            }
        } else{
            float velocity = chunk.velocities[presentIndex];
            float progress = chunk.progress[presentIndex];               
            velocity += static_cast<float>(gravity * dt);
            progress += static_cast<float>(velocity * dt / voxelEdgeLength);
            if(progress < 1){
                chunk.velocities[presentIndex] = velocity;
                chunk.progress[presentIndex] = progress;
                wakeVoxel(chunk.nextActive, static_cast<uint32_t>(presentIndex)); //Vote for itself
                continue;
            } else{
                VoxelLocation currentLocation = {chunkIndex, presentIndex};
                GridCoordinate currentCoordinates = getVoxelCoordinates(chunks[chunkIndex], presentIndex);
                while(progress >= 1){
                    std::optional<VoxelLocation> below = findNeighbour(chunks, currentLocation.chunkIndex, currentCoordinates, downDirection);
                    if(!below){
                        landVoxel(chunks, currentLocation);
                        break;
                    }
                    downDensity = materialProperties[static_cast<size_t>(chunks[below->chunkIndex].materials[below->voxelIndex])].density;
                    if(presentVoxelDensity <= downDensity || isVacated(chunks, *below)){
                        break;
                    }
                    moveVoxel(chunks, currentLocation, *below);
                    currentLocation = *below;
                    currentCoordinates = getVoxelCoordinates(chunks[currentLocation.chunkIndex], currentLocation.voxelIndex);
                    progress -= 1.0f;
                }
                chunks[currentLocation.chunkIndex].velocities[currentLocation.voxelIndex] = velocity;
                chunks[currentLocation.chunkIndex].progress[currentLocation.voxelIndex] = progress;
                continue;
            }
        }
        if(!target){
            continue;
        }
        moveVoxel(chunks, {chunkIndex, presentIndex}, *target);
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
    bufferCreateInfo.size = sizeof(FaceInstance) * maxFacesPerChunk;
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