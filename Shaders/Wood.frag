#version 330

#define SAMPLE_RATE 0.025

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	sampler2D diffuseTex;
	sampler2D shadowMap;
};

struct PointLight
{
	vec3 position;
	float intensity;
	vec3 color;
	float constant;
	float linear;
	float quadratic;
};

//input from vertex shader
in vec3 vs_position;
in vec3 vs_color;
in vec2 vs_texcoord;
in vec3 vs_normal;
in vec4 vs_fragPosLightSpace;

//output of fragment shadersendtoshader
out vec4 color;

//Uniforms

uniform Material material;
uniform PointLight pointLight;
uniform vec3 lightPos0;
uniform vec3 cameraPos;

//Functions

float ShadowCalculation(vec4 fragPosLightSpace)
{
	//return 1.0;

    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

	if(projCoords.z > 1.0)
	{
		 float shadow = 0.0;
		 return shadow;
	}
    

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(material.shadowMap, projCoords.xy).r;   //texture(material.specularTex, projCoords.xy).r;  
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;


	float bias = max(0.05 * (1.0 - dot(vs_normal, normalize(lightPos0 - vs_position))), 0.005);  


    // check whether current frag pos is in shadow
    //float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;


	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(material.shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
	for(int y = -1; y <= 1; ++y)
	{
		float pcfDepth = texture(material.shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
		shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
	}    
	}
	shadow /= 9.0;

    return shadow;
}  


vec3 calculateAmbient(Material material)
{
	return material.ambient * 2.0;
}

vec3 calculateDiffuse(Material material, vec3 vs_position, vec3 vs_normal, vec3 lightPos0)
{
	vec3 posToLightDirVec = normalize(lightPos0 - vs_position);
	float diffuse = clamp(dot(posToLightDirVec, normalize(vs_normal)), 0, 1);
	vec3 diffuseFinal = material.diffuse * diffuse * 0.5;

	return diffuseFinal;
}

vec3 calculateSpecular(Material material, vec3 vs_position, vec3 vs_normal, vec3 lightPos0, vec3 cameraPos)
{
	vec3 lightToPosDirVec = normalize(vs_position - lightPos0);
	vec3 reflectDirVec = normalize(reflect(lightToPosDirVec, normalize(vs_normal)));
	vec3 posToViewDirVec = normalize(cameraPos - vs_position);
	float specularConstant = pow(max(dot(posToViewDirVec, reflectDirVec), 0), 35);
	vec3 specularFinal = material.specular * specularConstant;

	return specularFinal;
}


//--------------------------------------------------------------------------------------------------------------------------------
//Noise
//--------------------------------------------------------------------------------------------------------------------------------



vec2 random2(vec2 st)
{
    st = vec2( dot(st,vec2(127.1,311.7)),
              dot(st,vec2(269.5,183.3)) );
    return -1.0 + 2.0*fract(sin(st)*43758.5453123);
}


float noise(vec2 st) 
{
    vec2 i = floor(st);
    vec2 f = fract(st);

    vec2 u = f*f*(3.0-2.0*f);

    return mix( mix( dot( random2(i + vec2(0.0,0.0) ), f - vec2(0.0,0.0) ),
                     dot( random2(i + vec2(1.0,0.0) ), f - vec2(1.0,0.0) ), u.x),
                mix( dot( random2(i + vec2(0.0,1.0) ), f - vec2(0.0,1.0) ),
                     dot( random2(i + vec2(1.0,1.0) ), f - vec2(1.0,1.0) ), u.x), u.y);
}

mat2 rotate2d(float angle)
{
     return mat2(cos(angle),sin(angle),
                -sin(angle),cos(angle));

	 //return mat2(cos(angle + 0.5 * 3.14159), -sin(angle + 0.5 * 3.14159),
                //sin(angle + 0.5 * 3.14159), cos(angle + 0.5 * 3.14159));
}

float lines(in vec2 pos, float b)
{
    float scale = 10.0;
    pos *= scale;
    return smoothstep(0.0,
                    .5+b*.5,
                    abs((sin(pos.x*3.1415)+b*2.0))*.5);
}




//--------------------------------------------------------------------------------------------------------------------------------
//Bump map using the noise function
//--------------------------------------------------------------------------------------------------------------------------------



vec4 bump_map(vec2 uv)
{	

	float n = noise(uv);
	vec2 du = vec2(SAMPLE_RATE,0.0);
	vec2 dv = vec2(0.0,SAMPLE_RATE);
	float dx = noise(uv + du) - n;
	float dy = noise(uv + dv) - n;
	vec3 bump = normalize(cross(vec3(dx,dy,1.0), vec3(dy,-dx,1.0)));
	return vec4(bump * 0.5 + 0.5, n);
}



//--------------------------------------------------------------------------------------------------------------------------------
//Main
//--------------------------------------------------------------------------------------------------------------------------------




void main()
{
	

	vec2 pos = vs_texcoord.yx*vec2(20., 40.);

	float pattern = pos.x;
	// Add noise
	pos = rotate2d( noise(pos) ) * pos;
	//pos = vec2(pos.y, pos.x); 
	// Draw lines
	
	pattern = lines(pos.yx,.5) * 10;
	pattern = clamp(pattern,0.0,1.0);
	vec4 diffuse_color = mix( vec4(90.0/255.0,34.0/255.0,0.0,1.0),vec4(107.0/255.0,42.0/255.0,22.0/255.0,1.0), pattern);

	//color = vec4(1.,1.,1.,1.) * pattern;

	//ShadowCalculation
	float shadow = ShadowCalculation(vs_fragPosLightSpace);


	//Bump Map
	//vec2 scaled_uv = vec2(vs_texcoord*1000.0);
	//vec4 bump_output = bump_map(scaled_uv);
	//vec3 normal = normalize(bump_output.rgb * 2.0 - 1.0);
	//vec3 preturbedNormal = normalize(vs_normal + (normal * 0.02));


	//Ambient light
	vec3 ambientFinal = calculateAmbient(material);

	//Diffuse light
	vec3 diffuseFinal = calculateDiffuse(material, vs_position, vs_normal, pointLight.position);
	//depth_Test
	//Specular light
	vec3 specularFinal = calculateSpecular(material, vs_position, vs_normal, pointLight.position, cameraPos);

	//Attenuation
	float distance = length(pointLight.position - vs_position);
	//constant linear quadratic
	float attenuation = pointLight.constant / (1.f + pointLight.linear * distance + pointLight.quadratic * (distance * distance));

	//Final light
	ambientFinal *= attenuation;
	diffuseFinal *= attenuation;
	specularFinal *= attenuation;

	

	///Final Color
	color = diffuse_color * ((vec4(ambientFinal, 1.f) + (1.0-shadow) * (vec4(diffuseFinal, 1.f) + vec4(specularFinal.xyz, 1.f))) * pointLight.intensity); 
	//color = diffuse_color * (vec4(ambientFinal, 1.f) + (vec4(diffuseFinal, 1.f) + vec4(specularFinal, 1.f))); 
	//color = diffuse_color * (vec4(ambientFinal, 1.f));
	//+ (vec4(diffuseFinal, 1.f) + vec4(specularFinal, 1.f))); 
	//color = diffuseandspec; 


	
}
