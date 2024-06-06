#version 330 core

in vec2 tex_coords;

out vec4 FragColour;

uniform sampler2D image1;

void main()
{
  FragColour = texture(image1, tex_coords);
}
