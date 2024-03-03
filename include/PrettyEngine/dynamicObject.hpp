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

	template<typename T>
	class PublicProperty {
	public:
		PublicProperty(SerialObject* newContainer, std::string newName, T newValue, std::function<std::string(T)> newSerializeFunction, std::function<T(std::string)> newDeserializeFunction) {
			this->Init(newContainer, newName, newValue, newSerializeFunction, newDeserializeFunction);
		}

		void Init(SerialObject* newContainer, std::string newName, T newValue, std::function<std::string(T)> newSerializeFunction, std::function<T(std::string)> newDeserializeFunction) {
			this->_name = newName;

			this->_value = newValue;

			this->_serializeFunction = newSerializeFunction;
			this->_deserializeFunction = newDeserializeFunction;

			this->_container = newContainer;

			this->_container.HaveValue([this](SerialObject* value) {
				value->AddSerializedField(typeid(T).name(), this->_name, (this->_serializeFunction)(this->_value));
			});
			this->UpdateValue();
		}

		void UpdateValue() {
			this->_container.HaveValue([this](SerialObject* value) {
				this->_value = this->_deserializeFunction(value->GetSerializedFieldValue(this->_name));
			});
		}

		void Save() {
			this->_container.HaveValue([this](SerialObject* value) {
				value->GetSerializedField(this->_name)->value = (this->_serializeFunction)(this->_value);
			});
		}

		T* Get() {
			return &this->_value;
		}

	private:
		std::function<std::string(T)> _serializeFunction;
		std::function<T(std::string)> _deserializeFunction;

		Option<SerialObject*> _container = Option<SerialObject*>(nullptr);

		std::string _name;

		T _value;
	};

 	/// Object that support being updated by the engine based on game events.
	class DynamicObject: public Tagged, public virtual SerialObject {
	public:
   		DynamicObject() {
   			this->publicFuncions.insert_or_assign("OnSetup", [this]() { this->OnSetup(); });
			this->publicFuncions.insert_or_assign("OnStart", [this]() { this->OnStart(); });
			this->publicFuncions.insert_or_assign("OnUpdate", [this]() { this->OnUpdate(); });
			this->publicFuncions.insert_or_assign("OnRender", [this]() { this->OnRender(); });
  		}

		~DynamicObject() { this->DynamicObject::OnDestroy(); }

		/// Minimum setup required by a dynamic object
		void SetupDynamicObject(EngineContent* newEngineContent) {
			this->engineContent = newEngineContent;
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

		EngineContent* engineContent;

		std::unordered_map<std::string, std::string> publicMap;

		std::vector<Request> requests;

  		std::unordered_map<std::string, std::function<void()>> publicFuncions;

	private:
		std::unordered_map<std::string, std::function<void(std::string)>> onPublicVariableChanged;
	};
}

#endif