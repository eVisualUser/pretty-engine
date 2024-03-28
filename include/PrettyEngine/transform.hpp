#ifndef H_TRANSFORM
#define H_TRANSFORM

#include <PrettyEngine/serial.hpp>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace PrettyEngine {
	/// Represent an object in a space.
	class Transform: public virtual SerialObject {
	public:
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
			if (const auto position = (*table)["position"].as_array()) {
				if (const auto x = position->get_as<double>(0))
					this->position.x = x->value_or(0.0f);
				if (const auto y = position->get_as<double>(0))
					this->position.x = y->value_or(0.0f);
				if (const auto z = position->get_as<double>(0))
					this->position.x = z->value_or(0.0f);
			}

			if (const auto rotation = (*table)["rotation"].as_array()) {
				this->rotation.x = static_cast<float>(rotation->get_as<double>(0)->get());
				this->rotation.y = static_cast<float>(rotation->get_as<double>(1)->get());
				this->rotation.z = static_cast<float>(rotation->get_as<double>(2)->get());
				this->rotation.w = static_cast<float>(rotation->get_as<double>(3)->get());
			}

			if (const auto scale = (*table)["scale"].as_array()) {
				this->scale.x = static_cast<float>(scale->get_as<double>(0)->get());
				this->scale.y = static_cast<float>(scale->get_as<double>(1)->get());
				this->scale.z = static_cast<float>(scale->get_as<double>(2)->get());
			}
		}

		glm::mat4 GetTransformMatrix() {
			auto result = glm::identity<glm::mat4>();

			result = glm::translate(result, this->position);
			result *= glm::mat4_cast(this->rotation);
			result = glm::scale(result, this->scale);

			return result;
		}

		glm::mat4 GetTransformMatrixHalfScale() {
			auto result = glm::identity<glm::mat4>();

			result = glm::translate(result, this->position);
			result *= glm::mat4_cast(this->rotation);
			result = glm::scale(result, this->halfScale);

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

		float GetAverageScale() {
			return (this->scale.x + this->scale.y + this->scale.z) / 3;
		}

		void TransformPosition(glm::vec3* value) {
			auto homogeneousCoord = glm::vec4(*value, 1.0f);
		  	homogeneousCoord = this->GetTransformMatrix() * homogeneousCoord;
		  	*value = homogeneousCoord;
		}

		glm::vec3 TransformPosition(const glm::vec3& value) {
		  glm::vec3 transformedValue = value;
		  TransformPosition(&transformedValue);
		  return transformedValue;
		}

		glm::vec3 position = glm::zero<glm::vec3>();
		glm::quat rotation = glm::identity<glm::quat>();
		glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 halfScale = glm::zero<glm::vec3>();
	};
}

#endif