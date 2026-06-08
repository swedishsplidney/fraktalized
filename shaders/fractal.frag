#version 330 core

out vec4 FragColor;

void main() {
    // test: output gradient
    vec2 st = gl_FragCoord.xy / vec2(1280.0, 270.0);

    FragColor = vec4(st.x, 0.8, st.y, 1.0);
}