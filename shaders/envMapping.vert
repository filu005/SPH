#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 Normal;
out vec3 Position;

out VertexData
{
  vec4 Position;
  vec4 OPosition;
  vec4 Normal;
  vec4 ONormal;
  vec4 UV;
} vertex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vec4 P = view * model * vec4(position, 1.0);
	vertex.OPosition = P;
	vertex.UV = normalize(P);
	vertex.Position = vec4(position, 1.0);
	vertex.Normal = view * model * vec4(normal, 1.0);
	vertex.ONormal = vec4(normal, 1.0);

    gl_Position = projection * view * model * vec4(position, 1.0f);
    Normal = normalize(mat3(transpose(inverse(model))) * normal);
    Position = vec3(model * vec4(position, 1.0f));
}