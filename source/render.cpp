#include <PrettyEngine/light.hpp>
#include <PrettyEngine/physics.hpp>
#include <PrettyEngine/camera.hpp>
#include <PrettyEngine/text.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/gl.hpp>

#include <PrettyEngine/mesh.hpp>
#include <PrettyEngine/texture.hpp>
#include <PrettyEngine/shaders.hpp>
#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/utils.hpp>

// GLM
#include <cstring>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/constants.hpp>

// GLFW
#include <GLFW/glfw3.h>
#include <vector>

// STB
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_TRUETYPE_IMPLEMENTATION 
#include <stb/stb_truetype.h>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <Guid.hpp>

#define PRETTY_ENGINE_DEFAULT_WINDOW_NAME "Pretty-Engine Window"

namespace PrettyEngine {
    std::vector<unsigned char> DecodeImage(std::vector<unsigned char>* in, int bytes, int* height, int* width, int* channels) {
        
        stbi_uc* imageData = stbi_load_from_memory(in->data(), bytes, width, height, channels, 0);

        std::vector<unsigned char> out;
        for (int i = 0; i < (*width * *height) * *channels; i++) {
            out.push_back(imageData[i]);
        }

        stbi_image_free(imageData);

        return out;
    }

    Mesh* Renderer::AddMesh(std::string name, Mesh mesh, MeshDrawType meshDrawType) {
        unsigned int vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        unsigned int vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        auto buffer = mesh.CreateBuffer();

        glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), buffer.data(), (GLenum)meshDrawType);

        unsigned int ebo;
        glGenBuffers(1, &ebo);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), (GLenum)meshDrawType);

        mesh.vao = vao;
        mesh.vbo = vbo;
        mesh.ebo = ebo;
        mesh.drawType = meshDrawType;

        this->glMeshList.insert(std::make_pair(name, mesh));

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0);

        // Normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));

        // Texture coordinates
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6));

        glBindVertexArray(0);

        return &this->glMeshList[name];
    }

    void Renderer::UpdateMesh(Mesh* mesh) {
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);

        auto data = mesh->CreateBuffer();

        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), (GLenum)mesh->drawType);
    }

    GLShaderProgramRefs* Renderer::AddShaderProgram(std::string name, std::string vertexShaderName, std::string fragmentShaderName, std::vector<std::string> otherShaders) {
        unsigned int shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, this->glShaders[vertexShaderName]);
        glAttachShader(shaderProgram, this->glShaders[fragmentShaderName]);

        for (auto & shader: otherShaders) {
            glAttachShader(shaderProgram, this->glShaders[shader]);
        }

        glBindFragDataLocation(shaderProgram, 0, "outColor");

        glLinkProgram(shaderProgram);
        glUseProgram(shaderProgram);

        GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
        glEnableVertexAttribArray(posAttrib);
        glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);

        GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
        glEnableVertexAttribArray(colAttrib);
        glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(float)));

        GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
        glEnableVertexAttribArray(texAttrib);
        glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE,
                               8*sizeof(float), (void*)(6*sizeof(float)));

        auto shaderProgramRefs = GLShaderProgramRefs();
        shaderProgramRefs.shaderProgram = shaderProgram;

        shaderProgramRefs.CreateUniform("Time", "time");
        shaderProgramRefs.CreateUniform("Model", "model");
        shaderProgramRefs.CreateUniform("View", "view");
        shaderProgramRefs.CreateUniform("Projection", "proj");
        shaderProgramRefs.CreateUniform("BaseColor", "baseColor");
        shaderProgramRefs.CreateUniform("UseTexture", "useTexture");
        shaderProgramRefs.CreateUniform("Layer", "layer");
        shaderProgramRefs.CreateUniform("MainLayer", "mainLayer");
        shaderProgramRefs.CreateUniform("Opacity", "opacity");
        shaderProgramRefs.CreateUniform("RenderText", "renderText");
        shaderProgramRefs.CreateUniform("TextOutLineWidth", "textOutLineWidth");
        shaderProgramRefs.CreateUniform("ColorFilter", "colorFilter");
        // Light
        shaderProgramRefs.CreateUniform("LightsCount", "lightsCount");
        shaderProgramRefs.CreateUniform("LightsPosition", "lightsPosition");
        shaderProgramRefs.CreateUniform("LightsColor", "lightsColor");
        shaderProgramRefs.CreateUniform("LightsFactor", "lightsFactor");
        shaderProgramRefs.CreateUniform("LightsRadius", "lightsRadius");
        shaderProgramRefs.CreateUniform("LightsDeferredFactor", "lightsDeferredFactor");
        shaderProgramRefs.CreateUniform("LightsLayer", "lightsLayer");
        shaderProgramRefs.CreateUniform("LightLayer", "lightLayer");
        shaderProgramRefs.CreateUniform("UseLight", "useLight");
        shaderProgramRefs.CreateUniform("LightsOpacityFactorEffect", "lightsOpacityFactorEffect");
        
        /// Spot Light
        shaderProgramRefs.CreateUniform("SpotLight", "spotLight");
        shaderProgramRefs.CreateUniform("SpotLightDirection", "spotLightDirection");
        shaderProgramRefs.CreateUniform("SpotLightCutOff", "spotLightCutOff");
        /// Sun Light
        shaderProgramRefs.CreateUniform("UseSunLight", "useSunLight");
        shaderProgramRefs.CreateUniform("SunLightColor", "sunLightColor");
        shaderProgramRefs.CreateUniform("SunLightFactor", "sunLightFactor");

        this->glShaderPrograms.insert(std::make_pair(name, shaderProgramRefs));
        return &this->glShaderPrograms[name];
    }

    Texture* Renderer::AddTexture(
        std::string name,
        std::string filePath,
        TextureType textureType,
        TextureWrap wrap,
        TextureFilter filter,
        TextureChannels channels
    ) {
        auto textureExist = this->TextureExist(name);
        if (!textureExist.first) {
            Texture texture;

            if (FileExist(filePath)) {
                unsigned int textureID;
                glGenTextures(1, &textureID);
                if (!textureID) {
                    DebugLog(LOG_ERROR, "Failed to generate texture for: " << filePath, true);                
                    std::exit(-1);
                }

                int width, height;

                unsigned char* data = stbi_load(filePath.c_str(), &width, &height, nullptr, 0);
                if (!data) {
                    DebugLog(LOG_ERROR, "Failed to load image: " << filePath, true);
                    std::exit(-1);
                }

                glBindTexture(GL_TEXTURE_2D, textureID);

                glTexImage2D(GL_TEXTURE_2D, 0, (GLenum)channels, width, height, 0, (GLenum)channels, GL_UNSIGNED_BYTE, data);

                stbi_image_free(data);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLenum)wrap);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLenum)wrap);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLenum)filter);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLenum)filter);

                texture.textureID = textureID;
                texture.textureType = textureType;
                texture.wrap = wrap; 
                texture.filter = filter;
                texture.name = name;

                this->glTextures.insert(std::make_pair(name, texture));

                return &this->glTextures[name];
            } else {
                DebugLog(LOG_ERROR, "File not found: " << filePath, true);
            }
        }
        return textureExist.second;
    }

    Texture* Renderer::AddTextureFromData(
        std::string name,
        unsigned char* data,
        int width,
        int height,
        TextureType textureType,
        TextureWrap wrap,
        TextureFilter filter,
        TextureChannels channels
    ) {
        auto textureExist = this->TextureExist(name);

        if (!textureExist.first) {
            Texture texture;

            unsigned int textureID;
            glGenTextures(1, &textureID);

            glBindTexture(GL_TEXTURE_2D, textureID);

            glTexImage2D(GL_TEXTURE_2D, 0, (GLenum)channels, width, height, 0, (GLenum)channels, GL_UNSIGNED_BYTE, data);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLenum)wrap);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLenum)wrap);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLenum)filter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLenum)filter);

            texture.textureID = textureID;
            texture.textureType = textureType;
            texture.wrap = wrap; 
            texture.filter = filter;
            texture.name = name;

            this->glTextures.insert(std::make_pair(name, texture));

            return &this->glTextures[name];
        }
        return textureExist.second;
    }

    void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    }

    void Renderer::DrawLine(glm::vec3 start, glm::vec3 end, glm::vec4 color, float width) {
        glLineWidth(width);
        glColor4d(color.r, color.g, color.b, color.a);
        
        glBegin(GL_LINES);
        
        glVertex3d(start.x, start.y, start.z);
        glVertex3d(end.x, end.y, end.z); 

        glEnd();
    }

    Renderer::Renderer() {
    	if (!glfwInit()) {
    		std::exit(-1);
    	}
    }

    Renderer::~Renderer() {
        glDeleteFramebuffers(1, &this->frameBuffer);

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwTerminate();
        this->Clear();
    }

    void WindowCloseFunc() {
        std::exit(0);
    }

    void Renderer::CreateWindow() {
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
        glfwWindowHint(GLFW_DEPTH_BITS, GLFW_TRUE);
        
        glfwWindowHint(GLFW_SAMPLES, this->antialiasing);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);

        glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_TRUE); 

        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
        
    	auto window = glfwCreateWindow(800, 600, PRETTY_ENGINE_DEFAULT_WINDOW_NAME, nullptr, nullptr);

        glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

        glfwMakeContextCurrent(window);

    	this->_window = window;

        this->HideWindow();
    }

    void Renderer::ShowWindow() {
    	glfwShowWindow(this->_window);
    }

    void Renderer::Setup(glm::vec3 renderCubeScale) {
        this->renderCube.scale = renderCubeScale;
        this->renderCube.colliderModel = ColliderModel::AABB;
        this->renderCube.UpdateHalfScale();

        if (!gladLoadGL()) {
            DebugLog(LOG_ERROR, "Failed to initialize glad", true);
            glfwTerminate();
            std::exit(-1);
        }

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            glfwTerminate();
            std::exit(-1);
        }

        glEnable(GL_MULTISAMPLE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        this->imGUIContext = ImGui::CreateContext();

        this->imGUIIO = new ImGuiIO();
        this->imGUIIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        this->imGUIIO->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        GL_CHECK_ERROR();

        ImGui_ImplGlfw_InitForOpenGL(this->_window, true);
        ImGui_ImplOpenGL3_Init();
    }

    void FlatenThread(
            std::vector<Light*>* lightsBuffer,
            std::vector<float>* flattenedLightsPosition,
            std::vector<float>* flattenedLightsColor,
            std::vector<float>* flattenedLightsFactor,
            std::vector<float>* flattenedLightsDeferredFactor,
            std::vector<float>* flattenedLightsOpacityFactorEffect
        ) {
        for (const auto& light : *lightsBuffer) {
                flattenedLightsOpacityFactorEffect->push_back(light->opacityFactorEffect);

                flattenedLightsPosition->push_back(light->position.x);
                flattenedLightsPosition->push_back(light->position.y);
                flattenedLightsPosition->push_back(light->position.z);

                flattenedLightsColor->push_back(light->color.x);
                flattenedLightsColor->push_back(light->color.y);
                flattenedLightsColor->push_back(light->color.z);

                flattenedLightsFactor->push_back(light->lightFactor);

                flattenedLightsDeferredFactor->push_back(light->deferredFactor);
        }
    }

    void Renderer::UpdateIO() {
        glfwPollEvents();

        this->deltaTime =  glfwGetTime() - this->_lastFrame;
        this->_lastFrame = glfwGetTime();
    }

    void Renderer::StartUIRendering() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Renderer::Draw() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        float currentTime = static_cast<float>(glfwGetTime());

        bool isMinimized = glfwGetWindowAttrib(this->_window, GLFW_ICONIFIED);
        bool isFocused = glfwGetWindowAttrib(this->_window, GLFW_FOCUSED);

        if (!isMinimized && isFocused) {
            // Draw all VisualObjects filtered by the layers
            unsigned int layerCount = 0;

            std::vector<Light*> lightsBuffer;

            for (auto & light: this->lights) {
                if (this->renderCube.PointIn(light->position)) {
                    lightsBuffer.push_back(light);
                }
            }

            size_t lightsCount = lightsBuffer.size();

            std::vector<float> flattenedLightsPosition;
            std::vector<float> flattenedLightsColor;
            std::vector<float> flattenedLightsFactor;
            std::vector<float> flattenedLightsDeferredFactor;
            std::vector<float> flattenedLightsOpacityFactorEffect;
            
            std::vector<int> flattenedLightsLayer;
            std::vector<float> flattenedRadius;

            std::vector<int> flattenedLightsSpotLight;
            std::vector<float> flattenedLightsDirection;
            std::vector<float> flattenedLightsCutOff;

            this->flatenThread = std::thread(FlatenThread, 
                &lightsBuffer,
                &flattenedLightsPosition,
                &flattenedLightsColor,
                &flattenedLightsFactor,
                &flattenedLightsDeferredFactor,
                &flattenedLightsOpacityFactorEffect
            );
            
            for (const auto& light : lightsBuffer) {
                flattenedLightsDirection.push_back(light->spotDirection.x);
                flattenedLightsDirection.push_back(light->spotDirection.y);
                flattenedLightsDirection.push_back(light->spotDirection.z);
                
                flattenedLightsSpotLight.push_back((int)(light->lightType == LightType::SpotLight));
                flattenedLightsCutOff.push_back(light->spotLightCutOff);

                flattenedLightsLayer.push_back(light->lightLayer);

                flattenedRadius.push_back(light->radius);
            }
            
            int width, height;
            glfwGetFramebufferSize(this->_window, &width, &height);

            float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

            this->flatenThread.join();

            for (auto & camera: this->cameraList) {
                if (camera.mainCamera) {
                    auto currentCameraMatrix = camera.GetTransformMatrix();
                    auto cameraProjection = camera.projection;
                    this->renderCube.position = camera.position;

                    int layerId;
                    for (auto & layer: this->visualObjects) {
                        if (!CheckIfVectorContain(&this->hiddenLayers, &layerCount)) {
                            for(auto & visualObject: layer) {
                                auto object = visualObject.second;

                                if (object->d3) {
                                    this->SwitchTo3D();
                                } else {
                                    this->SwitchTo2D();
                                }

                                if (object != nullptr && object->active) {
                                    if (!object->allowRenderCube || this->renderCube.PointIn(object->position)) {
                                        glm::mat4 proj = glm::identity<glm::mat4>();
                                        if (object->renderModel->overrideProjection) {
                                            object->renderModel->projection->aspectRatio = aspectRatio;
                                            proj = glm::perspective(glm::radians(object->renderModel->projection->fov), object->renderModel->projection->aspectRatio, object->renderModel->projection->nearPlane, object->renderModel->projection->farPlane);
                                        } else {
                                            cameraProjection->aspectRatio = aspectRatio;
                                            proj = glm::perspective(glm::radians(cameraProjection->fov), cameraProjection->aspectRatio, cameraProjection->nearPlane, cameraProjection->farPlane);
                                        }

                                        const auto mesh = object->renderModel->mesh;
                                        const auto shaderProgram = object->renderModel->shaderProgram;
                                        if (shaderProgram == nullptr) {
                                            DebugLog(LOG_ERROR, "Missing shader program for: " << visualObject.first, false);
                                        } else if (mesh == nullptr) {
                                            DebugLog(LOG_ERROR, "Missing mesh for: " << visualObject.first, false);
                                        } else {
                                            glBindVertexArray(mesh->vao);
                                            glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
                                            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
                                            GL_CHECK_ERROR();

                                            glUseProgram(shaderProgram->shaderProgram);

                                            auto modelTransform = object->GetTransformMatrix();
                                            if (object->haveParent) {
                                                modelTransform = modelTransform * object->parent->GetTransformMatrix();
                                            }

                                            for(auto & uniformMaker: this->_uniformMakers) {
                                                uniformMaker(object.get(), &camera);
                                            }

                                            glUniformMatrix4fv(shaderProgram->uniforms["Model"], 1, GL_FALSE, glm::value_ptr(modelTransform));

                                            glUniformMatrix4fv(shaderProgram->uniforms["View"], 1, GL_FALSE, glm::value_ptr(currentCameraMatrix));
                                            glUniformMatrix4fv(shaderProgram->uniforms["Projection"], 1, GL_FALSE, glm::value_ptr(proj));
                                            
                                            glUniform1f(shaderProgram->uniforms["Time"], currentTime);

                                            glUniform3fv(shaderProgram->uniforms["BaseColor"], 1, glm::value_ptr(object->baseColor));
                                            glUniform3fv(shaderProgram->uniforms["ColorFilter"], 1, glm::value_ptr(camera.colorFilter));

                                            glUniform1i(shaderProgram->uniforms["UseTexture"], object->renderModel->useTexture);

                                            glUniform1i(shaderProgram->uniforms["Layer"], object->renderLayer);
                                            glUniform1i(shaderProgram->uniforms["MainLayer"], this->mainLayer);

                                            glUniform1f(shaderProgram->uniforms["Opacity"], object->opacity);

                                            glUniform1i(shaderProgram->uniforms["UseSunLight"], object->sunLight);
                                            if (object->sunLight) {
                                                glUniform3fv(shaderProgram->uniforms["SunLightColor"], 1, glm::value_ptr(this->sunColor));
                                                glUniform1f(shaderProgram->uniforms["SunLightFactor"], this->sunLightFactor);
                                            }

                                            glUniform1i(shaderProgram->uniforms["UseLight"], object->useLight);
                                            if (object->useLight) {
                                                glUniform1i(shaderProgram->uniforms["LightsCount"], lightsCount);
                                                glUniform1i(shaderProgram->uniforms["LightLayer"], object->lightLayer);
                                                
                                                glUniform1iv(shaderProgram->uniforms["LightsLayer"], lightsCount, flattenedLightsLayer.data());

                                                glUniform3fv(shaderProgram->uniforms["LightsPosition"], lightsCount, flattenedLightsColor.data());
                                                glUniform3fv(shaderProgram->uniforms["LightsColor"], lightsCount, flattenedLightsColor.data());
                                                
                                                glUniform1fv(shaderProgram->uniforms["LightsRadius"], lightsCount, flattenedRadius.data());
                                                glUniform1fv(shaderProgram->uniforms["LightsFactor"], lightsCount, flattenedLightsFactor.data());
                                                glUniform1fv(shaderProgram->uniforms["LightsDeferredFactor"], lightsCount, flattenedLightsDeferredFactor.data());
                                                glUniform1fv(shaderProgram->uniforms["LightsOpacityFactorEffect"], lightsCount, flattenedLightsOpacityFactorEffect.data());
                                            
                                                glUniform1iv(shaderProgram->uniforms["SpotLight"], lightsCount, flattenedLightsSpotLight.data());
                                                glUniform3fv(shaderProgram->uniforms["SpotLightDirection"], lightsCount, flattenedLightsDirection.data());
                                                glUniform1fv(shaderProgram->uniforms["SpotLightCutOff"], lightsCount, flattenedLightsCutOff.data());
                                            }

                                            if (object->render) {
                                                if (object->renderModel->useTexture) {
                                                    for (auto & texture: object->textures) {
                                                        if (texture && texture->textureID && texture->textureType == TextureType::Base) {
                                                            glActiveTexture(GL_TEXTURE0);
                                                            
                                                            glBindTexture(GL_TEXTURE_2D, texture->textureID);
                                                            
                                                            glUniform1i(glGetUniformLocation(
                                                                object->renderModel->shaderProgram->shaderProgram,
                                                                "textureBase"
                                                            ), 0);
                                                        }
                                                    }
                                                }

                                                glDrawElements((GLenum)object->renderModel->drawMode, mesh->vertexCount, GL_UNSIGNED_INT, 0);
                                            }
                                            object->OnDraw((void*)this);
                                            glBindVertexArray(0);
                                            GL_CHECK_ERROR();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void Renderer::Render() {
        this->Draw();
        this->Show();
    }

    void Renderer::Show() {    	
        glFlush();

        GLFWmonitor* monitor;

        if (this->GetFullscreen()) {
            monitor = glfwGetWindowMonitor(this->_window);
        } else {
            monitor = glfwGetPrimaryMonitor();
        }

        const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);

        if (this->_targetFrameRate == 0) {
            this->_targetFrameRate = videoMode->refreshRate;
        }

        int interval = videoMode->refreshRate / this->_targetFrameRate;

        glfwSwapInterval(interval);

        glfwSwapBuffers(this->_window);
    }
}
