#version 330 core

in vec2 tex_coords;

out vec4 FragColour;

uniform sampler2D image1;
uniform sampler2D image2;

void main()
{
  FragColour = mix(texture(image1, tex_coords), texture(image2, tex_coords), 0.2);
}
