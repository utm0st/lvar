#version 330 core

out vec3 normal;
out vec3 frag_world_pos;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;

layout(std140) uniform matrices {
  mat4 projection;
  mat4 view;
};

uniform mat4 model;
uniform mat4 model_trans;

void main()
{
  gl_Position = projection * view * model * vec4(pos, 1.0);
  normal = mat3(model_trans) * norm;
  frag_world_pos = vec3(model * vec4(pos, 1.0));
}
