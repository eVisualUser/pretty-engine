#pragma once

#include <PrettyEngine/render/RenderFeature.hpp>
#include <PrettyEngine/debug/debug.hpp>
#include <PrettyEngine/render/light.hpp>

namespace Custom {
	class RF_Light: public PrettyEngine::RenderFeature {
	public:
		void OnInit() override {
            _flattenedLightsPosition.clear();
            _flattenedLightsColor.clear();
            _flattenedLightsFactor.clear();
            _flattenedLightsDeferredFactor.clear();
            _flattenedLightsOpacityFactorEffect.clear();
            
            _flattenedLightsLayer.clear();
            _flattenedRadius.clear();

            _flattenedLightsSpotLight.clear();
            _flattenedLightsDirection.clear();
            _flattenedLightsCutOff.clear();

            size_t lightsCount = this->lights->size();
            
            for (auto light : *this->lights) {
                _flattenedLightsOpacityFactorEffect.push_back(light->opacityFactorEffect);

                _flattenedLightsPosition.push_back(light->position.x);
                _flattenedLightsPosition.push_back(light->position.y);
                _flattenedLightsPosition.push_back(light->position.z);

                _flattenedLightsColor.push_back(light->color.x);
                _flattenedLightsColor.push_back(light->color.y);
                _flattenedLightsColor.push_back(light->color.z);

                _flattenedLightsFactor.push_back(light->lightFactor);

                _flattenedLightsDeferredFactor.push_back(light->deferredFactor);

                _flattenedLightsDirection.push_back(light->spotDirection.x);
                _flattenedLightsDirection.push_back(light->spotDirection.y);
                _flattenedLightsDirection.push_back(light->spotDirection.z);
                
                _flattenedLightsSpotLight.push_back((int)(light->lightType == PrettyEngine::LightType::SpotLight));
                _flattenedLightsCutOff.push_back(light->spotLightCutOff);

                _flattenedLightsLayer.push_back(light->lightLayer);

                _flattenedRadius.push_back(light->radius);
            }
		}

		void OnUniform(PrettyEngine::VisualObject *visualObject) override {
			auto shaderProgram = visualObject->renderModel->shaderProgram;
			glUniform1i(shaderProgram->uniforms["UseLight"], visualObject->useLight);
            if (visualObject->useLight) {
                glUniform1i(shaderProgram->uniforms["LightsCount"], this->lights->size());
                glUniform1i(shaderProgram->uniforms["LightLayer"], visualObject->lightLayer);
                
                glUniform1iv(shaderProgram->uniforms["LightsLayer"], this->lights->size(), _flattenedLightsLayer.data());

                glUniform3fv(shaderProgram->uniforms["LightsPosition"], this->lights->size(), _flattenedLightsColor.data());
                glUniform3fv(shaderProgram->uniforms["LightsColor"], this->lights->size(), _flattenedLightsColor.data());
                
                glUniform1fv(shaderProgram->uniforms["LightsRadius"], this->lights->size(), _flattenedRadius.data());
                glUniform1fv(shaderProgram->uniforms["LightsFactor"], this->lights->size(), _flattenedLightsFactor.data());
                glUniform1fv(shaderProgram->uniforms["LightsDeferredFactor"], this->lights->size(), _flattenedLightsDeferredFactor.data());
                glUniform1fv(shaderProgram->uniforms["LightsOpacityFactorEffect"], this->lights->size(), _flattenedLightsOpacityFactorEffect.data());
            
                glUniform1iv(shaderProgram->uniforms["SpotLight"], this->lights->size(), _flattenedLightsSpotLight.data());
                glUniform3fv(shaderProgram->uniforms["SpotLightDirection"], this->lights->size(), _flattenedLightsDirection.data());
                glUniform1fv(shaderProgram->uniforms["SpotLightCutOff"], this->lights->size(), _flattenedLightsCutOff.data());
            }
		}

    private:
        std::vector<float> _flattenedLightsPosition;
        std::vector<float> _flattenedLightsColor;
        std::vector<float> _flattenedLightsFactor;
        std::vector<float> _flattenedLightsDeferredFactor;
        std::vector<float> _flattenedLightsOpacityFactorEffect;
        
        std::vector<int> _flattenedLightsLayer;
        std::vector<float> _flattenedRadius;

        std::vector<int> _flattenedLightsSpotLight;
        std::vector<float> _flattenedLightsDirection;
        std::vector<float> _flattenedLightsCutOff;
	};
}
