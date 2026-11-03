#pragma once

#include "chunk.h"
#include <vector>

extern std::vector<Chunk> chunks;

void generateWorld();

inline constexpr int worldSize = 3;

inline constexpr int voxelsPerChunk = 10;

inline size_t getChunkIndex(int ci, int cj, int ck){
    assert(ci >= 0 && ci < worldSize && "ci is out of range.");
    assert(cj >= 0 && cj < worldSize && "cj is out of range.");
    assert(ck >= 0 && ck < worldSize && "ck is out of range.");
    return static_cast<size_t>(ci) * worldSize * worldSize + static_cast<size_t>(cj) * worldSize + static_cast<size_t>(ck);
}
