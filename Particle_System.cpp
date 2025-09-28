#include "Particle_System.h"
ParticleSystem::ParticleSystem() {
	myShader = Shader("pshader.vert", "pshader.frag");
	myParticles.resize(10000);
    float quadVertices[] = {
    -0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    -0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, 0.5f, 0.5f, 
    0.5f, 0.5f, -0.5f,
    -0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, -0.5f,
    -0.5f, -0.5f, 0.5f,
    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    -0.5f, 0.5f, -0.5f,
    0.5f, 0.5f, -0.5f
    };
	GLuint quadVBO;
	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, 10000 * sizeof(ParticleInstanceData), instanceData.data(), GL_DYNAMIC_DRAW);
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

	// Attribute 1: vec3 pos
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)offsetof(ParticleInstanceData, pos));
	glVertexAttribDivisor(1, 1);

	// Attribute 2: vec4 color
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)offsetof(ParticleInstanceData, color));
	glVertexAttribDivisor(2, 1);

	// Attribute 3: float size
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)offsetof(ParticleInstanceData, size));
	glVertexAttribDivisor(3, 1);

	// Attribute 4: mat4 rotation
	for (int i = 0; i < 4; ++i) {
		glEnableVertexAttribArray(4 + i);
		glVertexAttribPointer(4 + i, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData),
			(void*)(offsetof(ParticleInstanceData, rotation) + sizeof(glm::vec4) * i));
		glVertexAttribDivisor(4 + i, 1);
	}

}
void ParticleSystem::step(float deltaTime) {
	instanceData.clear();
	for (Particle& c : myParticles)
	{
		if (!c.active)
			continue;
		c.timeLeft -= deltaTime;
		if (c.timeLeft < 0.0f)
		{
			c.active = false; 
			continue;
		}
		c.pos += c.velocity * deltaTime;
		c.velocity *= glm::pow(c.drag, deltaTime);
		c.velocity += c.acceleration * deltaTime;
		c.rotation = glm::mat3(glm::rotate(glm::mat4(c.rotation), 3.0f*deltaTime, glm::vec3(c.rotation*glm::vec4(1, 0, 0,0))));
		float time = c.timeLeft / c.lifetime;
		instanceData.push_back({c.pos,glm::sqrt(c.endColor * c.endColor * (1 - time) + c.beginColor * c.beginColor * time),  c.endSize * (1 - time) + c.beginSize * time, c.rotation});
	}
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, instanceData.size() * sizeof(ParticleInstanceData), instanceData.data());
}

void ParticleSystem::collide(PhysicObject& Object) {
	for (Particle& c : myParticles)
	{
		if (!c.active)
			continue;
		CollisionData myData = { {} };
		boxAndPointCollision(Object, c.pos, myData);
		if (myData.contacts.size()>0)
			c.active = false;
	}
}

void ParticleSystem::draw()
{
	myShader.use();
	myShader.setMat4("rotMat", Camera::getCamera()->returnMatrixTransform());
	glBindVertexArray(quadVAO);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 14, instanceData.size());
	/*for (Particle& myPart : myParticles)
	{
		if (!myPart.active)
			continue;
		float time = myPart.timeLeft / myPart.lifetime;
		glm::vec4 color = glm::sqrt(myPart.endColor * myPart.endColor * (1 - time) + myPart.beginColor * myPart.beginColor  * time);
		float size = myPart.endSize * (1 - time) + myPart.beginSize * time;
		color.a *= time;
		myShader.setVec3("pos", myPart.pos);
		myShader.setMat4("transform", myPart.rotationWithPos);
		myShader.setFloat("size", size);
		myShader.setVec4("color", color);
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}*/
}

void ParticleSystem::createParticle(ParticleType& newParticle)
{
	Particle* thisPart = &myParticles[currentParticle];
	thisPart->active = true;
	thisPart->lifetime = newParticle.lifetime;
	thisPart->rotation = glm::mat4_cast(glm::quat(glm::normalize(randomVec(glm::vec4(1,1,1,1)))));
	srand(time(0));
	thisPart->beginColor = newParticle.beginColor + randomVec(newParticle.colorVar);
	thisPart->endColor = newParticle.endColor;
	srand(time(0));
	thisPart->beginSize = newParticle.beginSize + (2 * random() - 1) * newParticle.sizeVar;
	thisPart->endSize = newParticle.endSize;
	thisPart->timeLeft = thisPart->lifetime;
	srand(time(0));
	thisPart->velocity = newParticle.velocity + randomVec(newParticle.velocityVar);
	thisPart->acceleration = newParticle.acceleration;
	thisPart->pos = newParticle.pos;
	thisPart->drag = newParticle.drag;
	currentParticle--;
	if (currentParticle == -1)
		currentParticle = 9999;
}

void ParticleSystem::genParticle(ParticleGenerator& newParticle, float dt)
{
	if (newParticle.timeBetweenParticles == 0) return;

	newParticle.timePassed += dt;

	while (newParticle.timePassed >= newParticle.timeBetweenParticles)
	{
		for (int i = 0; i < newParticle.numberPerCycle; ++i)
			createParticle(newParticle.type);
		newParticle.timePassed -= newParticle.timeBetweenParticles;
	}
}

float random()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(0.0, 1.0);
	return dis(gen);
}

glm::vec3 randomVec(glm::vec3 myVec)
{
	glm::vec3 theVec = myVec;
	theVec.x *= (2 * random() - 1);
	theVec.y *= (2 * random() - 1);
	theVec.z *= (2 * random() - 1);
	return theVec;
}

glm::vec4 randomVec(glm::vec4 myVec)
{
	glm::vec4 theVec = myVec;
	theVec.x *= (2 * random() - 1);
	theVec.y *= (2 * random() - 1);
	theVec.z *= (2 * random() - 1);
	theVec.w *= (2 * random() - 1);
	return theVec;
}