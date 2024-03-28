#ifndef H_DYNAMIC_OBJECT
#define H_DYNAMIC_OBJECT

#include <PrettyEngine/serial.hpp>
#include <PrettyEngine/tags.hpp>
#include <PrettyEngine/localization.hpp>
#include <PrettyEngine/EngineContent.hpp>
#include <PrettyEngine/PrettyError.hpp>

#include <Guid.hpp>

#include <string>
#include <functional>

namespace PrettyEngine {
	enum class Request {
		SAVE = 0,
		EXIT,
	};

	#define SERIAL_FUNCTION(type, expr) [this](type x){return expr;}
	#define DESERIAL_FUNCTION(expr) [this](std::string x){return expr;}

	#define PUBLIC_INT(name, input_value) \
	PublicProperty<int>(this, "int", name, input_value, [this](const int& value){ return std::to_string(value); }, [this](const std::string& value){ return std::stoi(value); })

	#define PUBLIC_FLOAT(name, input_value) \
	PublicProperty<float>(this, "float", name, input_value, [this](const float& value){ return std::to_string(value); }, [this](const std::string& value){ return std::stof(value); })

	#define PUBLIC_DOUBLE(name, input_value) \
	PublicProperty<double>(this, "double", name, input_value, [this](const double& value){ return std::to_string(value); }, [this](const std::string& value){ return std::stod(value); })

	#define PUBLIC_STRING(name, input_value) \
	PublicProperty<std::string>(this, "std::string", name, input_value, [this](const std::string& value){ return value; }, [this](const std::string& value){ return value; })

	#define PUBLIC_BOOL(name, input_value) \
	PublicProperty<bool>(this, "bool", name, input_value, [this](const bool& value){ return (value ? "true" : "false"); }, [this](const std::string& value){ return (value == "true"); })

 	/// Object that support being updated by the engine based on game events.
	class DynamicObject: public Tagged, public virtual SerialObject {
	public:
		friend class World;
		friend class Editor;
		friend class Entity;

   		DynamicObject() {
   			this->publicFuncions.insert_or_assign("OnSetup", [this]() { this->OnSetup(); });
			this->publicFuncions.insert_or_assign("OnStart", [this]() { this->OnStart(); });
			this->publicFuncions.insert_or_assign("OnUpdate", [this]() { this->OnUpdate(); });
			this->publicFuncions.insert_or_assign("OnRender", [this]() { this->OnRender(); });
   		}

		/// Minimum setup required by a dynamic object
		void SetupDynamicObject(EngineContent* newEngineContent) {
			this->engineContent = newEngineContent;
		}

		/// Create a public var but do not override
		void CreatePublicVar(std::string name, std::string defaultValue = "") {
			if (!this->publicMap.contains(name)) {
				this->publicMap.insert(std::make_pair(name, defaultValue));
				for (auto &action : this->onPublicVariableChanged) {
					(action.second)(name);
				}
			}
		}

		std::string GetPublicVarValue(std::string name) {
			if (this->publicMap.contains(name)) {
				return this->publicMap[name];
			} else {
				return "null";
			}
		}

		void SetPublicVarValue(std::string name, std::string value) {
			if (value != this->GetPublicVarValue(name)) {
				this->publicMap.insert_or_assign(name, value);

				for (auto &action : this->onPublicVariableChanged) {
					(action.second)(name);
				}
			}
		}

		std::vector<float> GetPublicVarAsFloatVec(std::string name) {
			std::vector<float> out;

			for(auto & value: ParseCSVLine(this->GetPublicVarValue(name))) {
				out.push_back(std::stof(value.c_str()));
			}

			return out;
		}

		std::string AddActionOnPublicVariableChanged(std::function<void(std::string)> function, std::string name = xg::newGuid()) { 
			if (function == nullptr) {
				DebugLog(LOG_ERROR, "Tried to add a null function in " << this->serialObjectUnique, true);
			} else {
				this->onPublicVariableChanged.insert_or_assign(name, function);
			}
			return name;
		}

		void RemoveActionOnPublicVariableChanged(std::string name) { 
			this->onPublicVariableChanged.erase(name);
		}

		void RemovePublicFunction(std::string& functionName) {
			this->publicFuncions.erase(functionName);
		}

		void RemovePublicFunction(std::string functionName) {
			this->publicFuncions.erase(functionName);
		}

		EngineContent* engineContent = nullptr;

		std::unordered_map<std::string, std::string> publicMap;

		std::vector<Request> requests;

		std::unordered_map<std::string, std::function<void()>> publicFuncions;

		std::unordered_map<std::string, std::function<void(std::string)>> onPublicVariableChanged;

	private:
		void SetupProperties() {
			for(auto & property: this->propertyEventListennerList) {
				property->OnSetupProperties();
			}
		}

		/// Called when the components loaded
		virtual void OnSetup() {}
		/// Called before the first frame.
		virtual void OnStart() {}
		/// Called before the first frame in the editor only.
		virtual void OnEditorStart() {}
		/// Called each frame.
		virtual void OnUpdate() {}
		/// Called each update if the editor its in the editor.
		virtual void OnEditorUpdate() {}
		/// Called each frame without world optimization.
		virtual void OnAlwaysUpdate() {}
		/// The same as OnUpdate but multi-threaded.
		virtual void OnMTUpdate() {}
		/// Called when the update is done to let you to sync what was done in and out the thread, or other stuff.
		virtual void OnEndUpdate() {}
		/// Called when the update is done to let you to sync what was done in and out the thread, or other stuff.
		virtual void OnAlwaysEndUpdate() {}
		/// Called when object destroyed.
		virtual void OnDestroy() {}
		/// Called just before rendering, when the UI is working.
		virtual void OnRender() {}
		/// Called just before updating the physics.
		virtual void OnPrePhysics() {}
	};
};

#endif