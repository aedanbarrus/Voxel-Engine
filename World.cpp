#include "World.h"
Earth::Earth(){
	size_t chunkCount = 16 * 16 * 16;
	quadrantI.resize(chunkCount);
	quadrantII.resize(chunkCount);
	quadrantIII.resize(chunkCount);
	quadrantIV.resize(chunkCount);
	for (int i = 0; i< quadrantI.size(); i++)
	{
		quadrantI[i].worldPos = glm::vec3(i % 16 * 32, i / 16 % 16 * 32, i / 16 / 16 * 32);
	}
	for (int i = 0; i < quadrantII.size(); i++)
	{
		quadrantII[i].worldPos = glm::vec3(i % 16 * -32 -32, i / 16 % 16 * 32, i / 16 / 16 * 32);
	}
	for (int i = 0; i < quadrantIII.size(); i++)
	{
		quadrantIII[i].worldPos = glm::vec3(i % 16 * -32 - 32, i / 16 % 16 * 32, i / 16 / 16 * -32 -32);
	}
	for (int i = 0; i < quadrantIV.size(); i++)
	{
		quadrantIV[i].worldPos = glm::vec3(i % 16 * 32, i / 16 % 16 * 32, i / 16 / 16 * -32 -32);
	}
	changed = false;
	particles = ParticleSystem();
}

void Earth::draw(Shader& shader, glm::vec3 camPos)
{
	shader.use();
	shader.setVec3("cameraPos", Camera::getCamera()->pos);
	shader.setVec3("sunDirection", Camera::getCamera()->sunDirrection);
	shader.setVec3("sunColor", glm::vec3(1,1,1));

	for (Chunk& c : quadrantI)
	{
		if (c.blockCount > 0 && glm::distance(camPos,c.worldPos+glm::vec3(16))<256)
		{
			shader.setVec3("chunkOffset", c.worldPos);
			c.draw(shader);
		}
	}
	for (Chunk& c : quadrantII)
	{
		if (c.blockCount > 0 && glm::distance(camPos, c.worldPos + glm::vec3(16)) < 256)
		{
			shader.setVec3("chunkOffset", c.worldPos);
			c.draw(shader);
		}
	}
	for (Chunk& c : quadrantIII)
	{
		if (c.blockCount > 0 && glm::distance(camPos, c.worldPos + glm::vec3(16)) < 256)
		{
			shader.setVec3("chunkOffset", c.worldPos);
			c.draw(shader);
		}
	}
	for (Chunk& c : quadrantIV)
	{
		if (c.blockCount > 0 && glm::distance(camPos, c.worldPos + glm::vec3(16)) < 256)
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
	for (PhysicObject& c : physicObjects)
		c.step(deltaTime);
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
			if (fill == 0)
				setVoxel(voxel.x, voxel.y, voxel.z, 0, 0);
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