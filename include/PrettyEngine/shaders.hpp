#pragma once

namespace Shaders {

/// Generated shader variable from file
static const char* SHADER_FRAGMENT_FRAGMENT = R"(#version 430

in vec3 Color;
in vec2 Texcoord;
in vec4 Vertex;

out vec4 outColor;

uniform float time;
uniform vec3 baseColor;

uniform int useTexture;
uniform sampler2D textureBase;

uniform int useTransparencyTexture;
uniform sampler2D transparencyTexture;

uniform int useNormal;
uniform sampler2D normalTexture;

uniform int layer;
uniform int mainLayer;

uniform float opacity;

uniform int useSunLight;
uniform vec3 sunLightColor;
uniform float sunLightFactor;

uniform int useLight;
uniform int lightsCount;
uniform int lightLayer;

#define UNIFORM_ARRAY_SIZE 100

struct Light {
    int layer;
    vec3 position;
    vec3 color;
    float factor;
    float radius;
    float deferredFactor;
    float opacityFactorEffect;
    int spotLight;
    vec3 spotLightDirection;
    float spotLightCutoff;
};

layout(std140) uniform LightArray {
    Light lights[UNIFORM_ARRAY_SIZE];
};

uniform int[UNIFORM_ARRAY_SIZE] lightsLayer;

uniform vec3[UNIFORM_ARRAY_SIZE] lightsPosition;
uniform vec3[UNIFORM_ARRAY_SIZE] lightsColor;
uniform float[UNIFORM_ARRAY_SIZE] lightsFactor;
uniform float[UNIFORM_ARRAY_SIZE] lightsRadius;
uniform float[UNIFORM_ARRAY_SIZE] lightsDeferredFactor;
uniform float[UNIFORM_ARRAY_SIZE] lightsOpacityFactorEffect;

uniform int[UNIFORM_ARRAY_SIZE] spotLight;
uniform vec3[UNIFORM_ARRAY_SIZE] spotLightDirection;
uniform float[UNIFORM_ARRAY_SIZE] spotLightCutOff;

uniform vec3 colorFilter;

vec4 GetVertexLight() {
    vec4 lightOutColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);

    if (useSunLight != 0) {
        lightOutColor = vec4(sunLightColor * sunLightFactor, 1.0f);
    }

    if (useLight != 0) {
        for (int i = 0; i < lightsCount; i++) {
            if (lightsLayer[i] == lightLayer) {
                float distance = distance(Vertex.xyz, lightsPosition[i]);

                float distanceRatio = clamp(1.0 - distance / lightsRadius[i], 0.0, 1.0);
                float intensityReduction = smoothstep(0.0, 1.0, distanceRatio);
                float adjustedFactor = lightsFactor[i] * intensityReduction * lightsDeferredFactor[i];

                if (spotLight[i] != 0) {
                    vec3 lightDirection = normalize(lightsPosition[i] - Vertex.xyz);
                    float cosTheta = dot(lightDirection, normalize(-spotLightDirection[i]));

                    if (cosTheta > cos(spotLightCutOff[i])) {
                        lightOutColor = vec4((vec3(1.0f, 1.0f, 1.0f) + lightOutColor.xyz) * lightsColor[i] * adjustedFactor, lightsOpacityFactorEffect[i]);
                    }
                }
                
                if (spotLight[i] == 0 && distance < lightsRadius[i]) {
                    lightOutColor = vec4((vec3(1.0f, 1.0f, 1.0f) + lightOutColor.xyz) * lightsColor[i] * adjustedFactor, lightsOpacityFactorEffect[i]);
                }
            }
        }
    }

    if (useSunLight == 0 && useLight == 0) {
        return vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    return lightOutColor;
}

void main()
{
    vec4 lightOutColor = GetVertexLight();

    float outOpacity = 1.0f;

    if (useTransparencyTexture == 1) {
        outOpacity = texture(transparencyTexture, Texcoord).a;
    }

    if (useTexture == 0) {
        outColor = vec4(baseColor * Color, opacity) * lightOutColor;
    } else {
        outColor = (texture(textureBase, Texcoord) * vec4(baseColor * Color, opacity)) * lightOutColor;
    }

    if (useNormal == 1 && useLight == 1) {
        for(int i = 0; i < lightsCount; i++) {
            if (lightsLayer[i] == lightLayer) {
                vec3 normalColor = texture(normalTexture, Texcoord).rgb;

                vec3 lightDirection = normalize(lightsPosition[i] - Vertex.xyz);

                float diff = max(dot(normalColor, lightDirection), 0.0);

                outColor *= diff;
            }
        }
    }

    outColor *= vec4(outColor.xyz * colorFilter, outColor.w);
}
)";

/// Generated shader variable from file
static const char* SHADER_VERTEX_VERTEX = R"(#version 430

in vec3 position;
in vec3 color;

in vec2 texcoord;

out vec3 Color;
out vec2 Texcoord;
out vec4 Vertex;

uniform float time;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    Color = color;
    Texcoord = texcoord;
    gl_Position = proj * view * model * vec4(position, 1.0);

    Vertex = model * vec4(position, 1.0);
}
)";


} // Shaders