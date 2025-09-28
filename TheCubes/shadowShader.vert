#version 430 core
layout (location = 0) in vec3 aBasePos;
layout (location = 1) in int data;

uniform mat4 lightSpaceMatrix;
uniform vec3 chunkOffset;

void main()
{
	int x = data & 31;
	int y = (data>>5) & 31;
	int z = (data>>10) & 31;
	int face = (data>>15) & 7;
	int textureID = (data>>18) & 127;
	int width = ((data>>25) & 31) + 1;
	int height = ((data>>30) & 31) + 1;
	vec3 basePos = aBasePos;
	if (face % 2 == 0)
	{
		basePos.y+=1;
		basePos.xyz=basePos.zyx;
	}
	if (face == 1 || face == 0)
	{
		basePos.x*=width;
	}
	if (face == 2 || face == 3)
	{
		basePos.xyz = basePos.yzx;
		basePos.z *= width;
	}
	if (face == 4 || face == 5)
	{
		basePos.xyz = basePos.zxy;
		basePos.x*=width;
	}
    gl_Position = lightSpaceMatrix * vec4(vec3(x,y,z)+basePos+chunkOffset, 1.0);
} 