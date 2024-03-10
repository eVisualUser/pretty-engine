#pragma once

/******************************************************
 * THIS FILE IS AUTO-GENERATED BY A CMAKE SCRIPT.    *
 *                                                   *
 * DO NOT MODIFY THIS FILE MANUALLY, AS IT WILL BE   *
 * REGENERATED AUTOMATICALLY UPON THE NEXT BUILD.    *
 ******************************************************/

static const char* ASSET_BUILTIN_CONFIG = R"([engine.render]
window_title = "Default Window Name"
antiAliasing = 16

[engine.render.camera]
create_default_camera = true
set_default_camera_as_main = true

[engine.render.opengl]
background_color = [0.245, 0.40, 0.65, 0.5]

[engine.world.preload]
demo = [] # Preload meshes to allow world loading

[engine]
database = "public/data/engine.db"
logs_limit = 100)";

static const char* ASSET_BUILTIN_EDITOR_CONFIG = R"([engine.render]
window_title = "Game Editor"
antiAliasing = 16

[engine.render.camera]
create_default_camera = true
set_default_camera_as_main = true

[engine.render.opengl]
background_color = [0.245, 0.40, 0.65, 1.0]

[engine.world.preload]
demo = [] # Preload meshes to allow world loading

[engine]
database = "data/engine.db"
)";

static const char* ASSET_BUILTIN_EXAMPLE = R"(# Example

This is an asset example ...
)";

static const char* ASSET_BUILTIN_UNIFORMS = R"(Time;time;
Model;model;
View;view;
Projection;proj;
BaseColor;baseColor;
UseTexture;useTexture;
UseTransparencyTexture;UseTransparencyTexture;
UseNormal;UseNormal;
UseTexture;useTexture;
Layer;layer;
MainLayer;mainLayer;
Opacity;opacity;
ColorFilter;colorFilter;
LightsCount;lightsCount;
LightsPosition;lightsPosition;
LightsColor;lightsColor;
LightsFactor;lightsFactor;
LightsRadius;lightsRadius;
LightsDeferredFactor;lightsDeferredFactor;
LightsLayer;lightsLayer;
LightLayer;lightLayer;
UseLight;useLight;
LightsOpacityFactorEffect;lightsOpacityFactorEffect;
SpotLight;spotLight;
SpotLightDirection;spotLightDirection;
SpotLightCutOff;spotLightCutOff;
UseSunLight;useSunLight;
SunLightColor;sunLightColor;
SunLightFactor;sunLightFactor;)";
