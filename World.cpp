#include "World.h"
float getTTime()
{
	return static_cast<float>(glfwGetTime());
}

Earth::Earth(){
	size_t chunkCount = 16 * 16 * 16;
	quadrantI.resize(chunkCount);
	quadrantII.resize(chunkCount);
	quadrantIII.resize(chunkCount);
	quadrantIV.resize(chunkCount);
	float t = getTTime();
	for (int i = 0; i< quadrantI.size(); i++)
	{
		quadrantI[i].worldPos = glm::vec3(i % 16 * 32, i / 16 % 16 * 32, i / 16 / 16 * 32);
		quadrantI[i].generate();
	}
	for (int i = 0; i < quadrantII.size(); i++)
	{
		quadrantII[i].worldPos = glm::vec3(i % 16 * -32 -32, i / 16 % 16 * 32, i / 16 / 16 * 32);
		quadrantII[i].generate();
	}
	for (int i = 0; i < quadrantIII.size(); i++)
	{
		quadrantIII[i].worldPos = glm::vec3(i % 16 * -32 - 32, i / 16 % 16 * 32, i / 16 / 16 * -32 -32);
		quadrantIII[i].generate();
	}
	for (int i = 0; i < quadrantIV.size(); i++)
	{
		quadrantIV[i].worldPos = glm::vec3(i % 16 * 32, i / 16 % 16 * 32, i / 16 / 16 * -32 -32);
		quadrantIV[i].generate();
	}
	std::cout << "Generating Terrain Time: " << getTTime() - t << std::endl;
	changed = false;
	particles = ParticleSystem();
}

void Earth::draw(Shader& shader, glm::vec3 camPos)
{
	glEnable(GL_CULL_FACE);
	shader.use();
	shader.setVec3("cameraPos", Camera::getCamera()->pos);
	shader.setVec3("sunDirection", Camera::getCamera()->sunDirection);
	shader.setVec3("sunColor", glm::vec3(1,1,1));
	float maxDist = 512;
	for (Chunk& c : quadrantI)
	{
		if (c.blockCount > 0 && glm::distance(camPos,c.worldPos+glm::vec3(16))< maxDist)
		{
			shader.setVec3("chunkOffset", c.worldPos);
			c.draw(shader);
		}
	}
	for (Chunk& c : quadrantII)
	{
		if (c.blockCount > 0 && glm::distance(camPos, c.worldPos + glm::vec3(16)) < maxDist)
		{
			shader.setVec3("chunkOffset", c.worldPos);
			c.draw(shader);
		}
	}
	for (Chunk& c : quadrantIII)
	{
		if (c.blockCount > 0 && glm::distance(camPos, c.worldPos + glm::vec3(16)) < maxDist)
		{
			shader.setVec3("chunkOffset", c.worldPos);
			c.draw(shader);
		}
	}
	for (Chunk& c : quadrantIV)
	{
		if (c.blockCount > 0 && glm::distance(camPos, c.worldPos + glm::vec3(16)) < maxDist)
		{
			shader.setVec3("chunkOffset", c.worldPos);
			c.draw(shader);
		}
	}
	for (PhysicObject c : physicObjects)
		c.draw();
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	particles.draw();
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
}

void Earth::setVoxel(int x, int y, int z, int type, bool genMesh)
{
	if(genMesh = false)
		changed = true;
	if (x >= 0 && z >= 0 && 0 <= y && y < 16 * 32 && x < 32 * 16 && z < 32 * 16)
	{
		quadrantI[x / 32 + y / 32 * 16 + z / 32 * 16 * 16].setVoxel(x % 32, y % 32, z % 32, type);
		if(genMesh)
			quadrantI[x / 32 + y / 32 * 16 + z / 32 * 16 * 16].generateMesh();
		return;
	}
	if (x < 0 && z >= 0 && 0 <= y && y < 16 * 32 && x >= -32 * 16 && z < 32 * 16)
	{
		quadrantII[(-x - 1) / 32 + y / 32 * 16 + z / 32 * 16 * 16].setVoxel(31 + (x+1) % 32, y % 32, z % 32, type);
		if (genMesh)
			quadrantII[(-x - 1) / 32 + y / 32 * 16 + z / 32 * 16 * 16].generateMesh();
		return;
	}
	if (x < 0 && z < 0 && 0 <= y && y < 16 * 32 && x >= -32 * 16 && z >= -32 * 16)
	{
		quadrantIII[(-x - 1) / 32 + y / 32 * 16 + (-z - 1) / 32 * 16 * 16].setVoxel(31 + (x+1) % 32, y % 32, 31 + (z+1) % 32, type);
		if (genMesh)
			quadrantIII[(-x - 1) / 32 + y / 32 * 16 + (-z - 1) * 16 * 16].generateMesh();
		return;
	}
	if (x >= 0 && z < 0 && 0 <= y && y < 16 * 32 && x < 32 * 16 && z >= -32 * 16)
	{
		quadrantIV[x / 32 + y / 32 * 16 + (-z - 1) / 32 * 16 * 16].setVoxel(x % 32, y % 32, 31 + (z+1) % 32, type);
		if (genMesh)
			quadrantIV[x / 32 + y / 32 * 16 + (-z - 1) / 32 * 16 * 16].generateMesh();
		return;
	}
}

int Earth::getVoxel(int x, int y, int z)
{
	if (x >= 0 && z >= 0 && 0 <= y && y < 16 * 32 && x < 32 * 16 && z < 32 * 16)
		return quadrantI[x / 32 + y / 32 * 16 + z / 32 * 16 * 16].getVoxel(x % 32, y % 32, z % 32);
	if (x < 0 && z >= 0 && 0 <= y && y < 16 * 32 && x >= -32 * 16 && z < 32 * 16)
		return quadrantII[(-x - 1) / 32 + y / 32 * 16 + z / 32 * 16 * 16].getVoxel(31 + (x + 1) % 32, y % 32, z % 32);
	if (x < 0 && z < 0 && 0 <= y && y < 16 * 32 && x >= -32 * 16 && z >= -32 * 16)
		return quadrantIII[(-x - 1) / 32 + y / 32 * 16 + (-z - 1) / 32 * 16 * 16].getVoxel(31 + (x + 1) % 32, y % 32, 31 + (z + 1) % 32);
	if (x >= 0 && z < 0 && 0 <= y && y < 16 * 32 && x < 32 * 16 && z >= -32 * 16)
		return quadrantIV[x / 32 + y / 32 * 16 + (-z - 1) / 32 * 16 * 16].getVoxel(x % 32, y % 32, 31 + (z + 1) % 32);
	return 0;
}

void Earth::fill(int x1, int y1, int z1, int x2, int y2, int z2, int type)
{
	int maxx = glm::max(x1, x2);
	int maxy = glm::max(y1, y2);
	int maxz = glm::max(z1, z2);
	for (int i = glm::min(x1, x2); i <= maxx; i++)
		for (int j = glm::min(y1, y2); j <= maxy; j++)
			for (int k = glm::min(z1, z2); k <= maxz; k++)
			{
				setVoxel(i, j, k, type, false);
			}
}

void Earth::updateMesh()
{
	for (Chunk& c : quadrantI)
		if (c.changed == true)
			c.generateMesh();
	for (Chunk& c : quadrantII)
		if (c.changed == true)
			c.generateMesh();
	for (Chunk& c : quadrantIII)
		if (c.changed == true)
			c.generateMesh();
	for (Chunk& c : quadrantIV)
		if (c.changed == true)
			c.generateMesh();
}

void Earth::updateAllMeshes()
{
	for (Chunk& c : quadrantI)
		if (c.blockCount != 0)
			c.generateMesh();
	for (Chunk& c : quadrantII)
		if (c.blockCount != 0)
			c.generateMesh();
	for (Chunk& c : quadrantIII)
		if (c.blockCount != 0)
			c.generateMesh();
	for (Chunk& c : quadrantIV)
		if (c.blockCount != 0)
			c.generateMesh();
}

void Earth::step(const float deltaTime)
{
	particles.step(deltaTime);
	//for (PhysicObject& c : physicObjects)
	//	c.step(deltaTime);
}

bool Earth::isValidBlock(glm::vec3 pos)
{
	return (pos.y >= 0 && pos.y < 16 * 32 && pos.x >= -32 * 16 && pos.x < 32 * 16 && pos.z >= -32 * 16 && pos.z < 32 * 16);
}
void Earth::raycast(int fill)
{
	glm::vec3 rayOrigin = Camera::getCamera()->pos;
	glm::vec3 rayDir = glm::normalize(Camera::getCamera()->direction);
	glm::ivec3 voxel = glm::floor(rayOrigin);
	glm::ivec3 ovoxel = voxel;
	glm::ivec3 step(
		(rayDir.x > 0) ? 1 : (rayDir.x < 0) ? -1 : 0,
		(rayDir.y > 0) ? 1 : (rayDir.y < 0) ? -1 : 0,
		(rayDir.z > 0) ? 1 : (rayDir.z < 0) ? -1 : 0
	);
	glm::vec3 tMax, tDelta;
	for (int i = 0; i < 3; ++i) {
		if (rayDir[i] != 0.0f) {
			float nextVoxelBoundary = voxel[i] + (step[i] > 0 ? 1.0f : 0.0f);
			tMax[i] = (nextVoxelBoundary - rayOrigin[i]) / rayDir[i];
			tDelta[i] = std::abs(1.0f / rayDir[i]);
		}
		else {
			tMax[i] = std::numeric_limits<float>::max();
			tDelta[i] = std::numeric_limits<float>::max();
		}
	}
	for (int i = 0; i < 256; ++i) {
		if (!isValidBlock(glm::vec3(voxel)))
			return;
		if (getVoxel(voxel.x, voxel.y, voxel.z) != 0) {
			if (fill == 0) {
				setVoxel(voxel.x, voxel.y, voxel.z, 0, 0);
				for (int i = 0; i < 20; i++)
				{
					int side = int(6.0 * random());
					glm::vec3 pos = glm::vec3(voxel) + glm::vec3(0.5);
					if (side == 0)
					{
						pos.x += 0.5f;
						pos.y += 0.5f * (random() - 1.0);
						pos.z += 0.5f * (random() - 1.0);
					}
					else if(side==1)
					{
						pos.x += 0.5f;
						pos.y += 0.5f * (random() - 1.0);
						pos.z += 0.5f * (random() - 1.0);
					}
					else if (side == 2)
					{
						pos.y += 0.5f;
						pos.x += 0.5f * (random() - 1.0);
						pos.z += 0.5f * (random() - 1.0);
					}
					else if (side == 3)
					{
						pos.y -= 0.5f;
						pos.x += 0.5f * (random() - 1.0);
						pos.z += 0.5f * (random() - 1.0);
					}
					else if (side == 4)
					{
						pos.z += 0.5f;
						pos.y += 0.5f * (random() - 1.0);
						pos.x += 0.5f * (random() - 1.0);
					}
					else
					{
						pos.z -= 0.5f;
						pos.y += 0.5f * (random() - 1.0);
						pos.x += 0.5f * (random() - 1.0);
					}
					ParticleType particle = {};
					particle.pos = pos;
					particle.velocity = glm::vec3(0, 0, 0);
					particle.velocityVar = 0.5f * glm::vec3(0.1, 0.1, 0.1);
					particle.beginColor = glm::vec4(0.7, 0.7, 0.7, 1);
					particle.endColor = glm::vec4(0.2, 0.2, 0.2, 0);
					particle.colorVar = glm::vec4(0.05, 0.05, 0.05, 0);
					particle.acceleration = glm::vec3(0, -0.1, 0);
					particle.beginSize = 0.02f;
					particle.endSize = 0.005f;
					particle.sizeVar = 0.03f;
					particle.lifetime = 0.8f;

					addParticle(particle);
				}
			}
			else
				setVoxel(ovoxel.x, ovoxel.y, ovoxel.z, fill, 0);
			return;
		}
		ovoxel = voxel;
		if (tMax.x < tMax.y && tMax.x < tMax.z) {
			voxel.x += step.x;
			tMax.x += tDelta.x;
		}
		else if (tMax.y < tMax.z) {
			voxel.y += step.y;
			tMax.y += tDelta.y;
		}
		else {
			voxel.z += step.z;
			tMax.z += tDelta.z;
		}
	}
}


void Earth::addParticle(ParticleType particle)
{
	particles.createParticle(particle);
}

void Earth::genParticle(ParticleGenerator& particle, float dt)
{
	particles.genParticle(particle,dt);
}

void Earth::getCollisions(CollisionData& data)
{
	for (PhysicObject& c : physicObjects)
	{
		for (PhysicObject& o : physicObjects)
		{
			if (&c == &o)
			{
				continue;
			}
			boxAndBoxCollision(c, o, data);
		}
		Box myBox = { 1.0f,1.0f,1.0f,glm::vec3(0,0,0),glm::mat3(1,0,0,0,1,0,0,0,1) };
		Box boxC = Box(c);
		float minx = c.pos.x;
		float miny = c.pos.y;
		float minz = c.pos.z;
		float maxx = c.pos.x;
		float maxy = c.pos.y;
		float maxz = c.pos.z;
		for (int i = 0; i < 8; i++)
		{
			glm::vec3 currentCoord = returnPoint(boxC, i);
			minx = glm::min(minx, currentCoord.x);
			miny = glm::min(miny, currentCoord.y);
			minz = glm::min(minz, currentCoord.z);
			maxx = glm::max(maxx, currentCoord.x);
			maxy = glm::max(maxy, currentCoord.y);
			maxz = glm::max(maxz, currentCoord.z);

		}
		for (int x = int(minx); x <= int(maxx + 0.5f); x++)
			for (int y = int(miny); y <= int(maxy + 0.5f); y++)
				for (int z = int(minz); z <= int(maxz + 0.5f); z++)
					if (getVoxel(x, y, z) != 0)
					{
						myBox.pos = glm::vec3(x + 0.5, y + 0.5, z + 0.5);
						boxAndObjectCollision(myBox,boxC,data);
						//edgeAndEdgeCollision(myBox, boxC, data);
					}
		/*glm::vec3 deltaVelWorld = glm::inverse(c.rotationMat) * (data.contacts[0].contactPoint - c.pos) % data.contacts[0].contactNormal;
		deltaVelWorld = c.inverseInertiaTensor*deltaVelWorld;
		deltaVelWorld = glm::cross(deltaVelWorld, glm::inverse(c.rotationMat) * (data.contacts[0].contactPoint - c.pos));
		float deltaVelocity = glm::dot(deltaVelWorld, data.contacts[0].contactNormal);
		deltaVelocity += c.inverseMass;*/
		if(data.contacts.size()>0)
			for (const Contact& contact : data.contacts) {
				glm::vec3 r = contact.contactPoint - c.pos;
				c.pos += contact.contactNormal * contact.depth;
				float relVel = glm::dot(c.velocity, contact.contactNormal);
				if (relVel < 0.0f) {
					float restitution = 0.5f;
					float impulseMag = -(1.0f + restitution) * relVel;
					impulseMag /= c.inverseMass;
					glm::vec3 impulse = impulseMag * contact.contactNormal;
					glm::vec3 torque = glm::cross(r, impulse);
					c.rotationVelocity += c.inverseInertiaTensor * torque;
					c.velocity += impulse * c.inverseMass;
				}

			}
	}
}

Chunk::Chunk()
{
	blockCount = 0;
	for (int i = 0; i < 32 * 32 * 32; i++)
	{
		voxels[i] = 0;
	}
	float quadVertices[] = {
		 0.00f, 0.00f, 0.00f,
		 1.00f, 0.00f, 0.00f,
		 0.00f, 0.00f, 1.00f,
		 1.00f, 0.00f, 1.00f,
	};
	int faces[] = {
		0,1,2,3,4,5
	};
	for (int i = 0; i < 6; i++)
	{
		indirectBuffer[i][0] = 4;
		indirectBuffer[i][1] = 0;
		indirectBuffer[i][2] = 0;
		indirectBuffer[i][3] = 0;
	}

	glGenBuffers(1, &dataVBO);
	glBindBuffer(GL_ARRAY_BUFFER, dataVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uint32_t) * instanceData.size(), instanceData.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IBO);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(indirectBuffer), indirectBuffer, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

	glGenBuffers(1, &SSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(faces), faces, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, dataVBO);
	glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(uint32_t), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribDivisor(1, 1);

	changed = false;
}

unsigned short Chunk::getVoxel(int x, int y, int z)
{
	return voxels[x + z * 32 + y * 32 * 32];
}

unsigned short Chunk::getVoxel(int i)
{
	return voxels[i];
}

void Chunk::setVoxel(int x, int y, int z, short type) 
{
	int i = x + z * 32 + y * 32 * 32;
	if (voxels[i] == type)
	{
		return;
	}
	if (voxels[i] == 0 && type != 0)
		blockCount++;
	if (voxels[i] != 0 && type == 0)
		blockCount--;
	voxels[i] = char(type);
	changed = true;
}

void Chunk::setVoxel(int i, short type)
{
	if (voxels[i] == type)
	{
		return;
	}
	if (voxels[i] == 0 && type != 0)
		blockCount++;
	if (voxels[i] != 0 && type == 0)
		blockCount--;
	voxels[i] = char(type);
	changed = true;
}
void Chunk::generateMesh2()
{
	instanceData.clear();
	uint32_t colBinary[32*32*3];
	uint32_t colFaces[32 * 32 * 6];
	for (int i = 0; i < 32 * 32 * 32; i++)
	{
		if (voxels[i] != 0)
		{
			colBinary[i / 32] |= 1l << i % 32;
			//Reference: [Y+Z*32]>>X
			colBinary[i % (1024) + 1024] |= 1l << i / (1024);
			//Reference [X+Z*32]>>Y
			colBinary[i % 32 + i / (1024) * 32 + 2048] |= 1l << i / 32 % 32;
			//Reference [X+Y*32]>>Z
		}
	}
	for (int axis = 0; axis < 3; axis++)
	{
		for (int i = 0; i < 32 * 32; i++)
		{
			colFaces[axis * 2 * 32 * 32 + i] = colBinary[axis * 32 * 32 + i] & ~(colBinary[axis * 32 * 32 + i] >> 1);
			colFaces[(axis * 2 + 1) * 32 * 32 + i] = colBinary[axis * 32 * 32 + i] & ~(colBinary[axis * 32 * 32 + i] << 1);
		}
	}
	for (int face = 0; face < 6; face++)
	{
		std::vector<uint32_t> instanceData;
		for (int type = 0; type < 2; type++)
		{
			unsigned long faces[1024];
			memset(faces, 0, sizeof(faces));
			for (int i = 0; i < 32 * 32 * 32; i++)
			{
				faces[i / 32] |= ((voxels[i] == type ? 1l : 0l) << i % 32); //Initial Step of converting to boolean
			}
			for (int i = 0; i < 1024; i++)
			{
				faces[i] = faces[i] & ((face%2==1)?~(faces[i] >> 1): ~(faces[i] << 1));
			}
			//Then Convert to faces visible
			//Then Convert to mesh
		}
		
		indirectBuffer[face][3] = this->instanceData.size();
		this->instanceData.insert(instanceData.end(), instanceData.begin(), instanceData.end());
		indirectBuffer[face][1] = instanceData.size();
	}
	glBindBuffer(GL_ARRAY_BUFFER, dataVBO);
	glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(uint32_t), instanceData.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IBO);
	glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, sizeof(indirectBuffer), indirectBuffer);

	changed = false;
}
/*void greedyMesh(uint32_t face[32], std::vector<uint32_t>& instanceData, int axis)
{
	for (int row = 0; row < 32; row++)
	{
		int y = 0;
		while (y < 32)
		{
			y += __builtin_ctz(face[row]>>y);
			if (y >= 32)
			{
				continue;
			}
			int height = __builtin_ctz(~(face[row] >> y));
			uint32_t mask = (0xffffffffUL >> (32 - (height + y))) << y;
			int looprow;
			for (looprow = row+1; looprow < 32; looprow++)
			{
				if (!((face[looprow] & mask) == mask))
					break;
				face[looprow] ^= mask;
			}
		}
	}
}*/
void Chunk::generateMesh()
{
	instanceData.clear();
	unsigned long boolX[1024];
	unsigned long boolY[1024];
	unsigned long boolZ[1024];
	unsigned long boolPX[1024];
	unsigned long boolPY[1024];
	unsigned long boolPZ[1024];
	unsigned long boolNX[1024];
	unsigned long boolNY[1024];
	unsigned long boolNZ[1024];
	memset(boolX, 0, sizeof(boolX));
	memset(boolY, 0, sizeof(boolY));
	memset(boolZ, 0, sizeof(boolZ));
	memset(boolPX, 0, sizeof(boolPX));
	memset(boolPY, 0, sizeof(boolPY));
	memset(boolPZ, 0, sizeof(boolPZ));
	memset(boolNX, 0, sizeof(boolNX));
	memset(boolNY, 0, sizeof(boolNY));
	memset(boolNZ, 0, sizeof(boolNZ));
	std::vector<uint32_t> instanceDataPY;
	std::vector<uint32_t> instanceDataNY;
	std::vector<uint32_t> instanceDataPX;
	std::vector<uint32_t> instanceDataNX;
	std::vector<uint32_t> instanceDataPZ;
	std::vector<uint32_t> instanceDataNZ;
	for (int i = 0; i < 32 * 32 * 32; i++)
	{
		boolX[i / 32] |= ((voxels[i] != 0 ? 1l : 0l) << i % 32);
		//Reference: [Y+Z*32]>>X
		boolY[i % (1024)] |= ((voxels[i] != 0 ? 1l : 0l) << i / (1024));
		//Reference [X+Z*32]>>Y
		boolZ[i % 32 + i / (1024) * 32] |= ((voxels[i] != 0 ? 1l : 0l) << i / 32 % 32);
		//Reference [X+Y*32]>>Z
	}
	for (int i = 0; i < 1024; i++)
	{
		boolPX[i] = boolX[i] & ~(boolX[i] >> 1);
		boolPY[i] = boolY[i] & ~(boolY[i] >> 1);
		boolPZ[i] = boolZ[i] & ~(boolZ[i] >> 1);
		boolNX[i] = boolX[i] & ~(boolX[i] << 1);
		boolNY[i] = boolY[i] & ~(boolY[i] << 1);
		boolNZ[i] = boolZ[i] & ~(boolZ[i] << 1);
	}
	for (int i = 0; i < 32 * 32 * 32; i++)
	{
		if ((boolPX[i / 32 % 32 + i / 32 / 32 * 32] >> (i % 32)) % 2 == 1)
		{
			int width = 1;
			int height = 1;
			while ((boolPX[i / 32 % 32 + width + i / 32 / 32 * 32] >> (i % 32)) % 2 == 1 && width + i / 32 % 32 < 32 && getVoxel(i) == getVoxel(i+width*32))
			{
				boolPX[i / 32 % 32 + width + i / 32 / 32 * 32] ^= 1 << i % 32;
				width++;
			}
			while (height + i / 32 / 32 < 32)
			{
				int j;
				for (j = 0; j < width; j++)
				{
					if ((boolPX[i / 32 % 32 + j + (i / 32 / 32 + height) * 32] >> (i % 32)) % 2 != 1 || getVoxel(i) != getVoxel(i + j * 32 + height * 32 * 32))
						break;
				}
				if (j != width)
					break;
				for (int j = 0; j < width; j++)
				{
					boolPX[i / 32 % 32 + j + (i / 32 / 32 + height) * 32] ^= 1 << (i % 32);
				}
				height++;
			}
			addVertex(instanceDataPX, i % 32, i / 32 / 32, i / 32 % 32, getVoxel(i), width-1, height-1);
		}
		if ((boolNX[i / 32 % 32 + i / 32 / 32 * 32] >> (i % 32)) % 2 == 1)
		{
			int width = 1;
			int height = 1;
			while ((boolNX[i / 32 % 32 + width + i / 32 / 32 * 32] >> (i % 32)) % 2 == 1 && width + i / 32 % 32 < 32 && getVoxel(i) == getVoxel(i + width * 32))
			{
				boolNX[i / 32 % 32 + width + i / 32 / 32 * 32] ^= 1 << i % 32;
				width++;
			}
			while (height + i / 32 / 32 < 32)
			{
				int j;
				for (j = 0; j < width; j++)
				{
					if ((boolNX[i / 32 % 32 + j + (i / 32 / 32 + height) * 32] >> (i % 32)) % 2 != 1 || getVoxel(i) != getVoxel(i + j * 32 + height * 32 * 32))
						break;
				}
				if (j != width)
					break;
				for (int j = 0; j < width; j++)
				{
					boolNX[i / 32 % 32 + j + (i / 32 / 32 + height) * 32] ^= 1 << (i % 32);
				}
				height++;
			}
			addVertex(instanceDataNX, i % 32, i / 32 / 32, i / 32 % 32, getVoxel(i), width-1, height-1);
		}
		if ((boolPY[i % 32 + i / 32 % 32 * 32] >> (i / 32 / 32)) % 2 == 1)
		{
			int width = 1;
			int height = 1;
			while ((boolPY[i % 32 + width + i / 32 % 32 * 32] >> (i / 32 / 32)) % 2 == 1 && width + i % 32 < 32 && getVoxel(i) == getVoxel(i + width))
			{
				boolPY[i % 32 + width + i / 32 % 32 * 32] ^= 1 << (i / 32 / 32);
				width++;
			}
			while (height + i / 32 % 32 < 32)
			{
				int j;
				for (j = 0; j < width; j++)
				{
					if ((boolPY[i % 32 + j + (i / 32 % 32 + height) * 32] >> (i / 32 / 32)) % 2 != 1 || getVoxel(i) != getVoxel(i + j + height * 32))
						break;
				}
				if (j != width)
					break;
				for (int j = 0; j < width; j++)
				{
					boolPY[i % 32 + j + (i / 32 % 32 + height) * 32] ^= 1 << (i / 32 / 32);
				}
				height++;
			}
			addVertex(instanceDataPY, i % 32, i / 32 / 32, i / 32 % 32, getVoxel(i), width-1, height - 1);
		}
		if ((boolNY[i % 32 + i / 32 % 32 * 32] >> (i / 32 / 32)) % 2 == 1)
		{
			int width = 1;
			int height = 1;
			while ((boolNY[i % 32 + width + i / 32 % 32 * 32] >> (i / 32 / 32)) % 2 == 1 && width + i % 32 < 32 && getVoxel(i) == getVoxel(i + width))
			{
				boolNY[i % 32 + width + i / 32 % 32 * 32] ^= 1 << (i / 32 / 32);
				width++;
			}
			while (height + i / 32 % 32 < 32)
			{
				int j;
				for (j = 0; j < width; j++)
				{
					if ((boolNY[i % 32 + j + (i / 32 % 32 + height) * 32] >> (i / 32 / 32)) % 2 != 1 || getVoxel(i) != getVoxel(i + j + height * 32))
						break;
				}
				if (j != width)
					break;
				for (int j = 0; j < width; j++)
				{
					boolNY[i % 32 + j + (i / 32 % 32 + height) * 32] ^= 1 << (i / 32 / 32);
				}
				height++;
			}
			addVertex(instanceDataNY, i % 32, i / 32 / 32, i / 32 % 32, getVoxel(i), width-1, height-1);
		}
		if ((boolPZ[i % 32 + i / 32 / 32 * 32] >> (i / 32 % 32)) % 2 == 1)
		{
			int width = 1;
			int height = 1;
			while ((boolPZ[i % 32 + width + i / 32 / 32 * 32] >> (i / 32 % 32)) % 2 == 1 && width + i % 32 < 32 && getVoxel(i) == getVoxel(i + width))
			{
				boolPZ[i % 32 + width + i / 32 / 32 * 32] ^= 1 << (i / 32 % 32);
				width++;
			}
			while (height + i / 32 / 32 < 32)
			{
				int j;
				for (j = 0; j < width; j++)
				{
					if ((boolPZ[i % 32 + j + (i / 32 / 32 + height) * 32] >> (i / 32 % 32)) % 2 != 1 || getVoxel(i) != getVoxel(i + j + height * 32 * 32))
						break;
				}
				if (j != width)
					break;
				for (int j = 0; j < width; j++)
				{
					boolPZ[i % 32 + j + (i / 32 / 32 + height) * 32] ^= 1 << (i / 32 % 32);
				}
				height++;
			}
			addVertex(instanceDataPZ, i % 32, i / 32 / 32, i / 32 % 32, getVoxel(i), width-1, height-1);
		}
		if ((boolNZ[i % 32 + i / 32 / 32 * 32] >> (i / 32 % 32)) % 2 == 1)
		{
			int width = 1;
			int height = 1;
			while ((boolNZ[i % 32 + width + i / 32 / 32 * 32] >> (i / 32 % 32)) % 2 == 1 && width + i % 32 < 32 && getVoxel(i) == getVoxel(i + width))
			{
				boolNZ[i % 32 + width + i / 32 / 32 * 32] ^= 1 << (i / 32 % 32);
				width++;
			}
			while (height + i / 32 / 32 < 32)
			{
				int j;
				for (j = 0; j < width; j++)
				{
					if ((boolNZ[i % 32 + j + (i / 32 / 32 + height) * 32] >> (i / 32 % 32)) % 2 != 1 || getVoxel(i) != getVoxel(i + j + height * 32 * 32))
						break;
				}
				if (j != width)
					break;
				for (int j = 0; j < width; j++)
				{
					boolNZ[i % 32 + j + (i / 32 / 32 + height) * 32] ^= 1 << (i / 32 % 32);
				}
				height++;
			}
			addVertex(instanceDataNZ, i % 32, i / 32 / 32, i / 32 % 32, getVoxel(i), width-1, height-1);
		}
	}
	indirectBuffer[0][3] = 0;
	instanceData.insert(instanceData.end(), instanceDataPY.begin(), instanceDataPY.end());
	indirectBuffer[1][3] = instanceData.size();
	instanceData.insert(instanceData.end(), instanceDataNY.begin(), instanceDataNY.end());
	indirectBuffer[2][3] = instanceData.size();
	instanceData.insert(instanceData.end(), instanceDataPX.begin(), instanceDataPX.end());
	indirectBuffer[3][3] = instanceData.size();
	instanceData.insert(instanceData.end(), instanceDataNX.begin(), instanceDataNX.end());
	indirectBuffer[4][3] = instanceData.size();
	instanceData.insert(instanceData.end(), instanceDataPZ.begin(), instanceDataPZ.end());
	indirectBuffer[5][3] = instanceData.size();
	instanceData.insert(instanceData.end(), instanceDataNZ.begin(), instanceDataNZ.end());
	indirectBuffer[0][1] = instanceDataPY.size();
	indirectBuffer[1][1] = instanceDataNY.size();
	indirectBuffer[2][1] = instanceDataPX.size();
	indirectBuffer[3][1] = instanceDataNX.size();
	indirectBuffer[4][1] = instanceDataPZ.size();
	indirectBuffer[5][1] = instanceDataNZ.size();
	glBindBuffer(GL_ARRAY_BUFFER, dataVBO);
	glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(uint32_t), instanceData.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IBO);
	glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, sizeof(indirectBuffer), indirectBuffer);
	changed = false;
}

void Chunk::draw(Shader& shader)
{
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
	glMultiDrawArraysIndirect(GL_TRIANGLE_STRIP, 0, 6, sizeof(indirectBuffer[0]));
	glBindVertexArray(0);
}

void Chunk::addVertex(std::vector<uint32_t>& instanceData, int x, int y, int z, int textureID, int width, int height)
{
	instanceData.push_back(0);
	instanceData.back() |= x & 31; //5 bits
	instanceData.back() |= (y & 31) << 5; //5 bits
	instanceData.back() |= (z & 31) << 10; //5 bits
	instanceData.back() |= (textureID & 127) << 15; //3 bits
	instanceData.back() |= (width & 31) << 22;
	instanceData.back() |= (height & 31) << 27;
	int x1 = instanceData.back() & 31;
	int y1 = (instanceData.back() >> 5) & 31;
	int z1 = (instanceData.back() >> 10) & 31;
	int textureID1 = (instanceData.back() >> 15) & 127;
	int width1 = ((instanceData.back() >> 22) & 31);
	int height1 = ((instanceData.back() >> 27) & 31);
}

void Chunk::reset()
{
	for (int i = 0; i < 32 * 32 * 32; i++)
	{
		setVoxel(i, 0);
	}
}

// Use a fixed-size array instead of vector for the permutation table
// Arrays have better cache performance and no heap allocation overhead
static std::array<int, 512> Permutation;

// Initialize once at startup
static bool permutationInitialized = false;

inline void Shuffle(std::array<int, 256>& arr) {
	// Use std::swap instead of manual temp variable — compiler can optimize better
	// Also fixes a bug: original used (e - 1) instead of e, skipping last swap
	for (int e = 255; e > 0; e--) {
		int index = static_cast<int>(random() * e) % e;
		std::swap(arr[e], arr[index]);
	}
}

void InitPermutation() {
	std::array<int, 256> base;
	// std::iota is faster and cleaner than a push_back loop
	std::iota(base.begin(), base.end(), 0);
	Shuffle(base);

	// Single memcpy instead of two separate loops
	std::copy(base.begin(), base.end(), Permutation.begin());
	std::copy(base.begin(), base.end(), Permutation.begin() + 256);
	permutationInitialized = true;
}

// Inline + lookup table replaces branching — much faster in hot path
// Only 4 possible gradient vectors, precomputed
static constexpr glm::vec2 GRADIENTS[4] = {
	{ 1.0f,  1.0f},
	{-1.0f,  1.0f},
	{-1.0f, -1.0f},
	{ 1.0f, -1.0f}
};

inline glm::vec2 GetConstantVector(int v) {
	return GRADIENTS[v & 3];  // Replaces 4-branch if/else
}

inline float Fade(float t) {
	// Same math, but written for better FMA (fused multiply-add) opportunities
	return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

inline float Lerp(float t, float a1, float a2) {
	return std::fma(t, a2 - a1, a1);  // fma is faster and more precise than a1 + t*(a2-a1)
}

float Noise2D(glm::vec2 pos) {
	// Compute floor once and reuse — avoids calling glm::floor twice on same value
	float fx = glm::floor(pos.x);
	float fy = glm::floor(pos.y);

	int X = static_cast<int>(fx) & 255;
	int Y = static_cast<int>(fy) & 255;

	float xf = pos.x - fx;
	float yf = pos.y - fy;

	// Precompute permutation row lookups to avoid redundant table hits
	int pX = Permutation[X];
	int pX1 = Permutation[X + 1];

	int valueBottomLeft = Permutation[pX + Y];
	int valueBottomRight = Permutation[pX1 + Y];
	int valueTopLeft = Permutation[pX + Y + 1];
	int valueTopRight = Permutation[pX1 + Y + 1];

	// Precompute fade values once
	float u = Fade(xf);
	float v = Fade(yf);

	// Inline dot products with gradient vectors instead of calling glm::dot
	// Avoids vec2 construction and function call overhead in hot path
	const glm::vec2& gBL = GRADIENTS[valueBottomLeft & 3];
	const glm::vec2& gBR = GRADIENTS[valueBottomRight & 3];
	const glm::vec2& gTL = GRADIENTS[valueTopLeft & 3];
	const glm::vec2& gTR = GRADIENTS[valueTopRight & 3];

	float dotBL = gBL.x * xf + gBL.y * yf;
	float dotBR = gBR.x * (xf - 1.0f) + gBR.y * yf;
	float dotTL = gTL.x * xf + gTL.y * (yf - 1.0f);
	float dotTR = gTR.x * (xf - 1.0f) + gTR.y * (yf - 1.0f);

	return Lerp(u,
		Lerp(v, dotBL, dotTL),
		Lerp(v, dotBR, dotTR)
	);
}

float FractalBrownianMotion(float x, float y, int numOctaves) {
	float result = 0.0f;
	float amplitude = 80.0f;
	float frequency = 0.005f;

	for (int octave = 0; octave < numOctaves; octave++) {
		// Multiply into Noise2D args directly — avoids extra vec2 arithmetic
		result += amplitude * Noise2D({ x * frequency, y * frequency });

		// Bit shift is equivalent to *0.5 and *2.0 but avoids float multiply
		// (compiler likely does this anyway, but being explicit doesn't hurt)
		amplitude *= 0.5f;
		frequency *= 2.0f;
	}

	return result;
}

#define TAU 6.28318530717959f

glm::vec2 hash(glm::vec2 x) {
	glm::vec2 k = glm::vec2(0.3183099, 0.3678794);
	x = x * k + glm::vec2(k.y,k.x);
	return -1.0f + 2.0f * glm::fract(16.0f * k * (x.x * x.y * glm::fract(x.x + x.y)));
}

glm::vec4 PhacelleNoise(glm::vec2 p, glm::vec2 normDir, float freq, float offset, float normalization) {
	glm::vec2 sideDir = glm::vec2(normDir.y,normDir.x) * glm::vec2(-1.0, 1.0) * freq * TAU;
	offset *= TAU;
	glm::vec2 pInt = floor(p);
	glm::vec2 pFrac = fract(p);
	glm::vec2 phaseDir = glm::vec2(0.0);
	float weightSum = 0.0;
	for (int i = -1; i <= 2; i++) {
		for (int j = -1; j <= 2; j++) {
			glm::vec2 gridOffset = glm::vec2(i, j);
			glm::vec2 gridPoint = pInt + gridOffset;
			glm::vec2 randomOffset = hash(gridPoint) * 0.5f;
			glm::vec2 vectorFromCellPoint = pFrac - gridOffset - randomOffset;
			float sqrDist = dot(vectorFromCellPoint, vectorFromCellPoint);
			float weight = exp(-sqrDist * 2.0);
			weight = glm::max(0.0, weight - 0.01111);
			weightSum += weight;
			float waveInput = dot(vectorFromCellPoint, sideDir) + offset;
			phaseDir += glm::vec2(cos(waveInput), sin(waveInput)) * weight;
		}
	}

	glm::vec2 interpolated = phaseDir / weightSum;
	float magnitude = glm::sqrt(glm::dot(interpolated, interpolated));
	magnitude = glm::max(1.0f - normalization, magnitude);
	return glm::vec4(interpolated / magnitude, sideDir);
}

float clamp01(float t) {
	return glm::clamp(t, 0.0f, 1.0f);
}

float pow_inv(float t, float power) {
	return 1.0 - pow(1.0 - clamp01(t), power);
}

float ease_out(float t) {
	float v = 1.0 - clamp01(t);
	return 1.0 - v * v;
}

float smooth_start(float t, float smoothing) {
	if (t >= smoothing)
		return t - 0.5 * smoothing;
	return 0.5 * t * t / smoothing;
}

glm::vec2 safe_normalize(glm::vec2 n) {
	// A div-by-zero-safe replacement for normalize.
	float l = glm::length(n);
	return (abs(l) > 1e-10) ? (n / l) : n;
}

// Advanced Terrain Erosion Filter copyright (c) 2025 Rune Skovbo Johansen
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
glm::vec4 ErosionFilter(
	// Input parameters that vary per pixel.
	glm::vec2 p, glm::vec3 heightAndSlope, float fadeTarget,
	// Stylistic parameters that may vary per pixel.
	float strength, float gullyWeight, float detail, glm::vec4 rounding, glm::vec4 onset, glm::vec2 assumedSlope,
	// Scale related parameters that do not support variation per pixel.
	float scale, int octaves, float lacunarity,
	// Other parameters.
	float gain, float cellScale, float normalization,
	// Output parameters.
	float& ridgeMap, float& debug
) {
	strength *= scale;
	fadeTarget = glm::clamp(fadeTarget, -1.0f, 1.0f);

	glm::vec3 inputHeightAndSlope = heightAndSlope;
	float freq = 1.0 / (scale * cellScale);
	float slopeLength = glm::max(glm::length(glm::vec2(heightAndSlope.y,heightAndSlope.z)), 1e-10f);
	float magnitude = 0.0;
	float roundingMult = 1.0;

	float roundingForInput = glm::mix(rounding.y, rounding.x, clamp01(fadeTarget + 0.5)) * rounding.z;
	// The combined accumulating mask, based first on initial slope, and later on slope of each octave too.
	float combiMask = ease_out(smooth_start(slopeLength * onset.x, roundingForInput * onset.x));

	// Initialize the ridgeMap fadeTarget and mask.
	float ridgeMapCombiMask = ease_out(slopeLength * onset.z);
	float ridgeMapFadeTarget = fadeTarget;

	// Deteriming the strength of the initial slope used for gully directions
	// based on the specified mix of the actual slope and an assumed slope.
	glm::vec2 gullySlope = glm::mix(glm::vec2(heightAndSlope.y, heightAndSlope.z), glm::vec2(heightAndSlope.y, heightAndSlope.z) / slopeLength * assumedSlope.x, assumedSlope.y);

	for (int i = 0; i < octaves; i++) {
		// Calculate and add gullies to the height and slope.
		glm::vec4 phacelle = PhacelleNoise(p * freq, safe_normalize(gullySlope), cellScale, 0.25, normalization);
		// Multiply with freq since p was multiplied with freq.
		// Negate since we use slope directions that point down.
		phacelle.z *= -freq;
		phacelle.w *= -freq;
		// Amount of slope as value from 0 to 1.
		float sloping = abs(phacelle.y);
		// Add non-masked, normalized slope to gullySlope, for use by subsequent octaves.
		// It's normalized to use the steepest part of the sine wave everywhere.
		gullySlope += glm::sign(phacelle.y) * glm::vec2(phacelle.z, phacelle.w) * strength * gullyWeight;

		// Handle height offset and approximate output slope.

		// Gullies has height offset (from -1 to 1) in x and derivative in yz.
		glm::vec3 gullies = glm::vec3(phacelle.x, phacelle.y * glm::vec2(phacelle.z, phacelle.w));
		// Fade gullies towards fadeTarget based on combiMask.
		glm::vec3 fadedGullies = glm::mix(glm::vec3(fadeTarget, 0.0, 0.0), gullies * gullyWeight, combiMask);
		// Apply height offset and derivative (slope) according to strength of current octave.
		heightAndSlope += fadedGullies * strength;
		magnitude += strength;

		// Update fadeTarget to include the new octave.
		fadeTarget = fadedGullies.x;

		// Update the mask to include the new octave.
		float roundingForOctave = glm::mix(rounding.y, rounding.x, clamp01(phacelle.x + 0.5)) * roundingMult;
		float newMask = ease_out(smooth_start(sloping * onset.y, roundingForOctave * onset.y));
		combiMask = pow_inv(combiMask, detail) * newMask;

		// Update the ridgeMap fadeTarget and mask.
		ridgeMapFadeTarget = glm::mix(ridgeMapFadeTarget, gullies.x, ridgeMapCombiMask);
		float newRidgeMapMask = ease_out(sloping * onset.w);
		ridgeMapCombiMask = ridgeMapCombiMask * newRidgeMapMask;

		// Prepare the next octave.
		strength *= gain;
		freq *= lacunarity;
		roundingMult *= rounding.w;
	}

	ridgeMap = ridgeMapFadeTarget * (1.0 - ridgeMapCombiMask);
	debug = fadeTarget;

	glm::vec3 heightAndSlopeDelta = heightAndSlope - inputHeightAndSlope;
	return glm::vec4(heightAndSlopeDelta, magnitude);
}


glm::vec4 Heightmap(glm::vec2 p, glm::vec4 heightAndSlope) {

	// ------------------------------------------------------------------------
	// Erosion parameters.
	// ------------------------------------------------------------------------

	// The scale of the erosion effect, affecting it both horizontally and vertically.
	float EROSION_SCALE = 0.15;

	// The strength of the erosion effect, affecting the magnitude of all octaves,
	// and indirectly affecting the directions of the gullies as a result.
	float EROSION_STRENGTH = 0.22;

	// The magnitude of the gullies as a weight value from 0 to 1.
	// A value of 0 can sharpen peaks and valleys but feature virtually no gullies.
	// A value of 1 produces full gullies but may leave peaks and valleys rounded.
	// Adjusting erosion gully weight while inversely adjusting erosion scale can be
	// used to control the sharpness of peaks and valleys while leaving gully
	// magnitudes largely untocuhed.
	float EROSION_GULLY_WEIGHT = 0.5;

	// The overall detail of the erosion. Lower values restrict the effect of higher
	// frequency gullies to steeper slopes.
	float EROSION_DETAIL = 1.5;

	float ridgeRounding = 0.1;
	float creaseRounding = 0.0;
	// Separate rounding control of ridges and creases.
	//  x: Rounding of ridges.
	//  y: Rounding of creases.
	//  z: Multiplier applied to the initial height function.
	//     E.g. if the height function has noise of 5 times lower frequency
	//     than the largest gullies, a value of 0.2 can compensate for that.
	//  w: Multiplier applied to each subsequent gully octave after the first.
	//     Setting it to the same value as the erosion lacunarity will produce
	//     consistent rounding of all octaves.
	glm::vec4 EROSION_ROUNDING = glm::vec4(ridgeRounding, creaseRounding, 0.1, 2.0);

	// Control over how far away from ridges/creases the erosion takes effect.
	//  x: Onset used on the initial height function.
	//  y: Onset used on each gully octave.
	//  z: RidgeMap-specific onset used on the initial height function.
	//  w: RidgeMap-specific onset used on each gully octave.
	glm::vec4 EROSION_ONSET = glm::vec4(0.7, 1.25, 2.8, 1.5);

	// Control over the assumed slope of the initial height function.
	// In practise, assuming a slope can work better than using the input slope,
	// since the final terrain can be shaped quite differently than the input.
	//  x: An assumed slope value to override the actual slope.
	//  y: The amount (from 0 to 1) to override the actual slope.
	glm::vec2 EROSION_ASSUMED_SLOPE = glm::vec2(0.7, 1.0);

	// Gullies are based on stripes within Voronoi-like cells in the Phacelle noise
	// function. The cell scale parameter controls the sizes of the cells relative
	// to the overall erosion scale, while keeping the stripe widths unaffected.
	// Values close to 1 usually produce good results. Smaller values produce more
	// grainy gullies while larger values produce longer unbroken gullies, but too
	// large values produce chaotic curved gullies that are not aligned with the
	// slopes. Value changes can cause abrupt changes in output, especially far away
	// from the origin, so this parameter is not well suited for animation or for
	// modulation by other functions.
	float EROSION_CELL_SCALE = 0.7;
	// The degree of normalization applied in the Phacelle noise, between 0 and 1.
	// The erosion filter depends on a certain consistency in magnitude of the
	// Phacelle output. However, high values can create loopy results where ridges
	// and creases meet up at a point, which produces unnatural looking results.
	float EROSION_NORMALIZATION = 0.5;

	// Control over the erosion octaves, with each successive octave layering
	// smaller gullies onto the terrain.
	int EROSION_OCTAVES = 5;
	// The lacunarity controls the frequency (the inverse
	// horizontal scale) of each octave relative to the last.
	float EROSION_LACUNARITY = 2.0;
	// The gain controls the magnitude (the vertical
	// scale) of each octave relative to the last.
	float EROSION_GAIN = 0.5;


	// ------------------------------------------------------------------------
	// Terrain parameters not used in the erosion function itself.
	// ------------------------------------------------------------------------

	// Control over whether the erosion effect raises or lowers the terrain.
	//  x: An offset value between -1 and 1, where a value of -1 only lowers, while
	//     1 only raises. The offset is proportional to the erosion strength
	//     parameter, so if that parameter is the same for the entire terrain, the
	//     effect of the height offset will move the entire terrain surface up or
	//     down by the same emount.
	//  y: A value between 0 and 1 which is the degree to which the offset value is
	//     replaced by the negated erosion fade target value. This has the effect
	//     of only raising at valleys and only lowering at peaks, which, due to how
	//     the erosion filter works, has the effect of largely preserving the minima
	//     and maxima of the terrain.
	glm::vec2 TERRAIN_HEIGHT_OFFSET = glm::vec2(0.0, 0.0);


	// ------------------------------------------------------------------------
	// Logic for whether erosion is enabled or not.
	// ------------------------------------------------------------------------

	bool erosion = true;

#ifdef COMPARISON_SLIDER
	// Animated slider that displays terrain with/without erosion.
	if (1.0 - p.y > 0.5 - cos(iTime))
		erosion = false;
#endif


	// ------------------------------------------------------------------------
	// Heightmap implementation.
	// ------------------------------------------------------------------------

	// Get height and slope from painted values in Buffer A.
	//glm::vec3 n = texture(iChannel0, p).xyz;
	glm::vec3 n = glm::vec3(0);


	// Define the erosion fade target based on the altitude of the pre-eroded terrain.
	// The fade target should strive to be -1 at valleys and 1 at peaks, but overshooting is ok.
	float fadeTarget = glm::clamp((n.x - 0.0) / 0.15, -1.0, 1.0);

	// Store erosion in h (x : height delta, yz : slope delta, w : magnitude).
	// The output ridge map is -1 on creases and 1 on ridges.
	// The output debug value can be set to various values inside the erosion function.
	float ridgeMap, debug;
	glm::vec4 h = ErosionFilter(
		p, n, fadeTarget,
		EROSION_STRENGTH, EROSION_GULLY_WEIGHT, EROSION_DETAIL,
		EROSION_ROUNDING, EROSION_ONSET, EROSION_ASSUMED_SLOPE,
		EROSION_SCALE, EROSION_OCTAVES, EROSION_LACUNARITY,
		EROSION_GAIN, EROSION_CELL_SCALE, EROSION_NORMALIZATION,
		ridgeMap, debug);

	if (!erosion) {
		h = glm::vec4(0.0);
		ridgeMap = 1.0;
	}

	// Offset according to the height offset parameter by multiplying it with the magnitude.
	float offset = glm::mix(TERRAIN_HEIGHT_OFFSET.x, -fadeTarget, TERRAIN_HEIGHT_OFFSET.y) * h.w;
	float eroded = n.x + h.x + offset;

	// Add trees to terrain.
	float trees = -1.0;
#if defined(TREES)
	vec2 deriv = n.yz + h.yz;
	float normalY = 1.0 / sqrt(1.0 + dot(deriv, deriv));
	float treesAmount = GetTreesAmount(eroded, normalY, h.x / h.w + 0.5, ridgeMap);
	trees = (1.0 - pow(noised((p + 0.5) * 200.0).x * 0.5 + 0.5, 2.0) - 1.0 + 1.0 * treesAmount) * 1.5;
	if (trees > 0.0) {
		eroded += trees / 300.0;
	}
#endif

	return glm::vec4(eroded, 0.0, 0.0, 0.0);

}

void Chunk::generate() {
	for (int x = 0; x < 32; x++)
	{
		for (int z = 0; z < 32; z++)
		{
			float height = 64 + FractalBrownianMotion(worldPos.x + x, worldPos.z + z, 5);
			for (int y = 0; y < 32; y++)
			{
				if (y + worldPos.y < height - 1)
					setVoxel(x, y, z, 1);
				else {
					if (y + worldPos.y < height)
						setVoxel(x, y, z, 2);
					else
						break;

				}
			}
		}
	}
}