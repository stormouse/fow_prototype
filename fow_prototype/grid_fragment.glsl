#version 330

uniform vec3 fowColor;
out vec4 fragColor;

void main()
{
    fragColor = vec4(fowColor, 1.0f);
}