#pragma once

#include <RF_Light.hpp>
#include <RF_TextRender.hpp>

#include <PrettyEngine/render.hpp>
#include <PrettyEngine/RenderFeature.hpp>

static std::shared_ptr<PrettyEngine::RenderFeature> GetRenderFeature(std::string name) {
    if(name == "RF_Light") {
        return std::make_shared<Custom::RF_Light>();
    }     if(name == "RF_TextRender") {
        return std::make_shared<Custom::RF_TextRender>();
    } 
 return nullptr;
}