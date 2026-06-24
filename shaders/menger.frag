#version 430 core

in vec3 vertexColor;
in vec3 localPos;
in vec3 worldPos;
out vec4 fragColor;

uniform vec3 u_cameraPos;
uniform int u_maxIterations;
uniform mat4 u_model;

uniform int u_stop_count;
uniform vec3 u_gradient_colors[16];
uniform float u_gradient_stops[16];

uniform vec2 u_resolution;
uniform float u_brightness;

// box sdf
float sdBox(vec3 p, vec3 b) {
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

// sponge sdf
float mengerSpongeSDF(vec3 p, out float trap) {
    // outer bound box
    float d = sdBox(p, vec3(1.0));

    float minDist = 1e10;
    float s = 1.0;

    int iterations = clamp(u_maxIterations, 1, 8);

    for (int m = 0; m < iterations; m++) {
        vec3 a = mod(p * s, 2.0) - 1.0;
        s *= 3.0;
        vec3 r = abs(1.0 - 3.0 * abs(a));

        d = max(d, c);
    }

    trap = clamp(minDist * 0.7, 0.0, 1.0);
    return d;
}

// overload
float mengerSpongeSDF(vec3 p) {
    float dummy;
    return mengerSpongeSDF(p, dummy);
}

vec3 getNormal(vec3 p, float currentEpsilon) {
    vec2 e = vec2(currentEpsilon, 0.0);
    float d = mengerSpongeSDF(p);
    vec3 n = d - vec3(
        mengerSpongeSDF(p - e.xyy),
        mengerSpongeSDF(p - e.yxy),
        mengerSpongeSDF(p - e.yyx)
    );
    return normalize(n);
}

float getAO(vec3 hitPoint, vec3 normal) {
    float occ = 0.0;
    float weight = 1.0;

    for (int i = 1; i <= 5; i++) {
        float stepDist = 0.01 + 0.04 * float(i);
        vec3 samplePos = hitPoint + normal * stepDist;

        float sceneDist = mengerSpongeSDF(samplePos);

        occ += weight * (stepDist - sceneDist);
        weight *= 0.5;
    }

    return pow(clamp(1.0 - 1.8 * occ, 0.0, 1.0), 2.0);
}

void main() {
    vec3 rayOrigin = u_cameraPos;
    vec3 rayDir = normalize(worldPos - rayOrigin);

    mat4 invModel = inverse(u_model);

    float totalDistance = 0.0;
    int maxSteps = 512;
    float maxDrawDistance = 30.0;
    bool hit = false;
    vec3 hitPointLocal = vec3(0.0);
    float finalTrapValue = 0.0;
    float hitThreshold = 0.001;

    float closestPassRatio = 1e10;

    for (int i = 0; i < maxSteps; i++) {
        vec3 currentWorldPos = rayOrigin + rayDir * totalDistance;
        vec3 currentLocalPos = (invModel * vec4(currentWorldPos, 1.0)).xyz;

        float currentTrap;
        float distanceToScene = mengerSpongeSDF(currentLocalPos, currentTrap);

        hitThreshold = max(0.00003, 0.00008 * totalDistance);

        vec2 res = (u_resolution.x > 0.0) ? u_resolution : vec2(1920.0, 1080.0);
        float pixelRadius = max(hitThreshold, totalDistance * (2.0 / res.y));

        closestPassRatio = min(closestPassRatio, distanceToScene / pixelRadius);

        if (distanceToScene < hitThreshold) {
            hit = true;
            hitPointLocal = currentLocalPos;
            finalTrapValue = currentTrap;
            break;
        }

        totalDistance += distanceToScene;
        if (totalDistance > maxDrawDistance) break;
    }

    vec3 backgroundColor = u_gradient_colors[0];
    float edgeCoverage = clamp(1.0 - smoothstep(0.0, 1.0, closestPassRatio), 0.0, 1.0);

    if (hit) {
        vec3 normal = getNormal(hitPointLocal, hitThreshold);
        float ao = getAO(hitPointLocal, normal);

        float t = finalTrapValue;
        vec3 baseColor = vec3(0.5);

        if (u_stop_count > 0) {
            baseColor = u_gradient_colors[0];
            for (int i = 0; i < 15; i++) {
                if (i >= u_stop_count - 1) break;
                if (t >= u_gradient_stops[i] && t <= u_gradient_stops[i+1]) {
                    float factor = (t - u_gradient_stops[i]) / (u_gradient_stops[i+1] - u_gradient_stops[i]);
                    baseColor = mix(u_gradient_colors[i], u_gradient_colors[i+1], factor);
                    break;
                }
            }
        } else {
            baseColor = mix(abs(sin(hitPointLocal * 2.0)), vec3(0.9, 0.6, 0.2), t);
        }

        baseColor += (vec3(pow(t, 2.0) * 0.2));

        vec3 lightDir = normalize(vec3(0.6, 0.8, 0.5));
        float diffuse = clamp(dot(normal, lightDir), 0.0, 1.0);

        vec3 skyColor = vec3(0.3, 0.4, 0.5);
        vec3 groundColor = vec3(0.15, 0.1, 0.05);

        float skyGroundWeight = normal.y * 0.5 + 0.5;
        vec3 ambientLight = mix(groundColor, skyColor, skyGroundWeight);

        vec3 directLighting = vec3(diffuse * 0.8);
        float softAO = mix(0.15, 1.0, ao);

        vec3 finalColor = baseColor * (directLighting + ambientLight) * softAO;
        finalColor *= u_brightness;

        // depth fog
        float fogFactor = 1.0 - exp(-0.07 * totalDistance * totalDistance);
        finalColor = mix(finalColor, backgroundColor, clamp(fogFactor, 0.0, 1.0));

        if (totalDistance > maxDrawDistance * 0.8) {
            float edgeFade = smoothstep(maxDrawDistance * 0.8, maxDrawDistance, totalDistance);
            finalColor = mix(finalColor, backgroundColor, edgeFade);
        }

        // gamma mapping
        finalColor = pow(finalColor, vec3(1.0 / 2.0));
        fragColor = vec4(clamp(finalColor, 0.0, 1.0), 1.0);
    } else {
        vec3 finalBackground = mix(backgroundColor, vec3(0.5, 0.6, 0.7) * 0.1, edgeCoverage);
        fragColor = vec4(finalBackground, 1.0);
    }
}