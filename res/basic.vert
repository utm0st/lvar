#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoords;

out vec2 tex_coords;

uniform mat4 projection;
uniform mat4 wtf;
uniform mat4 view;
uniform mat4 model;

void main()
{
  vec4 test = vec4(pos, 1.0) + wtf[0];
  gl_Position = projection * view * model * test;
  tex_coords = texCoords;
}
