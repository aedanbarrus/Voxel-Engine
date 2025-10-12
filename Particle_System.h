#pragma once
#pragma once
#ifndef Particle_System
#define Particle_System
#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "SHADER.h"
#include <bitset>
#include <cstddef>
#include <ctime>
#include <random>
#include "Physics.h"
#include "camera.h"

struct ParticleType
{
	glm::vec3 pos;
	glm::vec3 velocity, velocityVar;
	glm::vec4 beginColor, endColor, colorVar;
	glm::vec3 acceleration;
	float beginSize, endSize, sizeVar;
	float lifetime = 1.0;
	float drag = 1.0;
	struct ParticleGenerator* generator = nullptr;
};
struct ParticleGenerator
{
	ParticleType type;
	float timeBetweenParticles;
	float timePassed;
	int numberPerCycle;
};

struct AttachedParticleGenerator
{
	ParticleType type;
	bool end;
	float time;
	int numberPerCycle;
};

class ParticleSystem
{
public:
	ParticleSystem();
	void createParticle(ParticleType& newParticle);
	void genParticle(ParticleGenerator& newParticle, float dt);
	void draw();
	void step(float deltaTime);
	void collide(PhysicObject& Object);
private:
	struct Particle
	{
		glm::vec3 pos;
		glm::vec3 velocity;
		glm::vec4 beginColor, endColor;
		glm::mat4 rotation;
		glm::mat4 rotationWithPos;
		glm::vec3 acceleration;
		float beginSize, endSize;
		float lifetime = 1.0;
		float timeLeft = 0.0;
		float drag = 1.0;
		bool active = true;
		ParticleGenerator* generator = nullptr;
	};
	struct ParticleInstanceData
	{
		glm::vec3 pos;
		glm::vec4 color;
		float size;
		glm::mat4 rotation;
	};
	std::vector<Particle> myParticles;
	int currentParticle = 9999;
	Shader myShader;
	GLuint quadVAO, instanceVBO;
	std::vector<ParticleInstanceData> instanceData;
};
float random();
glm::vec3 randomVec(glm::vec3 myVec);
glm::vec4 randomVec(glm::vec4 myVec);
#endif