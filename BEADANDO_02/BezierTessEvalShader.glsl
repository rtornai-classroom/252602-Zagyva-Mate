#version 410 core

layout (isolines, equal_spacing, ccw) in;

uniform mat4 matModelView;
uniform mat4 matProjection;
uniform int  curveType;
uniform int  controlPointsNumber;

// Binomiális együttható
float NCR(int n, int r) {
    if (r == 0 || r == n) return 1.0;
    if (r > n - r) r = n - r; // symmetry
    float result = 1.0;
    for (int k = 1; k <= r; ++k) {
        result *= float(n - k + 1);
        result /= float(k);
    }
    return result;
}

float B(int n, int i, float t) {
    return NCR(n, i) * pow(t, float(i)) * pow(1.0 - t, float(n - i));
}

vec3 bezierPoint(float t) {
    vec3 p = vec3(0.0);
    int  n = controlPointsNumber - 1;
    for (int i = 0; i < controlPointsNumber; ++i)
        p += B(n, i, t) * vec3(gl_in[i].gl_Position);
    return p;
}

void main() {
    float t = gl_TessCoord.x;
    vec3  p = bezierPoint(t);
    gl_Position = matProjection * matModelView * vec4(p, 1.0);
}
