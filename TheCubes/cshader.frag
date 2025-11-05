#version 460 core
out vec4 FragColor;
in vec3 ourColor;
in vec3 Normal;
in vec3 truePos;
in float myTexture;
float specularStrength = 0.1;
float ambientStrength = 0.3;
float diffuseStrength = 1;
uniform vec3 sunDirection;
uniform vec3 sunColor;
uniform vec3 cameraPos;

int hash(vec3 p) {
    p.x = int(p.x)%64;
    p.y = int(p.y)%64;
    p.z = int(p.z)%64;
    return int(fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453) * 256.0);
}
vec3 ambient(vec3 color, float strength)
{
    return strength*color;
}
vec3 diffuse(vec3 color, vec3 normal, vec3 lightDirection)
{
    return max(dot(normal,lightDirection),0.0) * color;
}
vec3 specular(vec3 color, float strength, vec3 normal, vec3 lightDirection, vec3 cameraPos, vec3 FragPos)
{
    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightDirection, normal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    return strength * spec * color;  
}
void main()
{
    vec3 color = ourColor;
    float hash = hash(truePos*16)/255.0f;
    vec3 norm = normalize(Normal);
    if(myTexture==1)
    {
        specularStrength = 0.7;
        diffuseStrength+=hash/30;
        specularStrength+=pow(hash,8.0) * 2;
    }
    if(myTexture==2 &&  fract(truePos.y)!=0.0f && fract(truePos.y)>9.0f/16.0f)
    {
        float chance = 0.0f;
        if(fract(truePos.y)>10.0f/16.0f)
            chance = 0.1f;
        if(fract(truePos.y)>11.0f/16.0f)
            chance = 0.3f;
        if(fract(truePos.y)>12.0f/16.0f)
            chance = 0.5f;
        if(fract(truePos.y)>13.0f/16.0f)
            chance = 0.7f;
        if(fract(truePos.y)>14.0f/16.0f)
            chance = 0.9f;
        if(fract(truePos.y)>15.0f/16.0f)
            chance = 1.0f;
        if(hash(truePos*16)/255.0f<chance)
            color = vec3(8, 128, 38)/256.0f;
            
    }
    FragColor = vec4(color*(hash/8+0.875),1);
    FragColor.xyz *= (ambient(sunColor,ambientStrength)+diffuseStrength*diffuse(sunColor,Normal,sunDirection)+specular(sunColor,specularStrength,Normal,sunDirection,cameraPos,truePos));
}