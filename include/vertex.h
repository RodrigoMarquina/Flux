#pragma once

#include <glm/glm.hpp>
#include <array>
#include <vector>

struct Vertex { //The stride would be 24 bytes per vertex. 3 for each float
    glm::vec3 position;
    glm::vec3 color;
};

struct FaceInstance {
    glm::vec3 origin;
    uint8_t faceDirectionIndex;
};

constexpr int verticesPerFace = 6;
