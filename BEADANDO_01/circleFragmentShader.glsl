#version 330

// Bemeno valtozok
in vec2 varyingPosition; // NDC Pozicio
out vec4 outColor;           // kimenő szín

// Uniform valtozok
uniform vec2 circleCenter;
uniform float circleRadius;
uniform bool colorSwapped;

// Ablak meret
uniform float windowWidth;
uniform float windowHeight;

// Vonal
uniform float lineY;
uniform float lineHalfLen;

void main(void) {
    float dist = distance(varyingPosition, circleCenter);

    if (dist > circleRadius) {
        discard;
    }

    // Interpolacio
    float t = dist / circleRadius;

    vec3 centerColor = colorSwapped ? vec3(0.0, 0.8, 0.0) : vec3(0.8, 0.0, 0.0); // piros
    vec3 borderColor = colorSwapped ? vec3(0.8, 0.0, 0.0) : vec3(0.0, 0.8, 0.0); // zold

    vec3 color = mix(centerColor, borderColor, t);
    outColor = vec4(color, 1.0);
}
