#pragma once

#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/render/light.hpp>

#include <Guid.hpp>
#include <string>

namespace Custom {
	class Light: public PrettyEngine::Component {
	public:
		void OnUpdatePublicVariables() override {
			this->CreatePublicVar("Color");
			this->CreatePublicVar("LightFactor");
			this->CreatePublicVar("DeferredFactor");
			this->CreatePublicVar("LightLayer");
			this->CreatePublicVar("Radius");
			this->CreatePublicVar("Type");
			this->CreatePublicVar("SpotLightCutOff");
			this->CreatePublicVar("SpotDirection");
		}

		void OnStart() override {
			auto radius = this->GetPublicVarValue("Radius");
			if (!radius.empty()) {
				this->light.radius = stof(radius);
			}

			auto lightType = this->GetPublicVarValue("Type");
			if (!lightType.empty()) {
				if (lightType == "spot") {
					this->light.lightType = PrettyEngine::LightType::SpotLight;

					// Load spot light specific public variables

					auto cutoff = this->GetPublicVarValue("SpotLightCutOff");
					if (!cutoff.empty()) {
						this->light.spotLightCutOff = stof(cutoff);
					}

					auto spotLightDir = this->GetPublicVarAsFloatVec("SpotDirection");
					if (spotLightDir.size() >= 3) {
						this->light.spotDirection.x = spotLightDir[0];
						this->light.spotDirection.y = spotLightDir[1];
						this->light.spotDirection.z = spotLightDir[2];
					}
				} else {
					this->light.lightType = PrettyEngine::LightType::PointLight;
				}
			}

			auto lightLayer = this->GetPublicVarValue("LightLayer");
			if (!lightLayer.empty()) {
				this->light.lightLayer = std::stoi(lightLayer);
			}

			auto deferredFactor = this->GetPublicVarValue("DeferredFactor");
			if (!deferredFactor.empty()) {
				this->light.deferredFactor = std::stof(deferredFactor);
			}

			auto lightFactor = this->GetPublicVarValue("LightFactor");
			if (!lightFactor.empty()) {
				this->light.lightFactor = std::stof(lightFactor);
			}

			auto color = this->GetPublicVarAsFloatVec("Color");
			if (color.size() == 3) {
				this->light.color.r = color[0];
				this->light.color.g = color[1];
				this->light.color.b = color[2];
			}
			if (color.size() == 4) {
				this->light.opacityFactorEffect = color[3];
			}

			this->engineContent->renderer.RegisterLight(this->lightID, &this->light);
		}

		void OnUpdate() override {
			this->light.position = dynamic_cast<PrettyEngine::Transform*>(this->owner)->position;
		}

		void OnDestroy() override {
			this->engineContent->renderer.UnRegisterLight(&this->light);
		}

	private:
		PrettyEngine::Light light;
		std::string lightID = xg::newGuid();
	};
}
