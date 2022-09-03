#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
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

struct PointLight {
    vec3 position;

    vec3 ambient;
        vec3 diffuse;
        vec3 specular;

        float constant;
        float linear;
        float quadratic;

};


in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} fs_in;

uniform vec3 viewPos;
uniform sampler2D planeTexture;
uniform SpotLight spotLight;
uniform DirLight dirLight;
uniform int numOfPointLights;

uniform PointLight pointLights[3];

//out vec4 FragColor;

vec3 calcDirectionalLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcPointLight (PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 normal = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    vec3 result = calcDirectionalLight(dirLight, normal, viewDir);
    result += calcSpotLight(spotLight, normal, fs_in.FragPos, viewDir);

    for(int i =  0; i < 3; ++i) {
        result+= calcPointLight(pointLights[i],normal,fs_in.FragPos,viewDir);
    }

         float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
            if(brightness > 1.0)
                BrightColor = vec4(result, 1.0);
            else
                BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

        FragColor = vec4(result,1.0);

    FragColor = vec4(result, 1.0);
}

vec3 calcDirectionalLight(DirLight light, vec3 normal, vec3 viewDir)
{
    // smer padanja svetlosti
    vec3 lightDir = normalize(-light.direction);
    //difuzna komponenta
    float diff = max(dot(normal, lightDir), 0.0);
    //specularna
    // po blinu
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);

    vec3 ambient = light.ambient * vec3(texture(planeTexture, fs_in.TexCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(planeTexture, fs_in.TexCoord));
    vec3 specular = light.specular * spec;
    return (ambient + diffuse + specular);
}


vec3 calcPointLight (PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal,lightDir),0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    //attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    vec3 ambient = light.ambient * vec3(texture(planeTexture, fs_in.TexCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(planeTexture, fs_in.TexCoord));
    vec3 specular = light.specular * spec * vec3(texture(planeTexture, fs_in.TexCoord));

     ambient *= attenuation;
     diffuse *= attenuation;
     specular *= attenuation;


    return (ambient + diffuse + specular);

}


vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
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

    vec3 ambient = light.ambient * vec3(texture(planeTexture, fs_in.TexCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(planeTexture, fs_in.TexCoord));
    vec3 specular = light.specular * spec * vec3(texture(planeTexture, fs_in.TexCoord));

    ambient *= attenuation;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}