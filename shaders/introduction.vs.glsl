#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

out vec2 TexCoord;

uniform vec3 transform;
uniform vec3 scale;

void main() {
    vec3 newPosition = (position + transform) * scale;

    gl_Position = vec4(newPosition, 1.0);
    TexCoord = texCoord;
}
