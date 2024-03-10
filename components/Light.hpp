#pragma once

#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/render/light.hpp>
#include <PrettyEngine/localization.hpp>

#include <Guid.hpp>
#include <string>

namespace Custom {
	class Light: public PrettyEngine::Component {
	public:
		void OnSetup() override {
			this->AddSerializedField(SERIAL_TOKEN(glm::vec3), "Color", "0;0;0");
			this->AddSerializedField(SERIAL_TOKEN(float), "LightFactor", "0");
			this->AddSerializedField(SERIAL_TOKEN(float), "DeferredFactor", "0");
			this->AddSerializedField(SERIAL_TOKEN(int), "LightLayer", "0");
			this->AddSerializedField(SERIAL_TOKEN(float), "Radius", "0");
			this->AddSerializedField(SERIAL_TOKEN(PrettyEngine::LightType), "Type", "point");
			this->AddSerializedField(SERIAL_TOKEN(float), "SpotLightCutOff", "0");
			this->AddSerializedField(SERIAL_TOKEN(glm::vec4), "SpotDirection", "0;0;0;0");
		}

		void OnStart() override {
			auto radius = this->GetSerializedFieldValue("Radius");
			if (!radius.empty()) {
				this->light.radius = stof(radius);
			}

			auto lightType = this->GetSerializedFieldValue("Type");
			if (!lightType.empty()) {
				if (lightType == "spot") {
					this->light.lightType = PrettyEngine::LightType::SpotLight;

					// Load spot light specific public variables

					auto cutoff = this->GetSerializedFieldValue("SpotLightCutOff");
					if (!cutoff.empty()) {
						this->light.spotLightCutOff = stof(cutoff);
					}
					
					auto spotLightDir = PrettyEngine::ParseCSVLine(this->GetSerializedFieldValue("SpotDirection"));
					if (spotLightDir.size() >= 3) {
						this->light.spotDirection.x = std::stof(spotLightDir[0]);
						this->light.spotDirection.y = std::stof(spotLightDir[1]);
						this->light.spotDirection.z = std::stof(spotLightDir[2]);
					}
					
				} else {
					this->light.lightType = PrettyEngine::LightType::PointLight;
				}
			}

			auto lightLayer = this->GetSerializedFieldValue("LightLayer");
			if (!lightLayer.empty()) {
				this->light.lightLayer = std::stoi(lightLayer);
			}

			auto deferredFactor = this->GetSerializedFieldValue("DeferredFactor");
			if (!deferredFactor.empty()) {
				this->light.deferredFactor = std::stof(deferredFactor);
			}

			auto lightFactor = this->GetSerializedFieldValue("LightFactor");
			if (!lightFactor.empty()) {
				this->light.lightFactor = std::stof(lightFactor);
			}

			auto color = PrettyEngine::ParseCSVLine(this->GetSerializedFieldValue("Color"));
			if (color.size() == 4) {
				this->light.color.r = std::stof(color[0]);
				this->light.color.g = std::stof(color[1]);
				this->light.color.b = std::stof(color[2]);
				this->light.opacityFactorEffect = std::stof(color[3]);
			}

			this->engineContent->renderer.UnRegisterLight(&this->light);
			this->engineContent->renderer.RegisterLight(this->lightID, &this->light);
		}

		void OnUpdate() override {
			this->light.position = dynamic_cast<PrettyEngine::Transform*>(this->owner)->position;
		}

		void OnDestroy() override {
			this->engineContent->renderer.UnRegisterLight(&this->light);
		}

		void OnEditorStart() override { this->OnStart(); }
		void OnEditorUpdate() override { this->OnUpdate();	}

	private:
		PrettyEngine::Light light;
		std::string lightID = xg::newGuid();
	};
}
