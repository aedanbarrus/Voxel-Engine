#pragma once
#ifndef World
#define World
#include <string>
#include <iostream>
#include <bit>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "SHADER.h"
#include "Physics.h"
#include <bitset>
#include <cstddef>
#include "Particle_System.h"

class Chunk
{
public:
	glm::vec3 worldPos;
	bool changed;
	int blockCount;
	unsigned int quadVAO, quadVBO, dataVBO, IBO, SSBO;
	unsigned char voxels[32 * 32 * 32];
	std::vector<uint32_t> instanceData;
	uint32_t indirectBuffer[6][4];
	Chunk();
	unsigned short getVoxel(int x, int y, int z);
	unsigned short getVoxel(int i);
	void setVoxel(int x, int y, int z, short type);
	void setVoxel(int i, short type);
	void generateMesh();
	void generateMesh2();
	void draw(Shader& shader);
	void reset();
private:
	void addVertex(std::vector<uint32_t>& instanceData, int x, int y, int z, int textureID, int width, int height);
};

class Earth
{
public:
	bool changed;
	std::vector<uint32_t> instanceData;
	std::vector<uint32_t[4]> indirectBuffer;
	std::vector<PhysicObject> physicObjects;
	std::vector<Chunk> quadrantI, quadrantII, quadrantIII, quadrantIV;
	ParticleSystem particles;
	Earth();
	void setVoxel(int x, int y, int z, int type, bool genMesh);
	int getVoxel(int x, int y, int z);
	void fill(int x1, int y1, int z1, int x2, int y2, int z2, int type);
	void draw(Shader& shader, glm::vec3 camPos);
	void step(const float deltaTime);
	void addParticle(ParticleType particle);
	void genParticle(ParticleGenerator& particle, float dt);
	void updateMesh();
	void updateAllMeshes();
	void getCollisions(CollisionData& data);
	bool isValidBlock(glm::vec3 pos);
	void raycast(int fill);
};

#endif