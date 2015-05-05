#version 330 core
in vec3 Normal;
in vec3 Position;
out vec4 color;

uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform sampler2D texturemap;
uniform sampler2D normalmap;

#define M_PI 3.14159265

//////////////////////////////////////////////////////////////////////////////////
// http://www.geeks3d.com/20130122/normal-mapping-without-precomputed-tangent-space-vectors/
mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );

    // solve the linear system
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    // construct a scale-invariant frame
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, N );
}

vec3 perturb_normal( vec3 N, vec3 V, vec2 texcoord )
{
    // assume N, the interpolated vertex normal and
    // V, the view vector (vertex to eye)
    vec3 map = texture(normalmap, texcoord ).xyz;
    map = map * 255./127. - 128./127.;
    mat3 TBN = cotangent_frame(N, -V, texcoord);
    return normalize(TBN * map);
}
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// http://www.geeks3d.com/20140521/shader-library-bumpy-sphere-env-normal-mapping/

vec3 baseColor = vec3(0.25, 0.64, 0.9);
float normalScale = 1.0;
float texScale = 0.2;
float useSSS = 0.0;
float useScreen = 0.0;

in VertexData
{
  vec4 Position;
  vec4 OPosition;
  vec4 Normal;
  vec4 ONormal;
  vec4 UV;
} vertex;

float random(vec3 scale,float seed)
{
    return fract(sin(dot(gl_FragCoord.xyz+seed,scale))*43758.5453+seed);
}

// The metaballs don't have generated UV coordinates, and tangents can't be
// calculated with the usual method because it relies on texture coordinates.
void main()
/*
{
    // http://gamedevelopment.tutsplus.com/articles/use-tri-planar-texture-mapping-for-better-terrain--gamedev-13821
    // http://http.developer.nvidia.com/GPUGems3/gpugems3_ch01.html
    // http://www.sccg.sk/~samuelcik/rtg/Lecture6.pdf
    // http://blog.bonzaisoftware.com/tnp/gl-water-tutorial/

    // vec2 texCoord = vec2(0.5 + atan(Position.z, Position.x)/(2.0 * M_PI), 0.5 - asin(Position.y)/M_PI);
    // vec3 NormalMap = texture2D(normalmap, texCoord).rgb;
    // vec3 N = normalize(NormalMap * 2.0 - 1.0);
    // vec3 V = normalize(normalize(Position));
    // vec3 PN = perturb_normal(N, V, texCoord);

    vec3 vNormal = vertex.Normal.xyz;
    vec3 vONormal = vertex.ONormal.xyz;
    vec4 vPosition = vertex.Position;
    vec4 vOPosition = vertex.OPosition;
    vec3 vU = vertex.UV.xyz;

    vec3 n = normalize( vONormal.xyz );
    vec3 blend_weights = abs( n );
    blend_weights = ( blend_weights - 0.2 ) * 7.;
    blend_weights = max( blend_weights, 0. );
    blend_weights /= (blend_weights.x + blend_weights.y + blend_weights.z);

    vec2 coord1 = vPosition.yz * texScale;
    vec2 coord2 = vPosition.zx * texScale;
    vec2 coord3 = vPosition.xy * texScale;

    vec3 bump1 = texture2D(normalmap, coord1).rgb;
    vec3 bump2 = texture2D(normalmap, coord2).rgb;
    vec3 bump3 = texture2D(normalmap, coord3).rgb;

    vec3 blended_bump = bump1 * blend_weights.xxx +
                      bump2 * blend_weights.yyy +
                      bump3 * blend_weights.zzz;

    vec3 tanX = vec3(vNormal.x, -vNormal.z, vNormal.y);
    vec3 tanY = vec3(vNormal.z, vNormal.y, -vNormal.x);
    vec3 tanZ = vec3(-vNormal.y, vNormal.x, vNormal.z);
    vec3 blended_tangent = tanX * blend_weights.xxx +
                         tanY * blend_weights.yyy +
                         tanZ * blend_weights.zzz;

    vec3 normalTex = blended_bump * 2.0 - 1.0;
    normalTex.xy *= normalScale;
    normalTex.y *= -1.;
    normalTex = normalize( normalTex );
    mat3 tsb = mat3( normalize( blended_tangent ),
                   normalize( cross( vNormal, blended_tangent ) ),
                   normalize( vNormal ) );
    vec3 finalNormal = tsb * normalTex;

    vec3 r = reflect( normalize( vU ), normalize( finalNormal ) );
    float m = 2.0 * sqrt( r.x * r.x + r.y * r.y + ( r.z + 1.0 ) * ( r.z + 1.0 ) );
    vec2 calculatedNormal = vec2( r.x / m + 0.5,  r.y / m + 0.5 );

    // refraction
    float ratio = 1.00 / 1.33;
    vec3 refractionI = normalize(Position - cameraPos);
    vec3 refractionR = refract(refractionI, normalize( finalNormal ), ratio);
    vec4 refractionColor = texture(skybox, refractionR);

    vec3 reflectionI = normalize(Position - cameraPos);
    vec3 reflectionR = reflect(reflectionI, normalize( normalTex ));
    vec4 reflectionColor = texture(skybox, reflectionR);

    vec3 base = refractionColor.xyz;
    //vec3 base = texture2D( texturemap, calculatedNormal ).rgb;

    // float rim = 1.75 * max( 0., abs( dot( normalize( vNormal ),
    //                      normalize( -vOPosition.xyz ) ) ) );
    // base += useSSS * baseColor * ( 1. - .75 * rim );
    // base += ( 1. - useSSS ) * 10. * base * baseColor * clamp( 1. - rim, 0., .15 );

    // if (useScreen == 1.)
    // {
    //     base = vec3( 1. ) - ( vec3( 1. ) - base ) * ( vec3( 1. ) - base );
    // }

    // float nn = .05 * random( vec3( 1. ), length( gl_FragCoord ) );
    // base += vec3( nn );

    vec4 FragColor = vec4( base.rgb, 1. );

	// reflection
	// vec3 reflectionI = normalize(Position - cameraPos);
 //    vec3 reflectionR = reflect(reflectionI, Normal);
 //    vec4 reflectionColor = texture(skybox, reflectionR);

    // refraction
    // float ratio = 1.00 / 1.33;
    // vec3 refractionI = normalize(Position - cameraPos);
    // vec3 refractionR = refract(refractionI, Normal, ratio);
    // vec4 refractionColor = texture(skybox, refractionR);

    //fresnel = 0.12 + ( 0.88 * pow2( 1.0 - abs( dot(normal, viewVec) ) ) );
    //float fresnel = dot(Normal, normalize(Position));
    //color = (1.0 - fresnel) * reflectionColor + fresnel * refractionColor;
    //vec4 finalColor = mix(reflectionColor, refractionColor, fresnel);
    //finalColor.a = 0.5 + 0.5*fresnel;
    color = FragColor;
}
*/
{
    // reflection
    vec3 reflectionI = normalize(Position - cameraPos);
    vec3 reflectionR = reflect(reflectionI, Normal);
    vec4 reflectionColor = texture(skybox, reflectionR);

    // refraction
    float ratio = 1.00 / 1.33;
    vec3 refractionI = normalize(Position - cameraPos);
    vec3 refractionR = refract(refractionI, Normal, ratio);
    vec4 refractionColor = texture(skybox, refractionR);

    // fresnel = 0.12 + ( 0.88 * pow2( 1.0 - abs( dot(normal, viewVec) ) ) );
    float fresnel = dot(Normal, normalize(Position));
    // color = (1.0 - fresnel) * reflectionColor + fresnel * refractionColor;
    vec4 finalColor = mix(reflectionColor, refractionColor, fresnel);
    finalColor.a = 0.5 + 0.5*fresnel;

    color = finalColor;
}
