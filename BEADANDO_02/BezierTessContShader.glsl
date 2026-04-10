#version 410 core

layout (vertices = 32) out;

uniform int controlPointsNumber;

void main() {
    if (gl_InvocationID < controlPointsNumber)
        gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = 1;
        gl_TessLevelOuter[1] = 1024; // gorbe felbonatsa
    }
}
