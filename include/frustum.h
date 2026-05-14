#pragma once

#include <glm/glm.hpp>
#include <array>

std::array<glm::vec4, 6> getFrustum(glm::mat4 m);

bool isChunkVisible(std::array<glm::vec4, 6>& planes, glm::vec3 chunkPos, float chunkSize);
