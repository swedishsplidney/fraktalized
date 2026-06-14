#version 330 core
#extension GL_ARB_gpu_shader_fp64 : enable // turn on 64 bit mode

out vec4 FragColor;
in vec2 v_coord;

// dynamic inputs
uniform vec2 u_resolution;
uniform dvec2 u_zoom_center;
uniform double u_zoom_level;
uniform float u_max_iter;

uniform vec3 u_gradient_colors[4];
uniform float u_gradient_stops[4];

uniform int u_fractal_type; // 0 = mandelbrot, 1 = julia
uniform dvec2 u_julia_c;     // constant point

uniform vec4 u_tile_bounds;

uniform int u_aaSamples;

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
        // mandelbrot
        z = dvec2(0.0, 0.0);
        c = coord;
    } else if (u_fractal_type == 1) {
        // julia
        z = coord;
        c = u_julia_c;
    } else if (u_fractal_type == 2) {
        // burning ship
        z = dvec2(0.0, 0.0);
        c = coord;
    } else if (u_fractal_type == 3) {
        // newton fractal
        z = coord;
        c = dvec2(0.0, 0.0);
    }

    float iter = u_max_iter;

    // fractal loop
    for (int i = 0; i < 2147483647; i++) {
        if (float(i) >= u_max_iter) break;

        double x_next;
        double y_next;

        if (u_fractal_type <= 1) {
            // mandelbrot / julia: Z = Z^2 + C
            x_next = z.x * z.x - z.y * z.y + c.x;
            y_next = 2.0 * z.x * z.y + c.y;

            z = dvec2(x_next, y_next);

            if (dot (z, z) > 4.0) {
                iter = float(i);
                break;
            }
        } else if (u_fractal_type == 2) {
            // burning ship: Z = | Z^2 | + C
            dvec2 absZ = abs(z);
            x_next = absZ.x * absZ.x - absZ.y * absZ.y + c.x;
            y_next = 2.0 * absZ.x * absZ.y + c.y;

            z = dvec2(x_next, y_next);

            if (dot (z, z) > 4.0) {
                iter = float(i);
                break;
            }
        } else if (u_fractal_type == 3) {
            // newton: Z^3 - 1 = 0
            double r2 = (z.x * z.x + z.y * z.y);
            double denom = 3.0 * r2 * r2;

            // no dividing by zero
            if (denom < 0.0000001) { iter = u_max_iter; break; }

            x_next = (2.0 / 3.0) * z.x + (z.x * z.x - z.y * z.y) / denom;
            y_next = (2.0 / 3.0) * z.y - (2.0 * z.x * z.y) / denom;

            dvec2 z_next = dvec2(x_next, y_next);
            dvec2 diff = z_next - z;
            double dist2 = dot(diff, diff);

            // tolerance limit
            if (dist2 < 0.0000001) {
                // smoothing math
                float log_zn = log(float(dist2));
                float nu = log(log_zn / log(0.0000001)) / log(3.0);

                iter = float(i) + 1.0 - nu;
                z = z_next;
                break;
            }
            z = z_next;
        }
    }

    // color mapping
    if (iter >= u_max_iter) {
        return vec3(0.0, 0.0, 0.0);
    } else {
        // normalize escape velocity
        float t = iter / u_max_iter;

        // gradient mapping
        vec3 color = u_gradient_colors[0]; // fallback

        if (u_fractal_type <= 2) {
            // mandelbrot, julia, burning ship
            for (int i = 0; i < 3; i++) {
                if (t >= u_gradient_stops[i] && t <= u_gradient_stops[i+1]) {
                    float factor = (t - u_gradient_stops[i]) / (u_gradient_stops[i+1] - u_gradient_stops[i]);
                    color = mix(u_gradient_colors[i], u_gradient_colors[i+1], factor);
                    break;
                }
            }
        } else if (u_fractal_type == 3) {
            // newton
            float smooth_t = fract(t * 15.0);

            for (int i = 0; i < 3; i++) {
                if (smooth_t >= u_gradient_stops[i] && smooth_t <= u_gradient_stops[i+1]) {
                    float factor = (smooth_t - u_gradient_stops[i]) / (u_gradient_stops[i+1] - u_gradient_stops[i]);
                    color = mix(u_gradient_colors[i], u_gradient_colors[i+1], factor);
                    break;
                }
            }

            if (z.x > 0.0) {
                color = mix(color, u_gradient_colors[1], 0.25);
            } else if (z.y > 0.0) {
                color = mix(color, u_gradient_colors[2], 0.25);
            } else {
                color = mix(color, u_gradient_colors[3], 0.25);
            }
        }

        // boost brightness at edges
        color += vec3(pow(t, 2.0) * 0.4);
        // clamp
        return clamp(color, 0.0, 1.0);
    }
}

void main() {
    // anti aliasing mode: 1 = none, 2 = 2x ssaa, 3 = 3x ssaa

    if (u_aaSamples <= 1) {
        // no aa
        FragColor = vec4(evaluateFractal(v_coord), 1.0);
    } else {
        vec3 accumulatedColor = vec3(0.0);

        // calc the size of a pixel
        vec2 pixelSize = vec2(2.0) / u_resolution;

        // loop through subpixel grid
        for (int y = 0; y < u_aaSamples; y++) {
            for (int x = 0; x < u_aaSamples; x++) {
                // calc fractional offsets within the pixel
                vec2 offset = vec2(
                    (float(x) + 0.5) / float(u_aaSamples) - 0.5,
                    (float(y) + 0.5) / float(u_aaSamples) - 0.5
                );

                // shift vertex input coord
                vec2 sampleCoord = v_coord + (offset * pixelSize);
                accumulatedColor += evaluateFractal(sampleCoord);
            }
        }

        // average out the samples
        vec3 finalColor = accumulatedColor / float(u_aaSamples * u_aaSamples);
        FragColor = vec4(finalColor, 1.0);
    }
}