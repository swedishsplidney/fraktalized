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
uniform float u_power;

// icebox sdf
float iceboxSDF(vec3 p, out float trap) {
    vec3 z = p;
    float dr = 1.0;
    float minDist = 1e10;
    float r;
    
    // settings
    float power = u_power;
    float foldLimit = 1.0;
    
    int iterations = clamp(u_maxIterations, 1, 64);
    
    for (int n = 0; n < iterations; n++) {
        r = max(abs(z.x), max(abs(z.y), abs(z.z)));
        if (r > 2.0) break;
        
        float theta = acos(clamp(z.z / (r+ 0.00001), -1.0, 1.0));
        float phi = atan(z.y, z.x);
        
        float derivpow = pow(r, max(0.0, power - 1.0));
        dr = derivpow * power * dr + 1.0;
        
        theta *= power;
        phi *= power;
        
        z = (derivpow * r) * vec3(cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta));
        z += p;
        
        z = clamp(z, -foldLimit, foldLimit) * 2.0 - z;
        
        float r2 = dot(z, z);
        minDist = min(minDist, r2);
        
        // sphere inversion
        if (r2 < 0.5) {
            z *= 2.6;
        } else if (r2 < 1.3) {
            z *= (1.3 / r2);
        }
    }
    
    trap = clamp(minDist * 0.2, 0.0, 1.0);
    return 0.25 * log(r) * r / dr;
}

// overload
float iceboxSDF(vec3 p) {
    float dummy;
    return iceboxSDF(p, dummy);
}

vec3 getNormal(vec3 p, float currentEpsilon) {
    vec2 e = vec2(currentEpsilon, 0.0);
    float d = iceboxSDF(p);
    vec3 n = d - vec3(
        iceboxSDF(p - e.xyy),
        iceboxSDF(p - e.yxy),
        iceboxSDF(p - e.yyx)
    );
    return normalize(n);
}

float getAO(vec3 hitPoint, vec3 normal) {
    float occ = 0.0;
    float weight = 1.0;
    for (int i = 1; i <= 5; i++) {
        float stepDist = 0.01 + 0.04 * float(i);
        vec3 samplePos = hitPoint + normal * stepDist;
        float sceneDist = iceboxSDF(samplePos);
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
    int maxSteps = 256;
    float maxDrawDistance = 20.0;
    bool hit = false;
    vec3 hitPointLocal = vec3(0.0);
    float finalTrapValue = 0.0;
    float hitThreshold = 0.001;
    float closestPassRatio = 1e10;

    for (int i = 0; i < maxSteps; i++) {
        vec3 currentWorldPos = rayOrigin + rayDir * totalDistance;
        vec3 currentLocalPos = (invModel * vec4(currentWorldPos, 1.0)).xyz;

        float currentTrap;
        float distanceToScene = iceboxSDF(currentLocalPos, currentTrap);
        hitThreshold = max(0.0001, 0.0002 * totalDistance);

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
            baseColor = mix(vec3(0.6, 0.8, 1.0), vec3(0.05, 0.2, 0.4), t);
        }

        vec3 lightDir = normalize(vec3(0.5, 0.8, 0.6));
        float diffuse = clamp(dot(normal, lightDir), 0.0, 1.0);

        // specular layer
        vec3 viewDir = -rayDir;
        vec3 halfDir = normalize(lightDir + viewDir);
        float spec = pow(clamp(dot(normal, halfDir), 0.0, 1.0), 32.0) * 0.5;

        vec3 skyColor = vec3(0.7, 0.85, 1.0);
        vec3 groundColor = vec3(0.1, 0.15, 0.2);
        vec3 ambientLight = mix(groundColor, skyColor, normal.y * 0.5 + 0.5);

        vec3 finalColor = baseColor * (diffuse * 0.8 + ambientLight) * mix(0.2, 1.0, ao) + vec3(spec);
        finalColor *= u_brightness;

        float fogFactor = 1.0 - exp(-0.1 * totalDistance * totalDistance);
        finalColor = mix(finalColor, backgroundColor, clamp(fogFactor, 0.0, 1.0));

        finalColor = pow(finalColor, vec3(1.0 / 2.2));
        fragColor = vec4(clamp(finalColor, 0.0, 1.0), 1.0);
    } else {
        fragColor = vec4(mix(backgroundColor, vec3(0.4, 0.5, 0.6) * 0.2, edgeCoverage), 1.0);
    }
}