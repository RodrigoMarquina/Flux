#include "world.h"

std::vector<Chunk> chunks;

int worldSize = 3;
int voxelsPerChunk = 10;

void generateWorld(){
    for(int i = 0; i < worldSize; i++){
        for(int j = 0; j < worldSize; j++){
            for(int k = 0; k < worldSize; k++){
                Chunk chunk;
                chunk.size = 10.0f;
                chunk.position = glm::vec3(i, j, k) * chunk.size;
                chunk.voxelsPerEdge = voxelsPerChunk;
                initializeChunk(chunk);
                chunks.push_back(chunk);
            }
        }
    }
}
