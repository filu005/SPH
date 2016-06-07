#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

in vec3 wc_vert_coord;
in vec3 wc_vert_normal;

// env mapping
in vec3 model_position;
in vec3 model_normal;
uniform samplerCube skybox;
uniform vec3 viewPos;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

vec4 getTriPlanarBlendColor(vec3 wc_coord, vec3 wc_normal)
{
    // in wc_normal is the world-space normal of the fragment
    vec3 blending = abs(wc_normal);
    blending = normalize(max(blending, 0.00000001)); // Force weights to sum to 1.0
    float b = (blending.x + blending.y + blending.z);
    blending /= vec3(b, b, b);

    vec4 xaxis = texture2D(diffuseMap, wc_coord.yz);
    vec4 yaxis = texture2D(diffuseMap, wc_coord.xz);
    vec4 zaxis = texture2D(diffuseMap, wc_coord.xy);
    // blend the results of the 3 planar projections.
    vec4 tex = xaxis * blending.x + yaxis * blending.y + zaxis * blending.z;
    return tex;
}

vec4 envMapping(vec3 m_position, vec3 m_normal)
{
    float ratio = 1.00 / 1.52;
    vec3 I = normalize(m_position - viewPos);
    vec3 R = refract(I, normalize(m_normal), ratio);
    return texture(skybox, R);
}

void main()
{
    // Obtain normal from normal map in range [0,1]
    vec3 normalT = texture(normalMap, fs_in.TexCoords * 2.0).rgb;
    // Transform normal vector to range [-1,1]
    vec3 normal = normalize(normalT * 2.0 - 1.0); // this normal is in tangent space

    // Get diffuse color
    vec3 color = texture(diffuseMap, fs_in.TexCoords).rgb;
    // Ambient
    vec3 ambient = 0.1 * color;
    // Diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // Specular
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = vec3(0.2) * spec;


    // FragColor = vec4(ambient + diffuse + specular, 1.0);
    // FragColor = getTriPlanarBlendColor(wc_vert_coord, wc_vert_normal);
    FragColor = envMapping(model_position, normal);
}