#ifndef H_LIGHT
#define H_LIGHT

#include "glm/trigonometric.hpp"
#include <PrettyEngine/texture.hpp>
#include <PrettyEngine/transform.hpp>

#include <string>

#include <glm/vec3.hpp>

namespace PrettyEngine {
	enum class LightType {
		PointLight = 0,
		SpotLight,
	};

	class Light: public Transform {
	public:
		void SetSpotLightCutOff(float angleInDegrees) {
			this->spotLightCutOff = glm::radians(angleInDegrees);
		}

	public:
		float radius = 10.0f;

		int lightLayer = 0;

		float deferredFactor = 1.0f;

		float lightFactor = 1.5f;

		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

		std::string name;

		LightType lightType;

		glm::vec3 spotDirection = glm::vec3(0.0f, 1.0f, 0.0f);
		float spotLightCutOff = glm::radians(35.0f);

		float opacityFactorEffect = 1.0f;
	};
}

#endif