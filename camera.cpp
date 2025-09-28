#include "camera.h"
Camera* Camera::mainCamera = nullptr;
glm::mat4 Camera::returnMatrixTransform()
{
	return projection * returnViewTransform();
}
glm::mat4 Camera::returnViewTransform()
{
	return glm::lookAt(pos, pos + direction, glm::vec3(0, 1, 0));
}
Camera::Camera()
{
	projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f);
	pos = glm::vec3(0, 0, 0);
	direction = glm::vec3(0, 0, 1);
	velocity = glm::vec3(0, 0, 1);
	velocityDamping = 0.045;
	maxSpeed = 0.05;
	mainCamera = this;
}
Camera::Camera(glm::vec3 pos, glm::vec3 cameraDirection, float fov, float near, float far)
{
	projection = glm::perspective(glm::radians(fov), 16.0f/9.0f, near, far);
	this->pos = pos;
	direction = cameraDirection;
	pitch = 0.0f;
	yaw = 90.0f;
	velocity = glm::vec3(0, 0, 0);
	velocityDamping = 0.045;
	maxSpeed = 0.2;
	mainCamera = this;
}
void Camera::step(float deltaTime)
{
	pos += velocity * deltaTime * 100.0f;
	float speed = glm::length(velocity);
	if (speed > 0.0f) {
		float newSpeed = glm::max(0.0f, glm::min(speed, maxSpeed) - velocityDamping * deltaTime);
		velocity *= (newSpeed / speed);
	}
}
Camera* Camera::getCamera()
{
	return mainCamera;
}

