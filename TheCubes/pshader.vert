#version 460 core

struct ParticleInstanceData
{
	vec3 pos;
	vec4 color;
	float size;
	mat4 rotation;
};

layout (location = 0) in vec3 aBasePos;
layout (location = 1) in vec3 instancePos;
layout (location = 2) in vec4 instanceColor;
layout (location = 3) in float instanceSize;
layout (location = 4) in vec4 instanceRotation0;
layout (location = 5) in vec4 instanceRotation1;
layout (location = 6) in vec4 instanceRotation2;
layout (location = 7) in vec4 instanceRotation3;


uniform mat4 rotMat;
uniform mat4 transform;
uniform vec3 pos;
uniform float size;
out vec4 color;

void main()
{
	gl_Position = rotMat * (mat4(instanceRotation0,instanceRotation1,instanceRotation2,instanceRotation3) * vec4(aBasePos*instanceSize, 1.0) + vec4(instancePos,0.0));
	color = instanceColor;
}