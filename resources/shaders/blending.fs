#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;



in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{
    vec4 texColor = texture(texture1, TexCoords);
    if (texColor.a < 0.1)
        discard;

    float brightness = dot(texColor.xyz, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
       BrightColor = vec4(texColor);
    else
       BrightColor = vec4(0.0, 0.0, 0.0, 1.0);


 FragColor = texColor;
}