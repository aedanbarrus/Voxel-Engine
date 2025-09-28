#version 460 core
out vec4 FragColor;
in vec3 ourColor;
in vec3 truePos;
in vec3 basePos;
in vec3 normal;
uniform vec3 sunDirection;
uniform vec3 sunColor;
uniform vec3 cameraPos;
uniform vec3 cameraCol;
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
    float hash = hash(basePos*16)/255.0f;
    FragColor = vec4(ourColor,1.0);
    int count = 0;
    float dist = distance(cameraCol,basePos);
    FragColor.xyz *= (ambient(sunColor,0.1)+(0.4+hash/20.0f)*diffuse(sunColor,normal,sunDirection)+specular(sunColor,0.2+hash/2.0f,normal,sunDirection,cameraPos,truePos));
    if(dist<=0.01)
    {
        dist = int((dist*dist)*100000)/10.0;
        FragColor = vec4(0,1,0,1)*dist+(1-dist)*FragColor;
    }
}