#ifndef H_TEXTURE
#define H_TEXTURE

#include <PrettyEngine/gc.hpp>

#include <glad/glad.h>

#include <string>

namespace PrettyEngine {
	enum class TextureType {
		Base,
		Normal,
		Transparency,
	};

	enum class TextureFilter {
		Linear = GL_LINEAR,
		Nearest = GL_NEAREST,

		MipMapNearestLinear = GL_NEAREST_MIPMAP_LINEAR,
		MipMapNearestNearest = GL_NEAREST_MIPMAP_NEAREST,
		MipMapLinearLinear = GL_LINEAR_MIPMAP_LINEAR,
		MipMapLinearNearest = GL_LINEAR_MIPMAP_NEAREST,
	};

	enum class TextureWrap {
		Repeat = GL_REPEAT,
		MirroredRepeat = GL_MIRRORED_REPEAT,
		ClampToEdge = GL_CLAMP_TO_EDGE,
		ClampToBorder = GL_CLAMP_TO_BORDER,
	};

	enum class TextureChannels {
		RGBA = GL_RGBA,
		RGB = GL_RGB,
		Red = GL_RED,
		Green = GL_GREEN,
		Blue = GL_BLUE,
		Alpha = GL_ALPHA,
	};

	class Texture: public GCObject {
	public:
		std::string name;

		unsigned int textureID;
		unsigned int colorChannels;
		TextureWrap wrap;
		TextureFilter filter;
		TextureType textureType;
	};
}

#endif