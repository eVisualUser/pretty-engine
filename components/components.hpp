#pragma once

#include <Light.hpp>
#include <LocalizationEditor.hpp>
#include <Physical.hpp>
#include <Render.hpp>

#include <PrettyEngine/entity.hpp>
#include <PrettyEngine/world.hpp>

#include <string>
#include <memory>

/******************************************************
 * THIS FILE IS AUTO-GENERATED BY A CMAKE SCRIPT.    *
 *                                                   *
 * DO NOT MODIFY THIS FILE MANUALLY, AS IT WILL BE   *
 * REGENERATED AUTOMATICALLY UPON THE NEXT BUILD.    *
 ******************************************************/

static std::shared_ptr<PrettyEngine::Component> GetCustomComponent(std::string name) {
    if(name == "Light") {
        return std::make_shared<Custom::Light>();
    }     if(name == "LocalizationEditor") {
        return std::make_shared<Custom::LocalizationEditor>();
    }     if(name == "Physical") {
        return std::make_shared<Custom::Physical>();
    }     if(name == "Render") {
        return std::make_shared<Custom::Render>();
    } 
 return nullptr;
}