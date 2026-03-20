#version 330

layout (location = 0) in vec2 aPosition;

out vec2 varyingPosition;

void main(void) {
    varyingPosition = aPosition;
    gl_Position = vec4(aPosition, 0.0, 1.0);
}
