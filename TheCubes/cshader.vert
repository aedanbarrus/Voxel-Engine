#version 460 core
layout (location = 0) in vec3 aBasePos;
layout (location = 1) in int data;
layout(std430,binding=0) buffer myFaces {
	int faces[];
};
uniform mat4 rotMat;
uniform vec3 chunkOffset;
uniform int myFace;
out vec3 ourColor;
out vec3 truePos;
out vec3 Normal;
out float myTexture;
void main()
{
	int x = data & 31;
	int y = (data>>5) & 31;
	int z = (data>>10) & 31;
	int textureID = (data>>15) & 127;
	int width = ((data>>22) & 31) + 1;
	int height = ((data>>27) & 31) + 1;
	vec3 basePos = aBasePos;
	int face = faces[gl_DrawID];
	if (face % 2 == 0)
	{
		basePos.y+=1;
		basePos.xyz=basePos.zyx;
	}
	if (face == 1 || face == 0)
	{
		basePos.x*=width;
		basePos.z*=height;
	}
	//if (face == 0)
	//	basePos.xyz = basePos.zyx;
	if (face == 2 || face == 3)
	{
		basePos.xyz = basePos.yzx;
		basePos.z *= width;
		basePos.y *= height;
	}
	if (face == 4 || face == 5)
	{
		basePos.xyz = basePos.zxy;
		basePos.x*=width;
		basePos.y*=height;
	}
	truePos = vec3(x,y,z)+basePos+chunkOffset;
	gl_Position = rotMat * vec4(truePos, 1.0);
	
	if(face==0)
		Normal = vec3(0.0f,1.0f,0.0f);
	if(face==1)
		Normal = vec3(0.0f,-1.0f,0.0f);
	if(face==2)
		Normal = vec3(1.0f,0.0f,0.0f);
	if(face==3)
		Normal = vec3(-1.0f,0.0f,0.0f);
	if(face==4)
		Normal = vec3(0.0f,0.0f,1.0f);
	if(face==5)
		Normal = vec3(0.0f,0.0f,-1.0f);
	ourColor = vec3(0.71,0.714,0.729);
	if(textureID == 2 && face == 0) 
		ourColor = vec3(0.19,1.0,0.21);
	myTexture = textureID;
}