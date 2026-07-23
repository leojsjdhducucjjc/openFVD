#version 130

out vec4 oFragColor;
uniform vec3 color;
uniform float opacity;

void main(void)
{
    oFragColor = vec4(color, opacity);
}
