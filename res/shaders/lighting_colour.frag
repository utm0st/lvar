#version 330 core

in vec3 normal;
in vec3 frag_world_pos;

out vec4 colour_frag;

uniform vec3 colour_object;
uniform vec3 colour_light;
uniform vec3 light_pos;

const float ambient_strength = 0.1f;

void main()
{
  // ambient
  vec3 ambient = ambient_strength * colour_light;
  // diffuse
  vec3 normal_normalised = normalize(normal);
  vec3 light_dir_normalised = normalize(light_pos - frag_world_pos);
  // @NOTE: if the angle between the normal and the light direction is greater than
  // 90 degrees, then the dot product will be negative, so we set it to 0 bc no light
  // will hit this particular fragment.
  float diff_factor = max(dot(normal_normalised, light_dir_normalised), 0.0f);
  vec3 diffuse = diff_factor * colour_light;
  // result
  vec3 result = (ambient + diffuse) * colour_object;
  colour_frag = vec4(result, 1.0);
}
