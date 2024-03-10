#ifndef H_RENDER_FEATURE
#define H_RENDER_FEATURE

#include <PrettyEngine/render/visualObject.hpp>
#include <PrettyEngine/render/PrettyGL.hpp>
#include <PrettyEngine/render/light.hpp>

#include <vector>

namespace PrettyEngine {
	/// Add a pass of logic the rendering engine.
	class RenderFeature {
	public:
		virtual void OnCreated() {}
		/// Called when a shader program is loaded
		virtual void OnShaderProgram(GLShaderProgramRefs* refs) {}
		/// Called when the object will be rendered
		virtual void OnInit() {}
		/// Called when the uniforms are updated
		virtual void OnUniform(VisualObject* visualObject) {}
		/// Called when rendering
		virtual void OnRender(VisualObject* visualObject) {}
		
	public:
		std::vector<Light*>* lights;
	};
}

#endif
