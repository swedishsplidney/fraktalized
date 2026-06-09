#version 330 core

out vec4 FragColor;

// dynamic inputs
uniform vec2 u_resolution;
uniform vec2 u_zoom_center;
uniform float u_zoom_level;
uniform float u_max_iter;
uniform vec3 u_color_tint;

void main() {
    vec2 res = (u_resolution.x > 0.0 && u_resolution.y > 0.0) ? u_resolution : vec2(1280.0, 720.0);

    // normalize aspect ratio
    vec2 st = (gl_FragCoord.xy / res) * 2.0 - 1.0;
    st.x *= (res.x / res.y);

    // use dynamic zoom level and center
    vec2 c = st * u_zoom_level + u_zoom_center;
    vec2 z = vec2(0.0, 0.0);

    float iter = u_max_iter;

    // fractal loop Z = Z^2 + C
    for (int i = 0; i < 250; i++) {
        if (float(i) >= u_max_iter) break; // break early to actually use the corret iteration count

        // complex squaring math: real (x^2 - y^2) and imaginary (2*x*y)
        float x_next = z.x * z.x - z.y * z.y + c.x;
        float y_next = 2.0 * z.x * z.y + c.y;

        z = vec2(x_next, y_next);

        // if it escapes radius 2 (diameter 4), it bounds off towards infinity (bad)
        if (dot(z, z) > 4.0) {
            iter = float(i);
            break;
        }
    }

    // color mapping
    if (iter >= u_max_iter) {
        // points inside set stay black
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        // normalize escape velocity to 0.0 > x > 1.0
        float t = iter / u_max_iter;

        // procedural color
        vec3 color = t * u_color_tint;

        // exponentially boost brightness at edges
        color += vec3(pow(t, 8.0) * 3.5) * vec3(1.2, 0.5, 1.0);

        FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
    }
}