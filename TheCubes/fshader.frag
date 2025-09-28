#version 460 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform float myKernel[11];
uniform sampler2D screenTexture;
uniform sampler2D depthTexture;
uniform sampler3D cloudTexture;

uniform vec3 cameraViewDirection;
uniform vec3 cameraPos;
uniform vec3 cameraSunDirection;
uniform mat4 transform;
uniform float threshhold;
uniform float scale;
uniform float time;

const vec3 sunColor = vec3(1,1,1); 
const vec3 box1 = vec3(0,0,0);
const vec3 box2 = vec3(7,7,7);

int hash(vec3 p) {
    p.x = int(p.x)%64;
    p.y = int(p.y)%64;
    p.z = int(p.z)%64;
    return int(fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453) * 256.0);
}

mat3 rotationMatrixFromDirection(vec3 dir) {
    vec3 forward = normalize(dir);
    vec3 up = vec3(0.0, 1.0, 0.0);
    // If forward is parallel to up, pick another up vector
    if (abs(dot(forward, up)) > 0.999) up = vec3(1.0, 0.0, 0.0);
    vec3 right = normalize(cross(up, forward));
    up = normalize(cross(forward, right));
    return mat3(right, up, forward);
}

vec2 rayBoxDst(vec3 boundsMin, vec3 boundsMax, vec3 rayOrigin, vec3 invRaydir) 
{
    vec3 t0 = (boundsMin - rayOrigin) * invRaydir;
    vec3 t1 = (boundsMax - rayOrigin) * invRaydir;
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);   
    float dstA = max(max(tmin.x, tmin.y), tmin.z);
    float dstB = min(tmax.x, min(tmax.y, tmax.z));
    float dstToBox = max(0, dstA);
    float dstInsideBox = max(0, dstB - dstToBox);
    return vec2(dstToBox, dstInsideBox);
}
const float mult = 1.f/(1.f-threshhold);
float sampleCube(vec3 pointRel)
{
    float density = texture(cloudTexture,(pointRel/scale)%1).r;
    if(density<threshhold)
        density = 0;
    density = (density-threshhold)*mult;
    return min(1,max(0,density));
}

float lightCast(vec3 startRel, vec3 rayDirection, float distance, int samplePoints)
{
    float density = 0;
    float step = distance/samplePoints;
    for(int i = 0; i<samplePoints; i++)
    {
        vec3 currentPoint = startRel+rayDirection*i*step;
        density += sampleCube(currentPoint);
    }
    return density*step;
}

vec2 sampleRayThroughCube(vec3 startRel, vec3 rayDirection, float distance, int samplePoints)
{
    float density = 0;
    float lightPercent = 0;
    vec3 invSun = 1.0 / cameraSunDirection;
    float step = distance/samplePoints;
    for(int i = 0; i<samplePoints; i++)
    {
        vec3 currentPoint = startRel+rayDirection*i*step;
        float cDensity = sampleCube(currentPoint);
        density += cDensity * step;
        if(cDensity>0)
        {
            vec2 something = rayBoxDst(vec3(0,0,0),box2-box1,currentPoint,invSun);
            if(something.y>0)
            {
                float lightDist = lightCast(currentPoint,cameraSunDirection,something.y,5) + density;
                lightPercent += exp(-lightDist) * cDensity * step;
            }
            else
            {
                lightPercent += density * cDensity * step;
            }
        }
    }
    return vec2(exp(-density),lightPercent);
}

vec2 square(vec2 number)
{
    float real = number.x*number.x-number.y*number.y;
    float imag = number.y*number.x*2;
    return vec2(real,imag);
}
bool isIn(vec2 c)
{
    vec2 num = vec2(0,0);
    for(int i = 0; i<800; i++)
    {
        num = square(num)+c;
        if(length(num)>100000)
            return false;
    }
    return true;
}

void main()
{ 
    vec2 uv = gl_FragCoord.xy/vec2(1920,1080)*2.0-1.0;
    uv.x*=-1920./1080;
    vec3 rd = normalize(vec3(uv,1));
    rd=rotationMatrixFromDirection(cameraViewDirection)*rd;
    const float offsetx = 1.0f / 1920.0 * 1;
    const float offsety = 1.0f / 1080.0 * 1; 
    vec2 offsets[9] = vec2[](
        vec2(-offsetx,  offsety), // top-left
        vec2( 0.0f,    offsety), // top-center
        vec2( offsetx,  offsety), // top-right
        vec2(-offsetx,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offsetx,  0.0f),   // center-right
        vec2(-offsetx, -offsety), // bottom-left
        vec2( 0.0f,   -offsety), // bottom-center
        vec2( offsetx, -offsety)  // bottom-right    
    );
    float kernel[9];
    for(int i = 0; i<9; i++)
    {
        kernel[i] = myKernel[i];
    }
    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        vec2 cord = TexCoords.st + offsets[i];
        if(cord.x < 0.0 || cord.x > 1.0 || cord.y < 0.0 || cord.y > 1.0)
            sampleTex[i] = vec3(0.0f,0.0f,0.0f);
        else
        {
            vec3 tempTex = vec3(texture(screenTexture, cord));
            if(myKernel[9] == 1.0 && tempTex.r + tempTex.g + tempTex.b >= 0.02)
            {
                tempTex = vec3(1,1,1);
            }
            sampleTex[i] = tempTex;
        }
    }
    vec3 tcol = vec3(0.0);
    for(int i = 0; i < 9; i++)
        tcol += sampleTex[i] * kernel[i];
    
    vec3 col = texture(screenTexture,TexCoords.st).xyz;
    float depth = texture(depthTexture,TexCoords).r;
    if (depth==1)
    {
        float dotProd = dot(cameraSunDirection,rd);
        dotProd=max(0,dotProd-0.999);
        dotProd*=1000;
        tcol += smoothstep(0,1,dotProd)*sunColor;
    }
    FragColor = vec4(tcol, 1.0);
    if((abs(uv.y)<0.003&&abs(uv.x)<0.015)||(abs(uv.x)<0.003&&abs(uv.y)<0.015))
    {
        FragColor = vec4(vec3(1)-tcol,1);
    }
    //FragColor.r = gl_FragCoord.x/1920; 
    //vec2 something = rayBoxDst(vec3(0,0,0),vec3(1,1000,1),cameraPos,1.0 / max(abs(rd), vec3(1e-6)) * sign(rd));
    /*vec2 something = rayBoxDst(box1,box2,cameraPos,1.0 / (rd));
    vec3 hitPos = cameraPos + rd * something.x;
    vec4 clipPos = transform * vec4(hitPos, 1.0);
    clipPos /= clipPos.w;

    float boxDepth = (clipPos.z * 0.5) + 0.5;
    if(something.y>0&&((something.x==0)||(boxDepth<depth)))
    {
        vec2 somethingElse = sampleRayThroughCube(hitPos-box1,rd,something.y,20);
        FragColor.xyz = FragColor.xyz*somethingElse.x + sunColor * somethingElse.y;
    }
    FragColor.xyz = vec3(texture(cloudTexture,vec3(TexCoords.x,cameraPos.z,TexCoords.y)));*/
}