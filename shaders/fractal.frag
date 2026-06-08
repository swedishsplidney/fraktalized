#version 330 core

out vec4 FragColor;

void main() {
    // normalize pixel coords
    vec2 st = (gl_FragCoord.xy / vec2(1280.0, 720.0)) * 2.0 - 1.0;
    st.x *= 1.770;

    // map coords to complex plane complex boundaries, shifting slightly to the left to center
    vec2 c = st * 1.5 - vec2(0.5, 0.0);
    vec2 z = vec2(0.0, 0.0);

    float iter = 0.0;
    float max_iter = 100.0;

    // fractal loop Z = Z^2 + C
    for (float i = 0.0; i < max_iter; i++) {
        // complex squaring math: real (x^2 - y^2) and imaginary (2*x*y)
        float x_next = z.x * z.x - z.y * z.y + c.x;
        float y_next = 2.0 * z.x * z.y + c.y;

        z = vec2(x_next, y_next);

        // if it escapes radius 2 (diameter 4), it bounds off towards infinity (bad)
        if (dot(z, z) > 4.0) {
            iter = i;
            break;
        }
    }

    // color mapping
    if (iter == max_iter) {
        // points inside set stay black
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        // normalize escape velocity to 0.0 > x > 1.0
        float t = iter / max_iter;

        // procedural color
        vec3 color = vec3(t * 0.4 + 0.1, 0.0, t * 0.8 +0.2);

        // exponentially boost brightness at edges
        color += vec3(pow(t, 4.0) * 0.5);

        FragColor = vec4(color, 1.0); // opaque
    }
}