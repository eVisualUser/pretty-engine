#ifndef H_DYNAMIC_OBJECT
#define H_DYNAMIC_OBJECT

#include <PrettyEngine/audio.hpp>
#include <PrettyEngine/render.hpp>
#include <PrettyEngine/serial.hpp>
#include <PrettyEngine/tags.hpp>
#include <PrettyEngine/Input.hpp>
#include <PrettyEngine/localization.hpp>
#include <PrettyEngine/PhysicalSpace.hpp>
#include <PrettyEngine/event.hpp>
#include <PrettyEngine/EngineContent.hpp>

#include <Guid.hpp>

#include <string>

namespace PrettyEngine {
	enum class Request {
		SAVE = 0,
		EXIT,
	};

 	/// Object dynamicly managed in the engine.
	class DynamicObject: public Tagged, public virtual SerialObject {
	public:
   		DynamicObject() {
   			this->publicFuncions.insert_or_assign("OnUpdatePublicVariables", [this]() { this->OnUpdatePublicVariables(); });
			this->publicFuncions.insert_or_assign("OnStart", [this]() { this->OnStart(); });
			this->publicFuncions.insert_or_assign("OnUpdate", [this]() { this->OnUpdate(); });
			this->publicFuncions.insert_or_assign("OnRender", [this]() { this->OnRender(); });
  		}

		/// Called when the components loaded
		virtual void OnUpdatePublicVariables() {}
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
				DebugLog(LOG_ERROR, "Tried to add a null function in " << this->unique, true);
			} else {
				this->onPublicVariableChanged.insert_or_assign(name, function);
			}
			return name;
		}

		void RemoveActionOnPublicVariableChanged(std::string name) { 
			this->onPublicVariableChanged.erase(name);
		}
		
	public:
		EngineContent* engineContent;

		std::unordered_map<std::string, std::string> publicMap;

		std::vector<Request> requests;

  		std::unordered_map<std::string, std::function<void()>> publicFuncions;

	public:
		std::string unique;
		std::string object;

	private:
		std::unordered_map<std::string, std::function<void(std::string)>> onPublicVariableChanged;
	};
}

#endif