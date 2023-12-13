#ifndef H_RENDER_FEATURE
#define H_RENDER_FEATURE

#include <PrettyEngine/render/visualObject.hpp>
#include <PrettyEngine/render/light.hpp>

#include <vector>

namespace PrettyEngine {
	/// Add a pass of logic the rendering engine.
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

#endif