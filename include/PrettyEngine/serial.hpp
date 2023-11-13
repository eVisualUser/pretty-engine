#pragma once

#include <PrettyEngine/debug.hpp>

#include <sstream>
#include <toml++/toml.h>
#include <Guid.hpp>

#include <string>
#include <unordered_map>

namespace PrettyEngine {
	#define REFLECT_OUT(variable, toStringFunction) this->AddSerializedField(#variable, toStringFunction(variable));
	#define REFLECT_OUT_EXTERN(variable, toStringFunction) AddSerializedField(#variable, toStringFunction(variable));

	#define REFLECT_IN(variable, fromStringFunction) fromStringFunction(this->GetSerializedField(#variable));
	#define REFLECT_IN_EXTERN(self, variable, fromStringFunction) fromStringFunction(self.GetSerializedField(#variable));

	#define SERIALIZE_TOKEN(token) #token

	enum class SerializationFormat {
		Toml,
	};

	class SerialObject {
	public:
		virtual void AddToToml(toml::table* table) { DebugLog(LOG_DEBUG, "To do", false); }
		virtual void FromToml(toml::table* table) { DebugLog(LOG_DEBUG, "To do", false); }

		void SetOSerializedName(std::string newName) {
			this->serialObjectName = newName;
		}

		void SetSerializedUnique(std::string newUnique) {
			this->serialObjectUnique = newUnique;
		}

		void AddSerializedField(std::string name, std::string value) {
			this->serialFields.insert_or_assign(name, value);
		}

		std::string GetSerializedField(std::string name) {
			if (this->serialFields.contains(name)) {
				return this->serialFields[name];
			}
			return "";
		}

		bool ContainSerializedField(std::string name) {
			return this->serialFields.contains(name);
		}

		std::string Serialize(SerializationFormat serialFormat = SerializationFormat::Toml) {
			if (serialFormat == SerializationFormat::Toml) {
				auto out = toml::parse("");

				out.insert_or_assign("ObjectName", this->serialObjectName);

				if (this->serialObjectUnique.empty()) {
					this->serialObjectUnique = xg::newGuid();
				}

				out.insert_or_assign("ObjectUnique", this->serialObjectUnique);

				auto fieldTable = toml::table();
				for(auto & field: this->serialFields) {
					fieldTable.insert_or_assign(field.first, field.second);
				}			

				out.insert_or_assign("fields", fieldTable);

				std::stringstream result;
				result << out;
				return result.str();
			}
		
			return "";
		}

		void Deserialize(std::string input, SerializationFormat serialFormat = SerializationFormat::Toml) {
			if (serialFormat == SerializationFormat::Toml) {
				auto out = toml::parse(input);

				this->serialObjectName = out["ObjectName"].value_or("Null");
				this->serialObjectUnique = out["ObjectUnique"].value_or("Null");

				if (this->serialObjectUnique.empty()) {
					this->serialObjectUnique = xg::newGuid();
				}

				auto fields = out["fields"].as_table();
				if (fields != nullptr) {
					for(auto & field: *fields) {
						std::string key = field.first.data();
						this->serialFields.insert_or_assign(key, field.second.value_or("Null"));
					}
				}
			}
		}

	private:
		std::string serialObjectName;
		std::string serialObjectUnique;
		std::unordered_map<std::string, std::string> serialFields;
	};
}
