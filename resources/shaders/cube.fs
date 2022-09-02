#version 330 core


struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;

    float cutOff;
    float outerCutoff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

in VS_OUT {
    vec3 Normal;
    vec2 TexCoord;
    vec3 FragPos;
} fs_in;

uniform vec3 viewPos;
uniform SpotLight spotLight;
uniform DirLight dirLight;
uniform int numOfPointLights;
uniform PointLight pointLights[3];

uniform sampler2D cubeTexture;
uniform sampler2D goldTexture;
uniform bool isPoint;


out vec4 FragColor;

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 result;
    vec3 normal = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    result = calcDirLight(dirLight, normal, viewDir);
    for(int i =  0; i < numOfPointLights; ++i) {
          result+= calcPointLight(pointLights[i],normal,fs_in.FragPos.xyz,viewDir);
    }
   result += calcSpotLight(spotLight, normal, fs_in.FragPos, viewDir);

    FragColor = vec4(result,1.0);
}

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec4 texColor = isPoint ? texture(goldTexture, fs_in.TexCoord): texture(cubeTexture, fs_in.TexCoord);
    // smer padanja svetlosti
    vec3 lightDir = normalize(-light.direction);
    //difuzna komponenta
    float diff = max(dot(normal, lightDir), 0.0);
    //specularna
    // po blinu
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);

    vec3 ambient = light.ambient * vec3(texColor);
    vec3 diffuse = light.diffuse * diff * vec3(texColor);
    vec3 specular = light.specular * spec;
    return (ambient + diffuse + specular);
}


vec3 calcPointLight (PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec4 texColor = isPoint ? texture(goldTexture, fs_in.TexCoord): texture(cubeTexture, fs_in.TexCoord);

    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal,lightDir),0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    //attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    vec3 ambient = light.ambient * vec3(texColor);
    vec3 diffuse = light.diffuse * diff * vec3(texColor);
    vec3 specular = light.specular * spec * vec3(texColor);

     ambient *= attenuation;
     diffuse *= attenuation;
     specular *= attenuation;


    return (ambient + diffuse + specular);

}


vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec4 texColor = isPoint ? texture(goldTexture, fs_in.TexCoord): texture(cubeTexture, fs_in.TexCoord);

    vec3 lightDir = normalize(light.position - fragPos);
    float theta = dot(lightDir, normalize(-light.direction));
    //diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    //specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    //attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);


    float epsilon = light.cutOff - light.outerCutoff;
    float intensity = clamp((theta - light.outerCutoff)/epsilon, 0.0, 1.0);

    vec3 ambient = light.ambient * vec3(texColor);
    vec3 diffuse = light.diffuse * diff * vec3(texColor);
    vec3 specular = light.specular * spec * vec3(texColor);

    ambient *= attenuation;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}