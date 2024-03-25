#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D texture1;
uniform float transparency;

void main() {
    vec2 flippedTexCoord = vec2(TexCoord.x, 1.0 - TexCoord.y);
    vec4 color1 = texture(texture1, flippedTexCoord);

    FragColor = color1 / transparency; 
}
