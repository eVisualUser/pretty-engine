#include <PrettyEngine/localization.hpp>
#include <PrettyEngine/light.hpp>
#include <PrettyEngine/collider.hpp>
#include <PrettyEngine/camera.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/Graphics.hpp>
#include <PrettyEngine/assets/builtin.hpp>

#include <PrettyEngine/mesh.hpp>
#include <PrettyEngine/texture.hpp>
#include <PrettyEngine/shaders.hpp>
#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/utils.hpp>
#include <RenderFeatures.hpp>

// GLM
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

// ImGYU
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
        // Vertex Array Object
        unsigned int vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // Vertex Array Buffer
        unsigned int vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        auto buffer = mesh.CreateBuffer();

        glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), buffer.data(), (GLenum)meshDrawType);

        // Element Array Buffer
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

        // Attack all bonus shaders
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

        shaderProgramRefs.CreateUniformsFromCSV(ASSET_BUILTIN_UNIFORMS);

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

        auto filePath = GetEnginePublicPath("RenderFeaturesList.csv", true);
        if (FileExist(filePath)) {
            auto features = ReadFileToString(filePath);

            for (auto & feature: ParseCSVLine(features)) {
                this->AddRenderFeature(GetRenderFeature(feature));
                this->_renderFeatures.back()->OnCreated();
            }
        } else {
            DebugLog(LOG_WARNING, "RenderFeature list not found", true);
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

            for(auto & renderFeature: this->_renderFeatures) {
                renderFeature->lights = &lightsBuffer;
                renderFeature->OnInit();
            }
            
            int width, height;
            glfwGetFramebufferSize(this->_window, &width, &height);

            float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

            // Render for each camera
            for (auto & camera: this->cameraList) {
                if (camera.active) {
                    auto currentCameraMatrix = camera.GetTransformMatrix();
                    auto cameraProjection = camera.projection;
                    this->renderCube.position = camera.position;

                    camera.Render();
                    GL_CHECK_ERROR();

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

                                            Graphics::BindVariable(shaderProgram->uniforms["Model"], modelTransform);

                                            glm::mat4 view = currentCameraMatrix;
                                            glm::mat4 projection = proj;

                                            if (object->screenObject) {
                                                view = glm::identity<glm::mat4>();
                                                projection = glm::identity<glm::mat4>();
                                            }

                                            Graphics::BindVariable(shaderProgram->uniforms["View"], view);
                                            Graphics::BindVariable(shaderProgram->uniforms["Projection"], projection);
                                            Graphics::BindVariable(shaderProgram->uniforms["Time"], currentTime);
                                            Graphics::BindVariable(shaderProgram->uniforms["BaseColor"], object->baseColor);
                                            Graphics::BindVariable(shaderProgram->uniforms["ColorFilter"], camera.colorFilter);

                                            auto baseTexture = object->GetTexture(TextureType::Base);
                                            Graphics::BindVariable(shaderProgram->uniforms["UseTexture"], baseTexture != nullptr);
                                            auto transparencyTexture = object->GetTexture(TextureType::Transparency);
                                            Graphics::BindVariable(shaderProgram->uniforms["UseTransparencyTexture"], transparencyTexture != nullptr);
                                            auto normalTexture = object->GetTexture(TextureType::Normal);
                                            Graphics::BindVariable(shaderProgram->uniforms["UseNormal"], normalTexture != nullptr);

                                            Graphics::BindVariable(shaderProgram->uniforms["Layer"], object->renderLayer);
                                            Graphics::BindVariable(shaderProgram->uniforms["MainLayer"], this->mainLayer);

                                            Graphics::BindVariable(shaderProgram->uniforms["Opacity"], object->opacity);

                                            Graphics::BindVariable(shaderProgram->uniforms["UseSunLight"], object->sunLight);
                                            if (object->sunLight) {
                                                Graphics::BindVariable(shaderProgram->uniforms["SunLightColor"], this->sunColor);
                                                Graphics::BindVariable(shaderProgram->uniforms["SunLightFactor"], this->sunLightFactor);
                                            }

                                            for(auto & renderFeature: this->_renderFeatures) {
                                                renderFeature->OnUniform(object.get());
                                            }

                                            if (object->render) {
                                                for(auto & renderFeature: this->_renderFeatures) {
                                                    renderFeature->OnRender(object.get());
                                                }

                                                if (object->renderModel->useTexture) {
                                                    if (baseTexture != nullptr) {
                                                        glActiveTexture(GL_TEXTURE0);
                                                                    
                                                        glBindTexture(GL_TEXTURE_2D, baseTexture->textureID);
                                                        
                                                        glUniform1i(glGetUniformLocation(
                                                            object->renderModel->shaderProgram->shaderProgram,
                                                            "textureBase"
                                                        ), 0);
                                                    }
                                                    if (transparencyTexture != nullptr) {
                                                        glActiveTexture(GL_TEXTURE1);
                                                                    
                                                        glBindTexture(GL_TEXTURE_2D, transparencyTexture->textureID);
                                                        
                                                        glUniform1i(glGetUniformLocation(
                                                            object->renderModel->shaderProgram->shaderProgram,
                                                            "transparencyTexture"
                                                        ), 0);
                                                    }
                                                    if (normalTexture != nullptr) {
                                                        glActiveTexture(GL_TEXTURE3);
                                                                    
                                                        glBindTexture(GL_TEXTURE_2D, normalTexture->textureID);
                                                        
                                                        glUniform1i(glGetUniformLocation(
                                                            object->renderModel->shaderProgram->shaderProgram,
                                                            "normalTexture"
                                                        ), 0);
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
                    camera.ResetRender();
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
        GLFWmonitor* monitor = nullptr;

        if (this->GetFullscreen()) {
            monitor = glfwGetWindowMonitor(this->_window);
        } else {
            monitor = glfwGetPrimaryMonitor();
        }

        const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);

        glfwSwapBuffers(this->_window);
        glFlush();
    }
}
