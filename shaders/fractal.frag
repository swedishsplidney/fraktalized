#version 330 core
#extension GL_ARB_gpu_shader_fp64 : enable // turn on 64 bit mode

out vec4 FragColor;
in vec2 v_coord;

// dynamic inputs
uniform vec2 u_resolution;
uniform dvec2 u_zoom_center;
uniform double u_zoom_level;
uniform float u_max_iter;
uniform vec3 u_color_tint;

uniform int u_fractal_type; // 0 = mandelbrot, 1 = julia
uniform dvec2 u_julia_c;     // constant point

void main() {
    vec2 res = (u_resolution.x > 0.0 && u_resolution.y > 0.0) ? u_resolution : vec2(1280.0, 720.0);

    // normalize aspect ratio
    vec2 st = v_coord;
    st.x *= (res.x / res.y);

    dvec2 coord = dvec2(st) * u_zoom_level + u_zoom_center;

    dvec2 z;
    dvec2 c;

    // set constant based on fractal type
    if (u_fractal_type == 0) {
        // mandelbrot
        z = dvec2(0.0, 0.0);
        c = coord;
    } else if (u_fractal_type == 1) {
        // julia
        z = coord;
        c = u_julia_c;
    }

    float iter = u_max_iter;

    // fractal loop Z = Z^2 + C
    for (int i = 0; i < 3000; i++) {
        if (float(i) >= u_max_iter) break; // break early to actually use the corret iteration count

        // complex squaring math: real (x^2 - y^2) and imaginary (2*x*y)
        double x_next = z.x * z.x - z.y * z.y + c.x;
        double y_next = 2.0 * z.x * z.y + c.y;

        z = dvec2(x_next, y_next);

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
        color += vec3(pow(t, 2.0) * 0.4);

        FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
    }
}