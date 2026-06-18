#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vertexColor;
out vec3 localPos;
out vec3 worldPos;

// holy trinity
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
    vertexColor = aColor;
    localPos = aPos;

    // calc world position
    vec4 worldPos4 = u_model * vec4(aPos, 1.0);
    worldPos = worldPos4.xyz;

    // final screen projection
    gl_Position = u_projection * u_view * worldPos4;
}