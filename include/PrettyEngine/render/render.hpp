#ifndef H_RENDER
#define H_RENDER

#include <PrettyEngine/debug/debug.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/render/mesh.hpp>
#include <PrettyEngine/render/texture.hpp>
#include <PrettyEngine/render/PrettyGL.hpp>
#include <PrettyEngine/render/visualObject.hpp>
#include <PrettyEngine/render/camera.hpp>
#include <PrettyEngine/collider.hpp>
#include <PrettyEngine/render/light.hpp>
#include <PrettyEngine/render/RenderFeature.hpp>
#include <PrettyEngine/assetManager.hpp>

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
		void ShowWindow() const;

		/// Initialize all the OpenGL and GLFW requirements.
		void Setup(glm::vec3 renderCubeScale = glm::vec3(20.0f, 20.0f, 100.0f));

		/// Draw elements.
		void Draw();
		
		/// Show elements to the target window.
		void Show();
		
		/// Render a frame.
		void Render();

		void SetBackgroundColor(float red, float green, float blue, float alpha) {
			glClearColor(red, green, blue, alpha);
		}

		/// Return true if the user try to close the window.
		bool Valid() const {
			return !glfwWindowShouldClose(this->_window);
		}

		void UpdateWindowRatio() {
			int width, height;
	    	glfwGetWindowSize(this->_window, &width, &height);

	    	bool isMinimized = glfwGetWindowAttrib(this->_window, GLFW_ICONIFIED);

	    	if (!isMinimized) {
				this->_windowRatio = static_cast<float>(width / height);
			}
		}

		void RegisterVisualObject(std::string& name, VisualObject* visualObject) {
			this->CreateLayer(visualObject->renderLayer);

			auto& list = this->visualObjects[visualObject->renderLayer];

			visualObject->OnRendererRegister((void*)this);

			list.insert(std::make_pair(name, visualObject));
		}

		bool UnRegisterVisualObject(std::string& name) {
			auto& list = this->visualObjects;

			for (auto & sublist: list) {
				for(auto & element: sublist) {
					if (element.first == name) {
						element.second->OnRendererUnRegister((void*)this);
						sublist.erase(name);
						return true;
					}
				}
			}

			return false;
		}
		
		void SetVisualObjectLayer(std::string name, VisualObject* visualObject) {
			this->UnRegisterVisualObject(name);
			this->RegisterVisualObject(name, visualObject);
		}

		/// Compile and register a shader from source.
		std::string AddShader(const std::string& name, ShaderType shaderType, const char* shader) {
			if (!this->glShaders.contains(name)) {
				if (GL_CHECK_ERROR()) {
					DebugLog(LOG_ERROR, "OpengGL error before compiling: " << name, true);
				}
				unsigned int glShader = glCreateShader(static_cast<GLenum>(shaderType));
			
				glShaderSource(glShader, 1, &shader, nullptr);
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

				if (GL_CHECK_ERROR()) {
					DebugLog(LOG_ERROR, "OpengGL error after compiling: " << name, true);
				}
			}

			return name;
		}

		GLShaderProgramRefs* AddShaderProgram(
			std::string name,
			std::string vertexShaderName,
			std::string fragmentShaderName,
			std::vector<std::string> otherShaders = {}
		);

		void RemoveShaderProgram(const std::string& name) {
			glDeleteProgram(this->glShaderPrograms[name].shaderProgram);
			this->glShaderPrograms.erase(name);
		}

		Mesh* AddMesh(
			std::string& name,
			Mesh mesh,
			MeshDrawType meshDrawType = MeshDrawType::Static
		);

		void UpdateMesh(Mesh* mesh);

		void RemoveMesh(const std::string& name) {
			auto &mesh = this->glMeshList[name];

			mesh.Cleanup();

			this->glMeshList.erase(name);		
		}

		Texture* AddTexture(
			std::string name,
			Asset* asset,
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

		void RemoveTexture(Texture* texture) {
			glDeleteTextures(1, &texture->textureID);

			this->glTextures.erase(texture->name);
		}

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

		unsigned int GetShaderProgramID(std::string& name) {
			return this->glShaderPrograms[name].shaderProgram;
		}

		void Clear(bool clearEverything = false) {
			for (auto & element: this->glTextures) {
				auto texture = &element.second;
				if (texture != nullptr && (texture->userCount <= 0 && texture->useGC || clearEverything)) {
					this->RemoveTexture(texture->name);
					return;
				}
			}

			for (auto & element: this->glMeshList) {
				auto mesh = &element.second;
				if (mesh->userCount <= 0 && mesh->useGC || clearEverything) {
					this->RemoveMesh(element.first);
					return;
				}
			}

			for (auto shaderProgram : this->glShaderPrograms) {
				if (shaderProgram.second.userCount <= 0 && shaderProgram.second.useGC || clearEverything) {
					this->RemoveShaderProgram(shaderProgram.first);
					return;
				}
			}
		}

		std::pair<bool, Texture*> TextureExist(std::string& name) {
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

		void RemoveCamera(const Camera* camera) {
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

		void SetWindowTitle(const std::string& title) const {
			glfwSetWindowTitle(this->_window, title.c_str());
		}

		void SetFullscreen(bool fullScreen) {
			this->fullscreen = fullScreen;

			GLFWmonitor* monitor = glfwGetPrimaryMonitor();

	        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	        
	        if (fullScreen) {
        		glfwSetWindowMonitor(this->_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	        } else {
        		glfwSetWindowMonitor(this->_window, nullptr, 100, 100, 1900 / 3, 1080 / 3, mode->refreshRate);
			}
		}

		void SetWindowOpacity(const float& opacity) const {
			glfwSetWindowOpacity(this->_window, opacity);
		}

		bool GetFullscreen() const {
			return this->fullscreen;
		}

		void RegisterLight(std::string& name, Light* light) {
			light->name = name;
			this->lights.push_back(light);
		}

		void UnRegisterLight(const Light* light) {
			for (size_t i = 0; i < this->lights.size(); i++) {
				if (this->lights[i]->name == light->name) {
					this->lights.erase(this->lights.begin() + i);		
				}
			}
		}

		GLFWwindow* GetWindow() const {
			return this->_window;
		}

		/// Activate features to get a better 3D rendering, but will break 2D rendering features
		void SwitchTo3D() const {
			glEnable(GL_DEPTH_TEST);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		void SwitchTo2D() const {
			glDisable(GL_DEPTH_TEST);
		}

		double GetTime() const {
			return glfwGetTime();
		}

		void HideWindow() const {
			glfwHideWindow(this->_window);
		}

		void UpdateIO();
		void StartUIRendering();

		unsigned int GetVisualObjectsCount() const {
			unsigned int out = 0;

			for (auto & layer: this->visualObjects) {
				out += layer.size();
			}
			
			return out;
		}

		unsigned int GetLightCount() const {
			return this->lights.size();
		}

		void DrawLine(glm::vec3 start, glm::vec3 end, glm::vec4 color, float width = 1.0f);

		int GetFPS() {
			return static_cast<int>(glm::floor(1.0f / this->GetDeltaTime()));
		}

		bool WindowActive() const {
			bool isMinimized = glfwGetWindowAttrib(this->_window, GLFW_ICONIFIED);
        	bool isFocused = glfwGetWindowAttrib(this->_window, GLFW_FOCUSED);
        	return isMinimized || isFocused;
		}

		/// Must be called before the window creation
		void SetAntiAliasing(int value) {
			this->antialiasing = value;
		}
		
		int GetAntiAliasing() const {
			return this->antialiasing;
		}

		void ResetWindowIcon() const {
			glfwSetWindowIcon(this->_window, 0, nullptr);
		}
		
		void SetWindowIcon(unsigned char* data, int width, int height) {
			this->_glfwIcons.clear();
			GLFWimage image {};
			image.pixels = data;
			image.width = width;
			image.height = height;

			this->_glfwIcons.push_back(image);

			glfwSetWindowIcon(this->_window, this->_glfwIcons.size(), this->_glfwIcons.data());
		}

		bool GetWindowFocus() const {
			return glfwGetWindowAttrib(this->_window, GLFW_FOCUSED);
		}

		bool GetWindowMinimized() const {
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

		int GetTargetFrameRate() const {
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
		std::vector<std::unordered_map<std::string, VisualObject*>> visualObjects;

		std::vector<Light*> lights;

		float sunLightFactor = 1.0f;
		glm::vec3 sunColor = glm::vec3(1.0f, 1.0f, 1.0f);

  		/// Store the VAO and VBO of each mesh
		std::unordered_map<std::string, Mesh> glMeshList;

		std::unordered_map<std::string, Texture> glTextures;

	private:
		std::vector<std::shared_ptr<RenderFeature>> _renderFeatures;

		unsigned int mainLayer = 0;

		std::vector<unsigned int> hiddenLayers;

		/// Store the index of each shaders
		std::unordered_map<std::string, unsigned int> glShaders;

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