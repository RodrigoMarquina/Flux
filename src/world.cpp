#include "world.h"

std::vector<Chunk> generateWorld(){
    std::vector<Chunk> chunks;
    chunks.reserve(worldSize * worldSize * worldSize);
    for(int i = 0; i < worldSize; i++){
        for(int j = 0; j < worldSize; j++){
            for(int k = 0; k < worldSize; k++){
                Chunk chunk;
                chunk.size = 10.0f;
                chunk.position = glm::vec3(i, j, k) * chunk.size;
                chunk.voxelsPerEdge = voxelsPerChunk;
                initializeChunk(chunk);
                chunk.chunkPosition.i = i;
                chunk.chunkPosition.j = j;
                chunk.chunkPosition.k = k;
                chunks.push_back(std::move(chunk));
            }
        }
    }

    return chunks;
}
