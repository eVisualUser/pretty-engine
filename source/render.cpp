#include "PrettyEngine/serial.hpp"
#include <PrettyEngine/localization.hpp>
#include <PrettyEngine/render/light.hpp>
#include <PrettyEngine/collider.hpp>
#include <PrettyEngine/render/camera.hpp>
#include <PrettyEngine/render/render.hpp>
#include <PrettyEngine/render/Graphics.hpp>
#include <PrettyEngine/assets/builtin.hpp>

#include <PrettyEngine/render/mesh.hpp>
#include <PrettyEngine/render/texture.hpp>
#include <PrettyEngine/debug/debug.hpp>
#include <PrettyEngine/utils.hpp>
#include <RenderFeatures.hpp>

// GLM
#include <future>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

    Mesh* Renderer::AddMesh(std::string& name, Mesh mesh, MeshDrawType meshDrawType) {
        /// Avoid loading multiple time the same mesh
        for (auto &exisitingMesh : this->glMeshList) {
			if (exisitingMesh.first == name) {
				return &this->glMeshList[name];
            }
        }

        // Vertex Array Object
        unsigned int vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // Vertex Array Buffer
        unsigned int vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        const auto buffer = mesh.CreateVertexBuffer();

        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(buffer.size() * sizeof(float)), buffer.data(), static_cast<GLenum>(meshDrawType));

        // Element Array Buffer
        unsigned int ebo;
        glGenBuffers(1, &ebo);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(mesh.indices.size() * sizeof(unsigned int)), mesh.indices.data(), static_cast<GLenum>(meshDrawType));

        mesh.vao = vao;
        mesh.vbo = vbo;
        mesh.ebo = ebo;
        mesh.drawType = meshDrawType;

        this->glMeshList.insert(std::make_pair(name, mesh));

    	// Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, nullptr);

        // Normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));

        // Texture coordinates
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6));

        glBindVertexArray(0);

        return &this->glMeshList[name];
    }

    void Renderer::UpdateMesh(Mesh* mesh) const {
    	if (mesh != nullptr) {
    		glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);

    		const auto data = mesh->CreateVertexBuffer();

    		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(data.size() * sizeof(float)), data.data(), static_cast<GLenum>(mesh->drawType));

    		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    		glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(mesh->indices.size() * sizeof(unsigned int)), mesh->indices.data(), static_cast<GLenum>(mesh->drawType));

    		// Position
    		glEnableVertexAttribArray(0);
    		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, nullptr);

    		// Normals
    		glEnableVertexAttribArray(1);
    		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, reinterpret_cast<void*>(sizeof(float) * 3));

    		// Texture coordinates
    		glEnableVertexAttribArray(2);
    		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, reinterpret_cast<void*>(sizeof(float) * 6));

    		glBindVertexArray(0);
    	} else {
    		DebugLog(LOG_ERROR, "Updated mesh with nullptr.", false);
    	}
    }

    GLShaderProgramRefs* Renderer::AddShaderProgram(std::string name, std::string vertexShaderName, std::string fragmentShaderName, std::vector<std::string> otherShaders) {
		if (!this->glShaderPrograms.contains(name)) {
			if (GL_CHECK_ERROR()) {
				DebugLog(LOG_ERROR, "OpengGL error before create ShaderProgram: " << name, true);
			}

			const unsigned int shaderProgram = glCreateProgram();
			glAttachShader(shaderProgram, this->glShaders[vertexShaderName]);
			glAttachShader(shaderProgram, this->glShaders[fragmentShaderName]);

			// Attack all bonus shaders
			for (auto & shader: otherShaders) {
				glAttachShader(shaderProgram, this->glShaders[shader]);
			}

			glBindFragDataLocation(shaderProgram, 0, "outColor");

			glLinkProgram(shaderProgram);
			glUseProgram(shaderProgram);

			const GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
			glEnableVertexAttribArray(posAttrib);
			glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);

			const GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
			glEnableVertexAttribArray(colAttrib);
			glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(float)));

			const GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
			glEnableVertexAttribArray(texAttrib);
			glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE,
								   8*sizeof(float), reinterpret_cast<void *>(6 * sizeof(float)));

			auto shaderProgramRefs = GLShaderProgramRefs();
			shaderProgramRefs.shaderProgram = shaderProgram;

			shaderProgramRefs.CreateUniformsFromCSV(ASSET_BUILTIN_UNIFORMS);

			for (auto &renderFeature : this->_renderFeatures) {
				renderFeature->OnShaderProgram(&shaderProgramRefs);
			}

			this->glShaderPrograms.insert(std::make_pair(name, shaderProgramRefs));

			if (GL_CHECK_ERROR()) {
				DebugLog(LOG_ERROR, "OpengGL error after create ShaderProgram: " << name, true);
			}
		}
    	return &this->glShaderPrograms[name];
    }

    Texture* Renderer::AddTexture(
        std::string name,
        Asset* asset,
        TextureType textureType,
        TextureWrap wrap,
        TextureFilter filter,
        TextureChannels channels
    ) {
    	auto textureExist = this->TextureExist(name);
        if (!textureExist.first) {
			if (asset->Exist()) {
				if (GL_CHECK_ERROR()) {
					DebugLog(LOG_ERROR, "Opengl error happended before loading of: " << asset->GetFilePath(), true);
				}

				Texture texture;

				asset->SetUsed(true);
                asset->AddSerializedField(SERIAL_TOKEN(int), "textureChannels", std::to_string(static_cast<int>(channels)));
                channels = static_cast<TextureChannels>(std::stoi(asset->GetSerializedFieldValue("textureChannels")));

                unsigned int textureID;
                glGenTextures(1, &textureID);
                if (!textureID) {
					DebugLog(LOG_ERROR, "Failed to generate texture for: " << asset->GetFilePath(), true);                
                    std::exit(-1);
                }

                int width, height;

                unsigned char *data = stbi_load(asset->GetFilePath().c_str(), &width, &height, nullptr, 0);
                if (!data) {
					DebugLog(LOG_ERROR, "Failed to load image: " << asset->GetFilePath(), true);
                    std::exit(-1);
                }

                glBindTexture(GL_TEXTURE_2D, textureID);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, width, height, 0, (GLint)channels, GL_UNSIGNED_BYTE, data);

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

				glBindTexture(GL_TEXTURE_2D, 0);

				if (GL_CHECK_ERROR()) {
					DebugLog(LOG_ERROR, "Error at texture loading of: " << asset->GetFilePath(), true);

					glDeleteTextures(1, &textureID);

					return nullptr;
				}

                this->glTextures.insert(std::make_pair(name, texture));

                return &this->glTextures[name];
            } else {
				DebugLog(LOG_ERROR, "File not found: " << asset->GetFilePath(), true);
            }
        }
    	DebugLog(LOG_DEBUG, "Tried to add existing texture", false);
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

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLenum>(wrap));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLenum>(wrap));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(filter));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(filter));

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
            DebugLog(LOG_ERROR, "Failed to open GLFW Window !", true);
    		std::exit(-1);
    	}
    }

    Renderer::~Renderer() {
        glDeleteFramebuffers(1, &this->frameBuffer);

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(this->_window);
        glfwTerminate();

    	this->_renderFeatures.clear();
    	this->visualObjects.clear();
        this->Clear(true);
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

#if ENGINE_EDITOR
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

    	const auto window = glfwCreateWindow(800, 600, PRETTY_ENGINE_DEFAULT_WINDOW_NAME, nullptr, nullptr);

        glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

        glfwMakeContextCurrent(window);

    	this->_window = window;
    }

    void Renderer::ShowWindow() const {
    	glfwShowWindow(this->_window);
    }

    void Renderer::Setup(glm::vec3 renderCubeScale) {
        this->renderCube.scale = renderCubeScale;
        this->renderCube.UpdateHalfScale();

        if (!gladLoadGL()) {
            DebugLog(LOG_ERROR, "Failed to initialize glad", true);
            glfwTerminate();
            std::exit(-1);
        }

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
            glfwTerminate();
            std::exit(-1);
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        auto syncRenderFeature = std::async([this] {
			auto filePath = GetEnginePublicPath("RenderFeaturesList.csv", true);
			if (FileExist(filePath)) {
				auto features = ReadFileToString(filePath);

				for (auto &feature : ParseCSVLine(features)) {
					this->AddRenderFeature(GetRenderFeature(feature));
					this->_renderFeatures.back()->OnCreated();
				}
			} else {
				DebugLog(LOG_WARNING, "RenderFeature list not found", true);
			}
		});

        this->imGUIContext = ImGui::CreateContext();

        this->imGUIIO = new ImGuiIO();
        this->imGUIIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        this->imGUIIO->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        ImGui_ImplGlfw_InitForOpenGL(this->_window, true);
		ImGui_ImplOpenGL3_Init();

        syncRenderFeature.get();

    	if (GL_CHECK_ERROR()) {
    		DebugLog(LOG_ERROR, "OpenGL Error during renderer setup !", true);
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

    	if (GL_CHECK_ERROR()) {
    		DebugLog(LOG_ERROR, "OpenGL Error during UI rendering startup !", true);
    	}
    }

    void Renderer::Draw() {
#if ENGINE_EDITOR
    	if (GL_CHECK_ERROR()) {
    		DebugLog(LOG_ERROR, "OpenGL Error before drawing frame !", true);
    	}
#endif

        auto currentTime = static_cast<float>(glfwGetTime());

        const bool isMinimized = glfwGetWindowAttrib(this->_window, GLFW_ICONIFIED);
        const bool isFocused = glfwGetWindowAttrib(this->_window, GLFW_FOCUSED);

        if (!isMinimized && isFocused) {
            // Draw all VisualObjects filtered by the layers
            unsigned int layerCount = 0;

            for (auto &renderFeature : this->_renderFeatures) {
				renderFeature->lights = &this->lights;
				renderFeature->OnInit();
			}
            
            int width, height;
            glfwGetFramebufferSize(this->_window, &width, &height);

            float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            auto renderImGUI = std::async([this]{
                ImGui::Render();
            });

            // Render for each camera
            for (auto & camera: this->cameraList) {
                if (camera.active) {
					glViewport(width * camera.viewportPositionRatio.x, height * camera.viewportPositionRatio.y, width * camera.viewportSizeRatio.x, height * camera.viewportSizeRatio.y);

                    auto currentCameraMatrix = camera.GetTransformMatrix();
                    auto cameraProjection = camera.projection;
                    this->renderCube.position = camera.position;

                    if (camera.renderToTexture) {
						camera.Render();
					} else {
						camera.ResetRender();
                    }

                    if (GL_CHECK_ERROR()) {
                        DebugLog(LOG_WARNING, "Got an OpenGL error before rendering !", false);
                    }

                    int layerId = 0;
                	unsigned int lastShaderProgram = UINT_MAX;
                    for (auto & layer: this->visualObjects) {
                        if (!CheckIfVectorContain(&this->hiddenLayers, &layerCount)) {
                            for(auto & visualObject: layer) {
                                auto & object = visualObject.second;

                                if (object->d3) {
                                    this->SwitchTo3D();
                                } else {
                                    this->SwitchTo2D();
                                }

                                if (object != nullptr && object->renderModel != nullptr && object->active) {
                                    auto projection = glm::identity<glm::mat4>();

                                    if (!object->screenObject) {
                                        if (object->renderModel->projection != nullptr) {
                                        	if (object->renderModel->overrideProjection) {
                                        		object->renderModel->projection->aspectRatio = aspectRatio;
                                        		projection = glm::perspective(glm::radians(object->renderModel->projection->fov), object->renderModel->projection->aspectRatio, object->renderModel->projection->nearPlane, object->renderModel->projection->farPlane);
                                        	}
                                        } else {
                                            cameraProjection->aspectRatio = aspectRatio;
                                            projection = glm::perspective(glm::radians(cameraProjection->fov), cameraProjection->aspectRatio, cameraProjection->nearPlane, cameraProjection->farPlane);
                                        }
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

                                    	if (shaderProgram->shaderProgram != lastShaderProgram) {
                                    		glUseProgram(shaderProgram->shaderProgram);
                                    		lastShaderProgram = shaderProgram->shaderProgram;
                                    	}

                                        auto modelTransform = object->GetTransformMatrix();
                                        if (object->haveParent) {
                                            modelTransform = modelTransform * object->parent->GetTransformMatrix();
                                        }

                                        for(auto & uniformMaker: this->_uniformMakers) {
                                            uniformMaker(object, &camera);
                                        }

                                        Graphics::BindVariable(shaderProgram->uniforms["Model"], modelTransform);

                                        glm::mat4 view = currentCameraMatrix;

                                        if (object->screenObject) {
                                            view = glm::identity<glm::mat4>();
                                        }

                                        if (object->render) {
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
                                                renderFeature->OnUniform(object);
                                            }

                                            for(auto & renderFeature: this->_renderFeatures) {
                                                renderFeature->OnRender(object);
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

                                            if (object->wireFrame) {
												glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                                            } else {
                                                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                                            }

                                            glDrawElements(static_cast<GLenum>(object->renderModel->drawMode), mesh->vertexCount, GL_UNSIGNED_INT, nullptr);
                                        }

                                        object->OnDraw((void*)this);
                                        glBindVertexArray(0);

                                        if (GL_CHECK_ERROR()) {
                                            object->active = false;
                                            DebugLog(LOG_WARNING, "OpenGL error triggered on: " << visualObject.first, false);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    
                    renderImGUI.wait();
                    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                    camera.ResetRender();
                }
            }
        }

        // End the frame even if the window is not rendered
        ImGui::EndFrame();
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
