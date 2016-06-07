#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

uniform vec3 viewPos;

// Declare an interface block; see 'Advanced GLSL' for what these are.
out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

out vec3 wc_vert_coord;
out vec3 wc_vert_normal;

// env mapping
out vec3 model_position;
out vec3 model_normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

// spherical UV mapping
vec2 calcUV(vec3 p)
{
    p = normalize(p);

    const float PI = 3.1415926;

    float u = ((atan(p.x, p.z) / PI) + 1.0) * 0.5;
    float v = (asin(p.y) / PI) + 0.5;

    return vec2(u, v);
}

void main()
{
	vec3 lightPos = vec3(-2.0, 2.0, 1.0);
	wc_vert_normal = normal;
	wc_vert_coord = position;

	// env mapping
	model_position = vec3(model * vec4(position, 1.0f));
	model_normal = mat3(transpose(inverse(model))) * normal;

    gl_Position = projection * view * model * vec4(position, 1.0);
    vs_out.FragPos = vec3(model * vec4(position, 1.0));
    vs_out.TexCoords = calcUV(position);


    ///////////////////////////////////////// obliczanie N, T, B
	vec3 tangent;
	vec3 bitangent;

	vec3 c1 = cross(normal, vec3(0.0, 0.0, 1.0));
	vec3 c2 = cross(normal, vec3(0.0, 1.0, 0.0));

	if(length(c1) > length(c2))
	{
		tangent = c1;
	}
	else
	{
		tangent = c2;
	}

	tangent = normalize(tangent);

	bitangent = cross(normal, tangent);
	bitangent = normalize(bitangent);


    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * tangent);
    vec3 B = normalize(normalMatrix * bitangent);
    vec3 N = normalize(normalMatrix * normal);

    mat3 TBN = transpose(mat3(T, B, N));
    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos  = TBN * viewPos;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;
}
