#pragma once
#ifndef Physics
#define Physics
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
#include "camera.h"
class PhysicObject;
class ForceGeneator;

struct Box
{
	float width;
	float height;
	float length;
	glm::vec3 pos;
	glm::mat3 rotation;
};

class ForceGenerator
{
public:
	virtual void updateForce(PhysicObject* myObject, float time);
};

class PhysicObject
{
public:
	glm::vec3 pos;
	glm::vec3 vecocity;
	glm::mat3 rotationMat;
	glm::quat rotationQuaternion;
	glm::mat3 inverseInertiaTensor;
	glm::mat3 inverseInertiaTensorWorld;
	glm::vec3 corners[8];
	float inverseMass = 1;
	glm::vec3 velocity;
	glm::vec3 lastVelocity;
	glm::vec3 rotationVelocity;
	glm::vec3 forceAccum;
	glm::vec3 torqueAccum;
	float width;
	float height;
	float length;
	float angularDamping = 0.4;
	float linearDamping = 0.8;
	unsigned int quadVAO, quadVBO;
	Shader shader;
	std::vector<ForceGenerator*> myForces;
	PhysicObject();
	void draw();
	void addForces(float time);
	void addForce(glm::vec3 force);
	void resetAccumulators();
	void integrate(float deltaTime);
	void step(float deltaTime);
	void addForceAtPoint(const glm::vec3& force, const glm::vec3& point);
	void addForceAtBodyPoint(const glm::vec3& force, const glm::vec3& point);
	glm::vec3 transformVector(glm::vec3 vector);
	void setInertiaTensor(glm::mat3 tensor);
	void calculateDerivedData();
	void transformInertiaTensor();
	glm::vec3 calculateCameraCollision(glm::vec3 cameraPos, glm::vec3 cameraDirection);
	Box toBox();
	operator Box() const;

};

class Gravity : public ForceGenerator
{
public:
	glm::vec3 gravity;
	void updateForce(PhysicObject* myObject, float time) override;
};

class Spring : public ForceGenerator
{
public:
	glm::vec3 springPos;
	glm::vec3 connectionPoint;
	float springDis;
	float springStrength;
	void updateForce(PhysicObject* myObject, float time) override;
};

class Contact
{
public:
	glm::vec3 contactPoint;
	glm::vec3 contactNormal;
	float depth;
};

struct CollisionData
{
	std::vector<Contact> contacts;
};
void rotateQuaternionByVector(glm::quat& quaternion, const glm::vec3& vector);
void addScaledVector(glm::quat& quat1, const glm::vec3& vector, float scale);
glm::vec3 rayCollision(glm::vec3 rayDirection, glm::vec3 rayOrigin, glm::vec3 planeNormal, glm::vec3 pointOnPlane);
float rayCollisionDist(glm::vec3 rayDirection, glm::vec3 rayOrigin, glm::vec3 planeNormal, glm::vec3 pointOnPlane);
void boxAndPointCollision(const Box& myObject, const glm::vec3& myPoint, CollisionData& data);
void boxAndBoxCollision(const Box& box1, const Box& box2, CollisionData& data);
bool boxAndPointCollision(const Box& myObject, const glm::vec3& myPoint);
void boxAndObjectCollision(const Box& box1, const Box& object, CollisionData& data);
void edgeAndEdgeCollision(const Box& box1, const Box& object, CollisionData& data);
glm::vec3 returnPoint(const Box& box, const int corner);
struct edge
{
	glm::vec3 point1;
	glm::vec3 point2;
};
edge getEdge(const Box& box, int number);

#endif