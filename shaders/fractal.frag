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

uniform vec4 u_tile_bounds;

// anti aliasing color calc
vec3 evaluateFractal(vec2 custom_v_coord) {
    // normalize aspect ratio
    vec2 normalizedCoord = vec2(
        mix(u_tile_bounds.x, u_tile_bounds.z, custom_v_coord.x * 0.5 + 0.5),
        mix(u_tile_bounds.y, u_tile_bounds.w, custom_v_coord.y * 0.5 + 0.5)
    );

    dvec2 coord = dvec2(normalizedCoord) * u_zoom_level + u_zoom_center;
    dvec2 z;
    dvec2 c;

    // set constant based on fractal type
    if (u_fractal_type == 0) {
        z = dvec2(0.0, 0.0);
        c = coord;
    } else if (u_fractal_type == 1) {
        z = coord;
        c = u_julia_c;
    }

    float iter = u_max_iter;

    // fractal loop Z = Z^2 + C
    for (int i = 0; i < 2147483647; i++) {
        if (float(i) >= u_max_iter) break;

        double x_next = z.x * z.x - z.y * z.y + c.x;
        double y_next = 2.0 * z.x * z.y + c.y;
        z = dvec2(x_next, y_next);

        if (dot (z, z) > 4.0) {
            iter = float(i);
            break;
        }
    }

    // color mapping
    if (iter >= u_max_iter) {
        return vec3(0.0, 0.0, 0.0);
    } else {
        // normalize escape velocity
        float t = iter / u_max_iter;

        vec3 color = t * u_color_tint;

        // boost brightness at edges
        color += vec3(pow(t, 2.0) * 0.4);
        // clamp
        return clamp(color, 0.0, 1.0);
    }
}

void main() {
    // anti aliasing mode: 1 = none, 2 = 2x ssaa, 3 = 3x ssaa
    int AA_SAMPLES = 2;

    if (AA_SAMPLES <= 1) {
        // no aa
        FragColor = vec4(evaluateFractal(v_coord), 1.0);
    } else {
        vec3 accumulatedColor = vec3(0.0);

        // calc the size of a pixel
        vec2 pixelSize = vec2(2.0) / u_resolution;

        // loop through subpixel grid
        for (int y = 0; y < AA_SAMPLES; y++) {
            for (int x = 0; x < AA_SAMPLES; x++) {
                // calc fractional offsets within the pixel
                vec2 offset = vec2(
                    (float(x) + 0.5) / float(AA_SAMPLES) - 0.5,
                    (float(y) + 0.5) / float(AA_SAMPLES) - 0.5
                );

                // shift vertex input coord
                vec2 sampleCoord = v_coord + (offset * pixelSize);
                accumulatedColor += evaluateFractal(sampleCoord);
            }
        }

        // average out the samples
        vec3 finalColor = accumulatedColor / float(AA_SAMPLES * AA_SAMPLES);
        FragColor = vec4(finalColor, 1.0);
    }
}