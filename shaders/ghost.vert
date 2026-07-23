#version 130

in vec3 aPosition;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;
uniform mat4 anchorBase;

void main(void)
{
    gl_Position = projectionMatrix * modelMatrix * anchorBase * vec4(aPosition, 1.0);
}
