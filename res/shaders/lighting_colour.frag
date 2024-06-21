#version 330 core

out vec4 colour_frag;

uniform vec3 colour_reflected;

void main()
{
  colour_frag = vec4(colour_reflected, 1.0);
}
