#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 Position;
out vec3 Normal;
out vec3 outColor;

// uniform vec3 color;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    Position = vec3(model * vec4(position, 1.0));
    Normal = normal;
    outColor = vec3(1.0, 0.4, 0.2);
}