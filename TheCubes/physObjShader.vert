#version 460 core
layout (location = 0) in vec3 aBasePos;
layout (location = 1) in vec3 aNormal;

uniform mat4 rotMat;
uniform mat3 rotation;
uniform vec3 offset;
out vec3 ourColor;
out vec3 truePos;
out vec3 basePos;
out vec3 normal;
void main()
{
	truePos = rotation * aBasePos + offset;
	basePos = aBasePos;
	gl_Position = rotMat * vec4(truePos, 1.0);
	ourColor = vec3(1.0);
	normal = rotation * aNormal;
}