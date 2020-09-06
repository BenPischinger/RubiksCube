#version 330 core
out vec4 FragColor;

struct Material 
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
	sampler2D diffuseMap;
}; 

struct DirLight 
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight 
{
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NR_POINT_LIGHTS 6
uniform PointLight pointLights[NR_POINT_LIGHTS];

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform DirLight dirLight;

uniform Material material;

vec3 diffuse;
vec3 ambient;
vec3 specular;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{    
	vec3 pixel = vec3(texture(material.diffuseMap, TexCoords));
	
	if(all(lessThanEqual(pixel, vec3(1.1,1.1,1.1))) && (all(greaterThanEqual(pixel, vec3(0.9,0.9,0.9))) && material.diffuse != vec3(0,0,0)))
	{
		ambient = material.ambient;
		diffuse = material.diffuse;
		specular = material.specular;
	}
	else
	{
		ambient = vec3(0,0,0);
		diffuse = vec3(0,0,0);
		specular = material.specular;
	}
	
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);
    
	vec3 result = CalcDirLight(dirLight, norm, viewDir);

	for(int i = 0; i < NR_POINT_LIGHTS; i++)
	{
		result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);    
	}
	
	// Color Inversion
	//FragColor = vec4(result + vec3(1.0 - texture(material.diffuseMap, TexCoords)), 1.0);
	
	// Grey Scale
    //float average = 0.2126 * result.r + 0.7152 * result.g + 0.0722 * result.b;
    //FragColor = vec4(average, average, average, 1.0);
	
	// Gamma Correction & Diffuse
	//vec3 diffuseColor = pow(texture(material.diffuseMap, texCoords).rgb, vec3(gamma));
	
	float gamma = 2.2;
	FragColor = vec4(result, 1.0);
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
	
    //FragColor = vec4(result, 1.0);
	
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
	// diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
	// specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	// combine results
    vec3 ambient = light.ambient * ambient;
    vec3 diffuse = light.diffuse * diff * diffuse;
    vec3 specular = light.specular * spec * specular;
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
	// diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
	// specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	// attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    vec3 ambient = light.ambient * ambient;
    vec3 diffuse = light.diffuse * diff * diffuse;
    vec3 specular = light.specular * spec * specular;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}