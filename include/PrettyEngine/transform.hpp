#pragma once

#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

class Transform {
public:
	Transform() {
		this->UpdateHalfScale();
	}

	glm::mat4 GetTransformMatrix() {
		auto result = glm::identity<glm::mat4>();

		result = glm::translate(result, this->position);
		result *= glm::mat4_cast(this->rotation);
		result = glm::scale(result, this->scale);

		return result;
	}

	void SetRotationUsingEuler(glm::vec3 euler) {
		this->rotation = glm::quat_cast(glm::mat3(glm::eulerAngleYXZ(euler.y, euler.x, euler.z)));	
	}

	void SetRotationUsingEuler(glm::vec3* euler) {
		this->rotation = glm::quat_cast(glm::mat3(glm::eulerAngleYXZ(euler->y, euler->x, euler->z)));	
	}

	void RotateUsingEuler(glm::vec3 euler) {
		this->rotation += glm::quat_cast(glm::mat3(glm::eulerAngleYXZ(euler.y, euler.x, euler.z)));	
	}

	void RotateUsingEuler(glm::vec3* euler) {
		this->rotation += glm::quat_cast(glm::mat3(glm::eulerAngleYXZ(euler->y, euler->x, euler->z)));	
	}

	void Rotate(float rotationInDegrees, glm::vec3 axis = glm::vec3(0.0f, 0.0f, 1.0f)) {
		glm::quat newRotation = glm::angleAxis(glm::radians(rotationInDegrees), axis);
		this->rotation = glm::normalize(this->rotation * newRotation);
	}

	void Rotate(float* rotationInDegrees, glm::vec3 axis = glm::vec3(0.0f, 0.0f, 1.0f)) {
		this->rotation += glm::angleAxis(glm::radians(*rotationInDegrees), axis);
	}

	void Translate(glm::vec3 translation) {
		this->position += translation;
	}

	void SetScale(glm::vec3 newScale) {
		this->scale = newScale;
		this->UpdateHalfScale();
	}

	void UpdateHalfScale() {
		this->halfScale = this->scale / 2.0f;
	}

	glm::vec3 GetEulerRotation() {
		return glm::eulerAngles(this->rotation);
	}

public:
	glm::vec3 position = glm::zero<glm::vec3>();
	glm::quat rotation = glm::identity<glm::quat>();
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 halfScale = glm::zero<glm::vec3>();
};
