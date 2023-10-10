#pragma once

#include <PrettyEngine/serial.hpp>

#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace PrettyEngine {
	class Transform: public SerialObject {
	public:
		Transform() {
			this->UpdateHalfScale();
		}

		Transform* GetTransform() {
			return this;
		}

		void AddToToml(toml::table* table) override {
			auto position = toml::array();
			position.push_back(this->position.x);
			position.push_back(this->position.y);
			position.push_back(this->position.z);

			auto rotation = toml::array();
			rotation.push_back(this->rotation.x);
			rotation.push_back(this->rotation.y);
			rotation.push_back(this->rotation.z);
			rotation.push_back(this->rotation.w);

			auto scale = toml::array();
			scale.push_back(this->scale.x);
			scale.push_back(this->scale.y);
			scale.push_back(this->scale.z);

			table->insert_or_assign("position", position);
			table->insert_or_assign("rotation", rotation);
			table->insert_or_assign("scale", scale);
		}

		void FromToml(toml::table* table) override {
			bool isArray = (*table)["position"].is_array();
			if (isArray) {
				if ((*table)["position"].is_array()) {
					auto position = (*table)["position"].as_array();

					this->position.x = position->get_as<double>(0)->value_or(0.0f);
					this->position.y = position->get_as<double>(1)->value_or(0.0f);
					this->position.z = position->get_as<double>(2)->value_or(0.0f);

					auto rotation = (*table)["rotation"].as_array();

					this->rotation.x = rotation->get_as<double>(0)->value_or(0.0f);
					this->rotation.y = rotation->get_as<double>(1)->value_or(0.0f);
					this->rotation.z = rotation->get_as<double>(2)->value_or(0.0f);
					this->rotation.w = rotation->get_as<double>(3)->value_or(0.0f);

					auto scale = (*table)["scale"].as_array();

					this->scale.x = scale->get_as<double>(0)->value_or(0.0f);
					this->scale.y = scale->get_as<double>(1)->value_or(0.0f);
					this->scale.z = scale->get_as<double>(2)->value_or(0.0f);

					auto arraySize = position->size();
				}
			}
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
}
