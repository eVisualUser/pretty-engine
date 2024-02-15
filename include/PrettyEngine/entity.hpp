#ifndef H_ENTITY
#define H_ENTITY

#include <PrettyEngine/dynamicObject.hpp>
#include <PrettyEngine/PrettyError.hpp>
#include <PrettyEngine/audio.hpp>
#include <PrettyEngine/render/render.hpp>
#include <PrettyEngine/render/visualObject.hpp>
#include <PrettyEngine/collider.hpp>
#include <PrettyEngine/transform.hpp>
#include <PrettyEngine/tags.hpp>

#include <Guid.hpp>

#include <memory>

namespace PrettyEngine {
	class World;

	#define DEFAULT_ENTITY_NAME "AnyEntity"

	class Component: public DynamicObject {
	public:
		~Component() {
			this->OnDestroy();
		}

		Transform* GetTransform() const {
			return dynamic_cast<Transform*>(this->owner);
		}

		void SetupComponent(DynamicObject* newOwner) {
			this->owner = newOwner; 
		}

	public:
   		/// The entity that own the component.
		DynamicObject* owner;

  		/// True if start was never called.
		bool worldFirst = true;
	};

 	/// An object that is part from a world and can contain components.
	class Entity: public virtual DynamicObject, public virtual Transform {
	public:
		~Entity() {
			this->OnDestroy();
			this->publicMap.clear();
			this->components.clear();
		}

		std::string GetGUID() {
			return this->_entityGUID;
		}
	public:

  		/// True if start was never called.
		bool worldFirst = true;

		std::string entityName = DEFAULT_ENTITY_NAME;

	public:
		template<typename T>
		T* AddComponent(std::string name) {
			auto newComponent = std::make_shared<T>();

			newComponent->SetupSerial(name, xg::newGuid());
			newComponent->SetupDynamicObject(this->engineContent);
			newComponent->SetupComponent(this);

			this->components.push_back(newComponent);

			return dynamic_cast<T*>(this->components.back().get());
		}

		void RemoveComponent(Component* component) {
			for(int i = 0; i < this->components.size(); i++) {
				if (this->components[i]->serialObjectUnique == component->GetObjectSerializedUnique()) {
					this->components.erase(this->components.begin() + i);
					break;
				}
			}
		}

		template<typename T>
		Error<T*> GetComponentAs(std::string unique) {
			for(auto & component: this->components) {
				if (component->serialObjectUnique == unique) {
					return Error<T*>("Component found", false, dynamic_cast<T*>(component.get()));
				}
			}

			return Error<T*>("Component not found", true, nullptr);
		}

		std::vector<std::shared_ptr<Component>> components;
	private:
		std::string _entityGUID = xg::newGuid();
	
	public:
		World *world;
	};
};

#endif