#pragma once

#include <glm/glm.hpp>
#include <array>

struct Vertex //The stride would be 24 bytes per vertex. 3 for each float
{
    glm::vec3 position;
    glm::vec3 color;
};

extern std::array<Vertex, 8> cube; //Extern tells the compiler that the variable exists in another translation unit

extern std::array<uint16_t, 36> indexArray;