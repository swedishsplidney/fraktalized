#version 430 core

layout(local_size_x = 256, local_size_y = 1) in;

layout(r32ui, binding = 0) uniform coherent uimage2D u_accumulationTex;

uniform vec2 u_resolution;
uniform dvec2 u_zoom_center;
uniform double u_zoom_level;
uniform int u_fractal_type;

// deterministic hash
uint pcg_hash(uint src) {
    uint state = src * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

// lcg random generator
double nextRandomDouble(inout uint seed) {
    seed = pcg_hash(seed);
    double part1 = double(seed & 0x1FFFFFu) / 2097152.0;

    seed = pcg_hash(seed);
    double part2 = double(seed) / 429467296.0;

    return part1 + (part2 / 2097152.0);
}

void main() {
    uint global_id = gl_GlobalInvocationID.x;
    if (global_id >= 200000) return;

    // base seed per invocation
    uint hash_x = pcg_hash(uint(abs(u_zoom_center.x) * 1000000.0));
    uint hash_y = pcg_hash(uint(abs(u_zoom_center.y) * 1000000.0));
    uint hash_z = pcg_hash(uint(u_zoom_level * 10000000.0));

    uint seed = pcg_hash(global_id + hash_x ^ hash_y ^ hash_z);

    // start at 0, 0
    dvec2 p = dvec2(0.0, 0.0);

    // get away from the origin
    for(int i = 0; i < 30; i++) {
        double r = nextRandomDouble(seed);
        dvec2 next_p;
        if (r < 0.01) {
            next_p.x = 0.0;
            next_p.y = 0.16 * p.y;
        } else if (r < 0.86) {
            next_p.x = 0.85 * p.x + 0.04 * p.y;
            next_p.y = -0.04 * p.x + 0.85 * p.y + 1.6;
        } else if (r < 0.93) {
            next_p.x = 0.20 * p.x - 0.26 * p.y;
            next_p.y = 0.23 * p.x + 0.22 * p.y + 1.6;
        } else {
            next_p.x = -0.15 * p.x + 0.28 * p.y;
            next_p.y = 0.26 * p.x + 0.24 * p.y + 0.44;
        }
        p = next_p;
    }

    double inverseZoomSq = 1.0 / (u_zoom_level * u_zoom_level);
    int steps_per_thread = int(clamp(1000.0 * inverseZoomSq, 1000.0, 100000.0));

    // camera and screen mapping
    float screenAspect = u_resolution.x / u_resolution.y;
    float scaleX = 1.0;
    float scaleY = 1.0;

    if (u_resolution.x >= u_resolution.y) {
        // landscape
        scaleY = 1.0 / screenAspect;
    } else {
        // portrait
        scaleX = screenAspect;
    }

    // main drawing loop
    for(int i = 0;  i < steps_per_thread; i++) {
        double r = nextRandomDouble(seed);
        dvec2 next_p;
        if (r < 0.01) {
            // stem
            next_p.x = 0.0;
            next_p.y = 0.16 * p.y;
        } else if (r < 0.86) {
            // smaller leaflets
            next_p.x = 0.85 * p.x + 0.04 * p.y;
            next_p.y = -0.04 * p.x + 0.85 * p.y + 1.6;
        } else if (r < 0.93) {
            // left leaflet
            next_p.x = 0.20 * p.x - 0.26 * p.y;
            next_p.y = 0.23 * p.x + 0.22 * p.y + 1.6;
        } else {
            // right leaflet
            next_p.x = -0.15 * p.x + 0.28 * p.y;
            next_p.y = 0.26 * p.x + 0.24 * p.y + 0.44;
        }
        p = next_p;

        dvec2 local_center = dvec2(u_zoom_center.x, -u_zoom_center.y);

        dvec2 screen_space = (p - local_center) / u_zoom_level;
        screen_space.x /= scaleX;
        screen_space.y /= scaleY;

        // convert to vec2 uv space
        vec2 final_uv = vec2(screen_space) * 0.5 + vec2(0.5);

        final_uv.y = 1.0 - final_uv.y;

        ivec2 write_pos = ivec2(final_uv * u_resolution);

        // point to texture
        if (write_pos.x >= 0 && write_pos.x < int(u_resolution.x) && write_pos.y >= 0 && write_pos.y < int(u_resolution.y)) {
            imageAtomicAdd(u_accumulationTex, write_pos, 1u);
        }
    }
}