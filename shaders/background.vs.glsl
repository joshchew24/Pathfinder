#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

out vec2 TexCoord;

uniform vec2 camera;
uniform float parallaxFactor;

void main() {
    vec2 adjustedTexCoord = texCoord - camera * vec2(parallaxFactor);
    gl_Position = vec4(position, 1.0);
    TexCoord = adjustedTexCoord;
}