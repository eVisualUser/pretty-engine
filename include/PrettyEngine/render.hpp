#ifndef H_RENDER
#define H_RENDER

#include "PrettyEngine/debug.hpp"
#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/mesh.hpp>
#include <PrettyEngine/texture.hpp>
#include <PrettyEngine/transform.hpp>
#include <PrettyEngine/PrettyGL.hpp>
#include <PrettyEngine/visualObject.hpp>
#include <PrettyEngine/camera.hpp>
#include <PrettyEngine/collider.hpp>
#include <PrettyEngine/light.hpp>
#include <PrettyEngine/RenderFeature.hpp>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <imgui.h>

#include <memory>
#include <utility>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>

namespace PrettyEngine {
	#define CHECK_OPENGL_ERROR() std::cout << "OpenGL Error: " << glGetError() << std::endl;

	std::vector<unsigned char> DecodeImage(std::vector<unsigned char>* in, int bytes, int* height, int* width, int* channels);
	
	typedef void (*UniformMaker)(VisualObject*, Camera*);

	class Renderer {
	public:
		Renderer();
		~Renderer();

		/// Create a window and hide it by default.
		void CreateWindow();
		void ShowWindow();

		/// Initialize all the OpenGL and GLFW requirements.
		void Setup(glm::vec3 renderCubeScale = glm::vec3(20.0f, 20.0f, 100.0f));

		/// Draw elements.
		void Draw();
		
		/// Show elements to the target window.
		void Show();
		
		/// Render a frame.
		void Render();

		void SetBackgroundColor(float r, float g, float b, float a) {
			glClearColor(r, g, b, a);
		}

		/// Return true if the user try to close the window.
		bool Valid() {
			return !glfwWindowShouldClose(this->_window);
		}

		void UpdateWindowRatio() {
			int width, height;
	    	glfwGetWindowSize(this->_window, &width, &height);

	    	bool isMinimized = glfwGetWindowAttrib(this->_window, GLFW_ICONIFIED);

	    	if (!isMinimized) {
				this->_windowRatio = (float) width / height;
			}
		}

		void RegisterVisualObject(std::string name, std::shared_ptr<VisualObject> visualObject) {
			this->CreateLayer(visualObject->renderLayer);

			auto& list = this->visualObjects[visualObject->renderLayer];

			visualObject->OnRendererRegister((void*)this);

			list.insert(std::make_pair(name, visualObject));
		}

		void UnRegisterVisualObject(std::string name) {
			auto& list = this->visualObjects;

			for (auto & sublist: list) {
				for(auto & element: sublist) {
					if (element.first == name) {
						element.second->OnRendererUnRegister((void*)this);
						sublist.erase(name);
					}
				}
			}
		}
		
		void SetVisualObjectLayer(std::string name, std::shared_ptr<VisualObject> visualObject) {
			this->UnRegisterVisualObject(name);
			this->RegisterVisualObject(name, visualObject);
		}

		/// Compile and register a shader from source.
		void AddShader(std::string name, ShaderType shaderType, const char* shader) {
			unsigned int glShader = glCreateShader(static_cast<GLenum>(shaderType));
			
			glShaderSource(glShader, 1, &shader, NULL);
	    	glCompileShader(glShader);

	    	#if _DEBUG
	    	int status;
		    glGetShaderiv(glShader, GL_COMPILE_STATUS, &status);

		    if(!status) {
		        char buffer[512];
		        glGetShaderInfoLog(glShader, 512, NULL, buffer);

		        std::cerr << buffer << std::endl;
		    }
	    	#endif

	    	this->glShaders.insert(std::make_pair(name, glShader));
		}

		GLShaderProgramRefs* AddShaderProgram(
			std::string name,
			std::string vertexShaderName,
			std::string fragmentShaderName,
			std::vector<std::string> otherShaders = {}
		);

		void RemoveShaderProgram(std::string name) {
			glDeleteProgram(this->glShaderPrograms[name].shaderProgram);
			this->glShaderPrograms.erase(name);
		}

		Mesh* AddMesh(
			std::string name,
			Mesh mesh,
			MeshDrawType meshDrawType = MeshDrawType::Static
		);

		void UpdateMesh(Mesh* mesh);

		void RemoveMesh(std::string name) {
			auto mesh = this->glMeshList[name];

			mesh.Cleanup();

			this->glMeshList.erase(name);		
		}

		Texture* AddTexture(
			std::string name,
			std::string fileName,
			TextureType textureType,
			TextureWrap wrap,
			TextureFilter filter,
			TextureChannels channels 
		);

		Texture* AddTextureFromData(
			std::string name,
			unsigned char* data,
			int width,
	    	int height,
	    	TextureType textureType,
			TextureWrap wrap,
			TextureFilter filter,
			TextureChannels channels = TextureChannels::RGBA
		);

		void RemoveTexture(std::string name) {
			glDeleteTextures(1, &this->glTextures[name].textureID);

			this->glTextures.erase(name);
		}

		void HideLayer(unsigned int layer) {
			this->hiddenLayers.push_back(layer);
		}

		void ShowLayer(unsigned int layer) {
			this->hiddenLayers.erase(
				std::remove(this->hiddenLayers.begin(), this->hiddenLayers.end(), layer), this->hiddenLayers.end()
			);
		}

		void SetMainLayer(unsigned int layer) {
			this->mainLayer = layer;
		}

		void CreateLayer(unsigned int layer) {
			while (this->visualObjects.size() <= layer) {
				this->visualObjects.push_back({});
			}
		}

		unsigned int GetShaderProgramID(std::string name) {
			return this->glShaderPrograms[name].shaderProgram;
		}

		void Clear() {
			for (auto & element: this->glTextures) {
				auto texture = &element.second;
				if (texture->userCount <= 0 && texture->useGC) {
     				DebugLog(LOG_WARNING, "Unused texture: " << texture->name, false);
					this->RemoveTexture(texture->name);
					this->Clear();
					return;
				}
			}

			for (auto & element: this->glMeshList) {
				auto mesh = &element.second;
				if (mesh->userCount <= 0 && mesh->useGC) {
     				DebugLog(LOG_WARNING, "Unused mesh: " << mesh->name, false);
					this->RemoveMesh(element.first);
					this->Clear();
					return;
				}
			}
		}

		std::pair<bool, Texture*> TextureExist(std::string name) {
			for (auto & texture: this->glTextures) {
				if (texture.second.name == name) {
					return std::make_pair(true, &texture.second);
				}
 			}
 			return std::make_pair(false, nullptr);
		}

		double GetDeltaTime() {
			return this->deltaTime * this->GetTimeSpeed();
		}
		
		void SetTimeSpeed(double newTimeSpeed) {
			this->timeSpeed = newTimeSpeed;
		}

		double GetTimeSpeed() {
			return this->timeSpeed;
		}

		Camera* AddCamera() {
			this->cameraList.push_back(Camera());
			return &this->cameraList.back();
		}

		void RemoveCamera(Camera* camera) {
			for (size_t i = 0; i < this->cameraList.size(); i++) {
				if (this->cameraList[i].id == camera->id) {
					this->cameraList.erase(this->cameraList.begin() + i);
					return;
				}
			}
		}

		Camera* GetCurrentCamera() {
			for (auto & cam: this->cameraList) {
				if (cam.active)
					return &cam;
			}
			return nullptr;
		}

		void SetWindowTitle(std::string title) {
			glfwSetWindowTitle(this->_window, title.c_str());
		}

		void SetFullscreen(bool fullScreen) {
			this->fullscreen = fullScreen;

			GLFWmonitor* monitor = glfwGetPrimaryMonitor();

	        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	        
	        if (fullScreen) {
        		glfwSetWindowMonitor(this->_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	        } else {
        		glfwSetWindowMonitor(this->_window, NULL, 100, 100, 1900 / 3, 1080 / 3, mode->refreshRate);
			}
		}

		void SetWindowOpacity(float opacity) {
			glfwSetWindowOpacity(this->_window, opacity);
		}

		bool GetFullscreen() {
			return this->fullscreen;
		}

		void RegisterLight(std::string name, Light* light) {
			light->name = name;
			this->lights.push_back(light);
		}

		void UnRegisterLight(Light* light) {
			for (size_t i; i < this->lights.size(); i++) {
				if (this->lights[i]->name == light->name) {
					this->lights.erase(this->lights.begin() + i);		
				}
			}
		}

		GLFWwindow* GetWindow() {
			return this->_window;
		}

		/// Activate features to get a better 3D rendering, but will break 2D rendering features
		void SwitchTo3D() {
			glEnable(GL_DEPTH_TEST);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		void SwitchTo2D() {
			glDisable(GL_DEPTH_TEST);
		}

		double GetTime() {
			return glfwGetTime();
		}

		void HideWindow() {
			glfwHideWindow(this->_window);
		}

		void UpdateIO();
		void StartUIRendering();

		unsigned int GetVisualObjectsCount() {
			unsigned int out = 0;

			for (auto & layer: this->visualObjects) {
				for (auto & obj: layer) {
					out++;
				}
			}
			
			return out;
		}

		unsigned int GetLightCount() {
			return this->lights.size();
		}

		void DrawLine(glm::vec3 start, glm::vec3 end, glm::vec4 color, float width = 1.0f);

		int GetFPS() {
			return (int)glm::floor(1.0f / this->GetDeltaTime());
		}

		bool WindowActive() {
			bool isMinimized = glfwGetWindowAttrib(this->_window, GLFW_ICONIFIED);
        	bool isFocused = glfwGetWindowAttrib(this->_window, GLFW_FOCUSED);
        	return isMinimized || isFocused;
		}

		/// Must be called before the window creation
		void SetAntiAliasing(int value) {
			this->antialiasing = value;
		}
		
		int GetAntiAliasing() {
			return this->antialiasing;
		}

		void ResetWindowIcon() {
			glfwSetWindowIcon(this->_window, 0, NULL);
		}
		
		void SetWindowIcon(unsigned char* data, int width, int height) {
			this->_glfwIcons.clear();
			GLFWimage image;
			image.pixels = data;
			image.width = width;
			image.height = height;

			this->_glfwIcons.push_back(image);

			glfwSetWindowIcon(this->_window, this->_glfwIcons.size(), this->_glfwIcons.data());
		}

		bool GetWindowFocus() {
			return glfwGetWindowAttrib(this->_window, GLFW_FOCUSED);
		}

		bool GetWindowMinimized() {
        	return glfwGetWindowAttrib(this->_window, GLFW_ICONIFIED);
		}

		void AddUniformMake(UniformMaker uniformMaker) {
			this->_uniformMakers.push_back(uniformMaker);
		}

		/// Add a render feature, a feature must be never deleted.
		void AddRenderFeature(std::shared_ptr<RenderFeature> renderFeature) {
			this->_renderFeatures.push_back(renderFeature);
		}

	private:
		std::vector<GLFWimage> _glfwIcons;

		void SetFrameRate(int frameRate) {
			this->_targetFrameRate = frameRate;
		}

		int GetTargetFrameRate() {
			return this->_targetFrameRate;
		}

	private:
		GLFWwindow* _window;
		double _lastFrame = glfwGetTime();
		float _windowRatio = 0.0f;

	public:
		/// Contain all id's to the shader programs based on their name
		std::unordered_map<std::string, GLShaderProgramRefs> glShaderPrograms;

		std::vector<Camera> cameraList;

		/// Contain all the visual objects
		std::vector<std::unordered_map<std::string, std::shared_ptr<VisualObject>>> visualObjects;

		std::vector<Light*> lights;

		float sunLightFactor = 1.0f;
		glm::vec3 sunColor = glm::vec3(1.0f, 1.0f, 1.0f);

	private:
		std::vector<std::shared_ptr<RenderFeature>> _renderFeatures;

		unsigned int mainLayer = 0;

		std::vector<unsigned int> hiddenLayers;

		/// Store the index of each shaders
		std::unordered_map<std::string, unsigned int> glShaders;

		/// Store the VAO and VBO of each mesh
		std::unordered_map<std::string, Mesh> glMeshList;

		std::unordered_map<std::string, Texture> glTextures;

		Collider renderCube = Collider();

		bool fullscreen = false;

		double deltaTime = 0.0f;
		double timeSpeed = 1.0f;

		unsigned int frameBuffer = 0; 
		unsigned int frameBufferColorTexture = 0;

		std::vector<UniformMaker> _uniformMakers;

		unsigned int _textVAO, _textVBO;

		ImGuiContext* imGUIContext;
		ImGuiIO* imGUIIO;

		int antialiasing = 16;

		int _targetFrameRate = 0;
	};
}

#endif