#version 410 core

layout (location = 0) in vec2 vPosition;

uniform mat4 matModelView;
uniform mat4 matProjection;

void main() {
    gl_Position = matProjection * matModelView * vec4(vPosition, 0.0, 1.0);
}
