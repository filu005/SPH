#version 330 core
in vec3 Position;
in vec3 Normal;
in vec3 outColor;

out vec4 color;

vec3 lightPos = vec3(5.0, 5.0, 0.0);
vec3 ambientComponent = vec3(0.2);
vec3 diffuseComponent = vec3(0.5);

void main()
{
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - Position);

	float diffuse = max(dot(norm, lightDir), 0.0);
	diffuseComponent = diffuse * diffuseComponent;

	vec3 result = (ambientComponent + diffuseComponent) * outColor;
    color = vec4(result, 1.0);
}
