#version 450

layout(location = 0) in vec3 inOrigin;
layout(location = 1) in uint inFaceDirectionIndex;
layout(location = 0) out vec3 outColor;

const vec3 faceColors[6] = vec3[](
    vec3(1.0, 1.0, 1.0),
    vec3(0.0, 1.0, 1.0),
    vec3(1.0, 0.0, 1.0),
    vec3(1.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 1.0, 0.0)
);

const vec3 positions[36] = vec3[](
    vec3( 0.5, -0.5,  0.5),
    vec3( 0.5,  0.5,  0.5),
    vec3( 0.5,  0.5, -0.5),
    vec3( 0.5, -0.5,  0.5),
    vec3( 0.5,  0.5, -0.5),
    vec3( 0.5, -0.5, -0.5),
    vec3(-0.5, -0.5, -0.5),
    vec3(-0.5,  0.5, -0.5),
    vec3(-0.5,  0.5,  0.5),
    vec3(-0.5, -0.5, -0.5),
    vec3(-0.5,  0.5,  0.5),
    vec3(-0.5, -0.5,  0.5),
    vec3(-0.5,  0.5, -0.5),
    vec3( 0.5,  0.5, -0.5),
    vec3( 0.5,  0.5,  0.5),
    vec3(-0.5,  0.5, -0.5),
    vec3( 0.5,  0.5,  0.5),
    vec3(-0.5,  0.5,  0.5),
    vec3(-0.5, -0.5,  0.5),
    vec3( 0.5, -0.5,  0.5),
    vec3( 0.5, -0.5, -0.5),
    vec3(-0.5, -0.5,  0.5),
    vec3( 0.5, -0.5, -0.5),
    vec3(-0.5, -0.5, -0.5),
    vec3(-0.5, -0.5,  0.5),
    vec3(-0.5,  0.5,  0.5),
    vec3( 0.5,  0.5,  0.5),
    vec3(-0.5, -0.5,  0.5),
    vec3( 0.5,  0.5,  0.5),
    vec3( 0.5, -0.5,  0.5),
    vec3( 0.5, -0.5, -0.5),
    vec3( 0.5,  0.5, -0.5),
    vec3(-0.5,  0.5, -0.5),
    vec3( 0.5, -0.5, -0.5),
    vec3(-0.5,  0.5, -0.5),
    vec3(-0.5, -0.5, -0.5)
);

layout(binding = 0) uniform UniformBuffer {
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

void main(){
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(positions[inFaceDirectionIndex * 6 + gl_VertexIndex] + inOrigin, 1.0);
    outColor = faceColors[inFaceDirectionIndex];
}
