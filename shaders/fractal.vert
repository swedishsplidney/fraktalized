#version 330 core

layout (location = 0) in vec3 aPos;

out vec2 v_coord;

void main() {
    v_coord = aPos.xy;

    gl_Position = vec4(aPos, 1.0);
}