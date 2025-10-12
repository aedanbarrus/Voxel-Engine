#pragma once
#ifndef camera
#define camera
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>
class Camera
{
private:
	static Camera* mainCamera;
public:
	glm::vec3 pos;
	glm::vec3 velocity;
	glm::vec3 direction;
	glm::mat4 projection;
	glm::vec3 sunDirection;
	float pitch;
	float yaw;
	float velocityDamping;
	float maxSpeed;
	static Camera* getCamera();
	Camera();
	Camera(glm::vec3 pos, glm::vec3 cameraDirection, float fov, float near, float far);
	glm::mat4 returnMatrixTransform();
	glm::mat4 returnViewTransform();
	void step(float deltaTime);
};
#endif