#include "frustum.h"

std::array<glm::vec4, 6> getFrustum(glm::mat4 m){
    glm::vec4 leftPlane = glm::vec4( //Left plane
        m[0][3] + m[0][0],
        m[1][3] + m[1][0],
        m[2][3] + m[2][0],
        m[3][3] + m[3][0]
    );
    leftPlane /= glm::length(glm::vec3(leftPlane));

    glm::vec4 rightPlane = glm::vec4( //Right plane
        m[0][3] - m[0][0],
        m[1][3] - m[1][0],
        m[2][3] - m[2][0],
        m[3][3] - m[3][0]
    );
    rightPlane /= glm::length(glm::vec3(rightPlane));

    glm::vec4 bottomPlane = glm::vec4( //Bottom plane
        m[0][3] + m[0][1],
        m[1][3] + m[1][1],
        m[2][3] + m[2][1],
        m[3][3] + m[3][1]
    );
    bottomPlane /= glm::length(glm::vec3(bottomPlane));

    glm::vec4 topPlane = glm::vec4( //Top plane
        m[0][3] - m[0][1],
        m[1][3] - m[1][1],
        m[2][3] - m[2][1],
        m[3][3] - m[3][1]
    );
    topPlane /= glm::length(glm::vec3(topPlane));

    glm::vec4 nearPlane = glm::vec4( //Near plane
        m[0][3] + m[0][2],
        m[1][3] + m[1][2],
        m[2][3] + m[2][2],
        m[3][3] + m[3][2]
    );
    nearPlane /= glm::length(glm::vec3(nearPlane));

    glm::vec4 farPlane = glm::vec4( //Far plane
        m[0][3] - m[0][2],
        m[1][3] - m[1][2],
        m[2][3] - m[2][2],
        m[3][3] - m[3][2]
    );
    farPlane /= glm::length(glm::vec3(farPlane));

    return {leftPlane, rightPlane, bottomPlane, topPlane, nearPlane, farPlane};
}

bool isChunkVisible(std::array<glm::vec4, 6>& planes, glm::vec3 chunkPos, float chunkSize){
    for(int plane = 0; plane < 6; plane++){
        bool allOutside = true;
        for(int x = 0; x <= 1; x++){
            for(int y = 0; y <= 1; y++){
                for(int z = 0; z <= 1; z++){
                    glm::vec3 corner = chunkPos + glm::vec3(x, y, z) * chunkSize;
                    if(planes[plane].x * corner.x + planes[plane].y * corner.y + planes[plane].z * corner.z + planes[plane].w >= -0.5f){
                        allOutside = false;
                    }
                }
            }
        }
        if(allOutside){
            return false;
        }
    }
    return true;
}