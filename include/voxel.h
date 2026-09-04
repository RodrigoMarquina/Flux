#pragma once

#include <cstdint>

enum class MaterialType:uint8_t {
    AIR,
    STONE
};

struct StorageVoxel {
    MaterialType material = MaterialType::AIR;
};


