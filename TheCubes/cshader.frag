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
    float hash = hash(truePos*16)/255.0f;
    vec3 norm = normalize(Normal);
    if(myTexture==1)
    {
        specularStrength = 0.7;
        diffuseStrength+=hash/30;
        specularStrength+=pow(hash,8.0) * 2;
    }
    FragColor = vec4(ourColor*(hash/8+0.875),1);
    //FragColor = vec4(ourColor,1);
    FragColor.xyz *= (ambient(sunColor,ambientStrength)+diffuseStrength*diffuse(sunColor,Normal,sunDirection)+specular(sunColor,specularStrength,Normal,sunDirection,cameraPos,truePos));
    //FragColor.xyz = vec3(gl_FragCoord.w,0,0);
    //gl_FragDepth = gl_FragCoord.w-1;
}