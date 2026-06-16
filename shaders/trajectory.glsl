#version 430 core

// execute threads in 16x16 blocks
layout(local_size_x =  16, local_size_y = 16) in;

// master canvas
layout(r32ui, binding = 0) uniform coherent uimage2D u_accumulation_tex;

uniform vec2 u_resolution;
uniform dvec2 u_zoom_center;
uniform double u_zoom_level;
uniform float u_max_iter;
uniform int u_fractal_type;

// deterministic random generator
uint hash(uint x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

void main() {
    ivec2 tex_coord = ivec2(gl_GlobalInvocationID.xy);
    if (tex_coord.x >= int(u_resolution.x) || tex_coord.y >= int(u_resolution.y)) return;

    // read the existing value accumulated from previous frames
    uint current_val = imageLoad(u_accumulation_tex, tex_coord).r;
    // fade it out slightly so it doesn't saturate to infinity
    uint faded_val = uint(float(current_val) * 0.75);
    imageStore(u_accumulation_tex, tex_coord, uvec4(faded_val, 0, 0, 0));

    memoryBarrierImage();
    barrier();

    // generate pseudorandom coord offsets
    uint seed1 = hash(gl_GlobalInvocationID.x + hash(gl_GlobalInvocationID.y));
    uint seed2 = hash(seed1 + 147u);

    int samples_per_thread = 30;
    for(int s = 0; s < samples_per_thread; s++) {

        // mutate seeds for each iteration so samples are unique
        seed1 = hash(seed1 ^ uint(s));
        seed2 = hash(seed2 + seed1);

        float rand_x = float(seed1 & 0xFFFFu) / 65535.0;
        float rand_y = float(seed2 & 0xFFFFu) / 65535.0;

        // convert pixel coords and jitter back into dvec2
        vec2 normalized = (vec2(tex_coord) + vec2(rand_x, rand_y)) / u_resolution;
        normalized = normalized * 2.0 - vec2(1.0);

        // aspect ratio scaling
        float aspect = u_resolution.x / u_resolution.y;
        if (u_resolution.x >= u_resolution.y) {
            // landscape
            normalized.y /= aspect;
        } else {
            // portrait
            normalized.x *= aspect;
        }

        dvec2 c_start = dvec2(normalized) * u_zoom_level + u_zoom_center;

        // optimization for anti-buddhabrot
        double q = (c_start.x - 0.25) * (c_start.x - 0.25) + c_start.y * c_start.y;
        bool in_main_cardioid = q * (q + (c_start.x - 0.25)) < 0.25 * c_start.y * c_start.y;
        bool in_period2_bulb = (c_start.x + 1.0) * (c_start.x + 1.0) + c_start.y * c_start.y < 0.0625;

        if (u_fractal_type == 5 && (in_main_cardioid || in_period2_bulb)) {
            continue;
        }

        dvec2 z = dvec2(0.0, 0.0);

        // check if point escapes
        bool escapes = false;
        int escape_iterations = 0;

        for (int i = 0; i < int(u_max_iter); i++) {
            double x_next = z.x * z.x - z.y * z.y + c_start.x;
            double y_next = 2.0 * z.x * z.y + c_start.y;
            z = dvec2(x_next, y_next);

            if (dot(z, z) > 4.0) {
                escapes = true;
                escape_iterations = i;
                break;
            }
        }

        // determine render mode
        bool should_plot = false;
        int loop_limit = 0;

        if (u_fractal_type == 4 && escapes && escape_iterations > 50) {
            should_plot = true;
            loop_limit = escape_iterations;
        } else if (u_fractal_type == 5 && !escapes) {
            should_plot = true;
            loop_limit = int(u_max_iter);
        }

        if (should_plot) {
            z = dvec2(0.0, 0.0);

            for (int i = 0; i < loop_limit; i++) {
                double x_next = z.x * z.x - z.y * z.y + c_start.x;
                double y_next = 2.0 * z.x * z.y + c_start.y;
                z = dvec2(x_next, y_next);

                float screenAspect = u_resolution.x / u_resolution.y;
                float scaleX = 1.0;
                float scaleY = 1.0;

                if (u_resolution.x >= u_resolution.y) {
                    scaleY = 1.0 / screenAspect;
                } else {
                    scaleX = screenAspect;
                }

                dvec2 screen_space = (z - u_zoom_center) / u_zoom_level;
                screen_space.x /= scaleX;
                screen_space.y /= scaleY;

                vec2 final_uv = vec2(screen_space) * 0.5 + vec2(0.5);

                ivec2 write_pos = ivec2(final_uv * u_resolution);

                if (write_pos.x >= 0 && write_pos.x < int(u_resolution.x) && write_pos.y >= 0 && write_pos.y < int(u_resolution.y)) {
                    imageAtomicAdd(u_accumulation_tex, write_pos, 1u);
                }
            }
        }
    }
}