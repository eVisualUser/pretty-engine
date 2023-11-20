#pragma once

#include <PrettyEngine/visualObject.hpp>
#include <PrettyEngine/light.hpp>

#include <vector>

namespace PrettyEngine {
	class RenderFeature {
	public:
		virtual void OnCreated() {}
		virtual void OnInit() {}
		virtual void OnUniform(VisualObject* visualObject) {}
		virtual void OnRender(VisualObject* visualObject) {}

	public:
		std::vector<Light*>* lights;
	};
}