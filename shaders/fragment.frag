#version 430

in vec3 Color;
in vec2 Texcoord;
in vec4 Vertex;

out vec4 outColor;

uniform float time;
uniform vec3 baseColor;
uniform int useTexture;

uniform sampler2D textureBase;

uniform int layer;
uniform int mainLayer;

uniform float opacity;

uniform int renderText;
uniform float textOutLineWidth;

uniform int useSunLight;
uniform vec3 sunLightColor;
uniform float sunLightFactor;

uniform int useLight;
uniform int lightsCount;
uniform int lightLayer;

#define UNIFORM_ARRAY_SIZE 100

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

    if (useSunLight == 0 || useLight == 0) {
        vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    if (useSunLight != 0) {
        lightOutColor = vec4(sunLightColor * sunLightFactor, 1.0f);
    }

    if (useLight != 0) {
        int haveLightning = 0;
        for (int i = 0; i < lightsCount; i++) {
            if (lightsLayer[i] == lightLayer) {
                float distance = distance(Vertex.xyz, lightsPosition[i]);

                float distanceRatio = clamp(1.0 - distance / lightsRadius[i], 0.0, 1.0);
                float intensityReduction = smoothstep(0.0, 1.0, distanceRatio);
                float adjustedFactor = lightsFactor[i] * intensityReduction * lightsDeferredFactor[i];

                vec3 lightDirection = normalize(lightsPosition[i] - Vertex.xyz);
                float cosTheta = dot(lightDirection, normalize(-spotLightDirection[i]));
                
                if (spotLight[i] != 0 && cosTheta > cos(spotLightCutOff[i])) {
                    lightOutColor = vec4((vec3(1.0f, 1.0f, 1.0f) + lightOutColor.xyz) * lightsColor[i] * adjustedFactor, lightsOpacityFactorEffect[i]);
                    haveLightning = 1;
                } else if (spotLight[i] == 0 && distance < lightsRadius[i]) {
                    lightOutColor = vec4((vec3(1.0f, 1.0f, 1.0f) + lightOutColor.xyz) * lightsColor[i] * adjustedFactor, lightsOpacityFactorEffect[i]);
                    haveLightning = 1;
                }
            }
        }
    }

    return lightOutColor;
}

void main()
{
    vec4 lightOutColor = GetVertexLight();

    if (useTexture == 0) {
        outColor = vec4(baseColor * Color, opacity) * lightOutColor;
    } else if (renderText == 0) {
        outColor = texture(textureBase, Texcoord) * vec4(baseColor * Color, opacity) * lightOutColor;
    } else {
        float smoothing = textOutLineWidth / textureSize(textureBase, 0).x;
        float alpha = texture(textureBase, Texcoord).r;
        float smoothedAlpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, alpha);
        
        vec4 sampled = vec4(1.0, 1.0, 1.0, texture(textureBase, Texcoord).r);
        outColor = vec4(baseColor, smoothedAlpha) * lightOutColor;
    }

    outColor *= vec4(outColor.xyz * colorFilter, outColor.w);
}
