#pragma once

#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/utils.hpp>
#include <PrettyEngine/mesh.hpp>
#include <PrettyEngine/texture.hpp>
#include <PrettyEngine/transform.hpp>
#include <PrettyEngine/gl.hpp>
#include <PrettyEngine/visualObject.hpp>
#include <PrettyEngine/camera.hpp>
#include <PrettyEngine/physics.hpp>
#include <PrettyEngine/light.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <imgui.h>

#include <memory>
#include <utility>
#include <vcruntime.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <thread>

namespace PrettyEngine {

	class InputHandler {
	public:
		virtual void OnWindowPolls(void* renderer) { /* Do nothing */ }
	};

	void MouseWheelScrollCallBack(GLFWwindow* window, double x, double y);

	enum class KeyCode {
	    Unknown = GLFW_KEY_UNKNOWN,
	    Space = GLFW_KEY_SPACE,
	    Apostrophe = GLFW_KEY_APOSTROPHE,
	    Comma = GLFW_KEY_COMMA,
	    Minus = GLFW_KEY_MINUS,
	    Period = GLFW_KEY_PERIOD,
	    Slash = GLFW_KEY_SLASH,
	    Num0 = GLFW_KEY_0,
	    Num1 = GLFW_KEY_1,
	    Num2 = GLFW_KEY_2,
	    Num3 = GLFW_KEY_3,
	    Num4 = GLFW_KEY_4,
	    Num5 = GLFW_KEY_5,
	    Num6 = GLFW_KEY_6,
	    Num7 = GLFW_KEY_7,
	    Num8 = GLFW_KEY_8,
	    Num9 = GLFW_KEY_9,
	    Semicolon = GLFW_KEY_SEMICOLON,
	    Equal = GLFW_KEY_EQUAL,
	    A = GLFW_KEY_A,
	    B = GLFW_KEY_B,
	    C = GLFW_KEY_C,
	    D = GLFW_KEY_D,
	    E = GLFW_KEY_E,
	    F = GLFW_KEY_F,
	    G = GLFW_KEY_G,
	    H = GLFW_KEY_H,
	    I = GLFW_KEY_I,
	    J = GLFW_KEY_J,
	    K = GLFW_KEY_K,
	    L = GLFW_KEY_L,
	    M = GLFW_KEY_M,
	    N = GLFW_KEY_N,
	    O = GLFW_KEY_O,
	    P = GLFW_KEY_P,
	    Q = GLFW_KEY_Q,
	    R = GLFW_KEY_R,
	    S = GLFW_KEY_S,
	    T = GLFW_KEY_T,
	    U = GLFW_KEY_U,
	    V = GLFW_KEY_V,
	    W = GLFW_KEY_W,
	    X = GLFW_KEY_X,
	    Y = GLFW_KEY_Y,
	    Z = GLFW_KEY_Z,
	    LeftBracket = GLFW_KEY_LEFT_BRACKET,
	    Backslash = GLFW_KEY_BACKSLASH,
	    RightBracket = GLFW_KEY_RIGHT_BRACKET,
	    GraveAccent = GLFW_KEY_GRAVE_ACCENT,
	    World1 = GLFW_KEY_WORLD_1,
	    World2 = GLFW_KEY_WORLD_2,
	    Escape = GLFW_KEY_ESCAPE,
	    Enter = GLFW_KEY_ENTER,
	    Tab = GLFW_KEY_TAB,
	    Backspace = GLFW_KEY_BACKSPACE,
	    Insert = GLFW_KEY_INSERT,
	    Delete = GLFW_KEY_DELETE,
	    Right = GLFW_KEY_RIGHT,
	    Left = GLFW_KEY_LEFT,
	    Down = GLFW_KEY_DOWN,
	    Up = GLFW_KEY_UP,
	    PageUp = GLFW_KEY_PAGE_UP,
	    PageDown = GLFW_KEY_PAGE_DOWN,
	    Home = GLFW_KEY_HOME,
	    End = GLFW_KEY_END,
	    CapsLock = GLFW_KEY_CAPS_LOCK,
	    ScrollLock = GLFW_KEY_SCROLL_LOCK,
	    NumLock = GLFW_KEY_NUM_LOCK,
	    PrintScreen = GLFW_KEY_PRINT_SCREEN,
	    Pause = GLFW_KEY_PAUSE,
	    F1 = GLFW_KEY_F1,
	    F2 = GLFW_KEY_F2,
	    F3 = GLFW_KEY_F3,
	    F4 = GLFW_KEY_F4,
	    F5 = GLFW_KEY_F5,
	    F6 = GLFW_KEY_F6,
	    F7 = GLFW_KEY_F7,
	    F8 = GLFW_KEY_F8,
	    F9 = GLFW_KEY_F9,
	    F10 = GLFW_KEY_F10,
	    F11 = GLFW_KEY_F11,
	    F12 = GLFW_KEY_F12,
	    F13 = GLFW_KEY_F13,
	    F14 = GLFW_KEY_F14,
	    F15 = GLFW_KEY_F15,
	    F16 = GLFW_KEY_F16,
	    F17 = GLFW_KEY_F17,
	    F18 = GLFW_KEY_F18,
	    F19 = GLFW_KEY_F19,
	    F20 = GLFW_KEY_F20,
	    F21 = GLFW_KEY_F21,
	    F22 = GLFW_KEY_F22,
	    F23 = GLFW_KEY_F23,
	    F24 = GLFW_KEY_F24,
	    F25 = GLFW_KEY_F25,
	    KP0 = GLFW_KEY_KP_0,
	    KP1 = GLFW_KEY_KP_1,
	    KP2 = GLFW_KEY_KP_2,
	    KP3 = GLFW_KEY_KP_3,
	    KP4 = GLFW_KEY_KP_4,
	    KP5 = GLFW_KEY_KP_5,
	    KP6 = GLFW_KEY_KP_6,
	    KP7 = GLFW_KEY_KP_7,
	    KP8 = GLFW_KEY_KP_8,
	    KP9 = GLFW_KEY_KP_9,
	    KPDecimal = GLFW_KEY_KP_DECIMAL,
	    KPDivide = GLFW_KEY_KP_DIVIDE,
	    KPMultiply = GLFW_KEY_KP_MULTIPLY,
	    KPSubtract = GLFW_KEY_KP_SUBTRACT,
	    KPAdd = GLFW_KEY_KP_ADD,
	    KPEnter = GLFW_KEY_KP_ENTER,
	    KPEqual = GLFW_KEY_KP_EQUAL,
	    LeftShift = GLFW_KEY_LEFT_SHIFT,
	    LeftControl = GLFW_KEY_LEFT_CONTROL,
	    LeftAlt = GLFW_KEY_LEFT_ALT,
	    LeftSuper = GLFW_KEY_LEFT_SUPER,
	    RightShift = GLFW_KEY_RIGHT_SHIFT,
	    RightControl = GLFW_KEY_RIGHT_CONTROL,
	    RightAlt = GLFW_KEY_RIGHT_ALT,
	    RightSuper = GLFW_KEY_RIGHT_SUPER,
	    Menu = GLFW_KEY_MENU
	};

	enum class CursorState {
		Hidden = GLFW_CURSOR_HIDDEN,
		Locked = GLFW_CURSOR_DISABLED,
		Normal = GLFW_CURSOR_NORMAL,
	};

	#define CHECK_OPENGL_ERROR() std::cout << "OpenGL Error: " << glGetError() << std::endl;

	std::vector<unsigned char> DecodeImage(std::vector<unsigned char>* in, int bytes, int* height, int* width, int* channels);
	
	class Renderer {
	public:
		Renderer();
		~Renderer();

		/// Create a window and hide it by default
		void CreateWindow();
		void ShowWindow();

		void Setup(glm::vec3 renderCubeScale = glm::vec3(20.0f, 20.0f, 100.0f));

		/// Draw elements
		void Draw();
		
		/// Show elements to the target window
		void Show();
		
		void Render();

		void SetBackgroundColor(float r, float g, float b, float a) {
			glClearColor(r, g, b, a);
		}

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

		void RegisterInputHandler(std::string name, std::shared_ptr<InputHandler> inputHandler) {
			auto& list = this->_inputHandlerList;

			list.insert(std::make_pair(name, inputHandler));
		}
		
		void UnRegisterInputHandler(std::string name, unsigned int layer) {
			auto& list = this->_inputHandlerList;

			list.erase(name);
		}
		
		void SetVisualObjectLayer(std::string name, std::shared_ptr<VisualObject> visualObject) {
			this->UnRegisterVisualObject(name);
			this->RegisterVisualObject(name, visualObject);
		}

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
			TextureChannels channels = TextureChannels::RGBA 
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
					this->RemoveTexture(texture->name);
					this->Clear();
					return;
				}
			}

			for (auto & element: this->glMeshList) {
				auto mesh = &element.second;
				if (mesh->userCount <= 0 && mesh->useGC) {
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

		Font LoadFont(std::string name, std::string fileName, TextureFilter filter = TextureFilter::Linear, float size = 48.0f);

		bool GetKeyPress(KeyCode key) {
			return GLFW_PRESS == glfwGetKey(this->_window, (int)key);
		}

		bool GetKeyDown(KeyCode key) {
		    static std::unordered_map<int, bool> keyStates;
		    int state = glfwGetKey(this->_window, (int)key);
		    bool isKeyDown = (state == GLFW_PRESS) && !keyStates[(int)key];
		    keyStates[(int)key] = (state == GLFW_PRESS);

		    return isKeyDown;
		}

		bool GetKeyUp(KeyCode key) {
			return GLFW_RELEASE == glfwGetKey(this->_window, (int)key);
		}

		bool GetMouseButton(int button = 0) {
			int state = glfwGetMouseButton(this->_window, button);
			if (state == GLFW_PRESS) {
				return true;
			}
			return false;
		}

		bool GetMouseButtonClick(int button = 0) {
			static std::unordered_map<int, bool> keyStates;
		    int state = glfwGetMouseButton(this->_window, button);
		    bool isKeyDown = (state == GLFW_PRESS) && !keyStates[button];
		    keyStates[button] = (state == GLFW_PRESS);

		    return isKeyDown;
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

		glm::vec2 GetCursorPosition() {
			double x, y;
			glfwGetCursorPos(this->_window, &x, &y);

			auto out = glm::vec2(x, y);
			return out;
		}

		glm::vec2 GetCursorDelta() {
			return this->_cursorLastPosition - this->GetCursorPosition();
		}

		float GetMouseWheelDelta();

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
				if (cam.mainCamera)
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

	        glfwSetScrollCallback(this->_window, &MouseWheelScrollCallBack);
	        
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

		void SetCursorState(CursorState state) {
			glfwSetInputMode(this->_window, GLFW_CURSOR, (int)state);
		}

		std::pair<std::vector<Texture>, std::vector<Mesh>> LoadModel(std::string filePath, std::string id) {
			Assimp::Importer importer;

			unsigned int flags = aiProcess_Triangulate         // Triangulate faces if not already done
                       | aiProcess_JoinIdenticalVertices // Join identical vertices
                       | aiProcess_GenSmoothNormals     // Generate smooth normals if no normals are present
                       | aiProcess_FixInfacingNormals   // Fix normals facing inward
                       | aiProcess_FindInvalidData      // Find and remove invalid data
                       | aiProcess_ValidateDataStructure; // Validate the imported data structure

			const aiScene* scene = importer.ReadFile(filePath, flags);
			
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        		DebugLog(LOG_ERROR, "Error loading model: " << importer.GetErrorString(), true);
        		std::exit(-1);
        	}

        	std::pair<std::vector<Texture>, std::vector<Mesh>> buffer;
        	this->AIProcessNode(scene->mRootNode, scene, &buffer);
		
        	return buffer;
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
		
	private:
		std::vector<GLFWimage> _glfwIcons;

		std::pair<Texture, Mesh> AIProcessMesh(aiMesh* mesh, const aiScene* scene) {
			std::pair<Texture, Mesh> out;

		    if (mesh->HasFaces()) {
				for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
					aiFace face = mesh->mFaces[i];
			    	for (unsigned int j = 0; j < face.mNumIndices; j++) {
			        	out.second.indices.push_back(face.mIndices[j]);
			    	}
				}
			}
			
		    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		        Vertex vertex;
		        
		        // Process position
		        if (mesh->HasPositions()) {
		        	vertex.position.x = mesh->mVertices[i].x;
		        	vertex.position.y = mesh->mVertices[i].y;
		        	vertex.position.z = mesh->mVertices[i].z;
		    	}

		        if (mesh->HasVertexColors(i)) {
		        	vertex.color.r = mesh->mColors[i]->r;
		        	vertex.color.g = mesh->mColors[i]->g;
		        	vertex.color.b = mesh->mColors[i]->b;
		        	vertex.color.a = mesh->mColors[i]->a;
		        }

		        // Process texture coordinates (if available)
		        if (mesh->HasTextureCoords(0)) {
		            vertex.textureCoord.x = mesh->mTextureCoords[0][i].x;
		            vertex.textureCoord.y = mesh->mTextureCoords[0][i].y;
		        } else {
		            vertex.textureCoord = glm::vec2(0.0f, 0.0f);
		        }

		        out.second.vertices.push_back(vertex);
		    }

		    out.second.vertexCount = mesh->mNumVertices;

		    auto material = scene->mMaterials[mesh->mMaterialIndex];
		    if (material && mesh->mMaterialIndex < scene->mNumMaterials) {
		    	aiString path;
		    	aiReturn out = material->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &path);

		    	if (out == AI_SUCCESS) {
		    		// Todo
		    	}
		    }

		    return out;
		}

		void AIProcessNode(aiNode* node, const aiScene* scene, std::pair<std::vector<Texture>, std::vector<Mesh>>* buffer) {
	    	// Process all the meshes in the current node
	    	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
	        	aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
	        	buffer->second.push_back(AIProcessMesh(mesh, scene).second);
	    	}

	    	// Process all the children nodes recursively
	    	for (unsigned int i = 0; i < node->mNumChildren; i++) {
	        	AIProcessNode(node->mChildren[i], scene, buffer);
	    	}
		}

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
		std::unordered_map<std::string, std::shared_ptr<InputHandler>> _inputHandlerList;

		std::vector<Light*> lights;

		float sunLightFactor = 1.0f;
		glm::vec3 sunColor = glm::vec3(1.0f, 1.0f, 1.0f);

	private:
		unsigned int mainLayer = 0;

		std::vector<unsigned int> hiddenLayers;

		/// Store the index of each shaders
		std::unordered_map<std::string, unsigned int> glShaders;

		/// Store the VAO and VBO of each mesh
		std::unordered_map<std::string, Mesh> glMeshList;

		std::unordered_map<std::string, Texture> glTextures;

		glm::vec2 _cursorLastPosition = glm::vec2();

		Collider renderCube = Collider();

		bool fullscreen = false;

		double deltaTime = 0.0f;
		double timeSpeed = 1.0f;

		unsigned int frameBuffer = 0; 
		unsigned int frameBufferColorTexture = 0;
	private:
		std::thread flatenThread;

		unsigned int _textVAO, _textVBO;

		ImGuiContext* imGUIContext;
		ImGuiIO* imGUIIO;

		int antialiasing = 16;

		int _targetFrameRate = 0;
	};
}
