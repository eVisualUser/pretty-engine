#pragma once

#include <RF_Light.hpp>

#include <PrettyEngine/render.hpp>
#include <PrettyEngine/RenderFeature.hpp>

static std::shared_ptr<PrettyEngine::RenderFeature> GetRenderFeature(std::string name) {
    if(name == "RF_Light") {
        return std::make_shared<Custom::RF_Light>();
    } 
 return nullptr;
}