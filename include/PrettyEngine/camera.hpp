#pragma once

#include <PrettyEngine/texture.hpp>
#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/transform.hpp>
#include <PrettyEngine/gl.hpp>

#include <glm/glm.hpp>
#include <string>

namespace PrettyEngine {
	class Projection {
	public:
		float fov = 45.0f;
		float aspectRatio = 1.0f;
		float nearPlane = 0.001f;
		float farPlane = 1000.0f;
	};

	static size_t CAMERA_MAX_ID = 0;

	static size_t CreateCameraID() {
		CAMERA_MAX_ID++;
		return CAMERA_MAX_ID;
	}

	static Projection defaultProjection;

	class Camera: public Transform {
	public:
		Camera() {
			this->texture.textureID = 0;
			this->texture.textureType = TextureType::Base;
			this->texture.colorChannels = (int)TextureChannels::RGBA;
			this->texture.filter = TextureFilter::Linear;
			this->texture.useGC = false;
			this->texture.wrap = TextureWrap::ClampToBorder;
			this->texture.name = "ENGINE_RESERVED_TEXTURE_NAME_FOR_CAMERA_" + std::to_string(this->id);
		}

		void SetRenderToTexture(bool state) {
			if (state == false) {
				glDeleteTextures(1, &this->texture.textureID);
				glDeleteFramebuffers(1, &this->glFrameBufferID);
				this->renderToTexture = false;
			}
			else {
				// Generate Texture
				glGenTextures(1, &this->texture.textureID);
				glBindTexture(GL_TEXTURE_2D, this->texture.textureID);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->resolution.x, this->resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				// Generate Framebuffer
				glGenFramebuffers(1, &this->glFrameBufferID);
				glBindFramebuffer(GL_FRAMEBUFFER, this->glFrameBufferID);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->texture.textureID, 0);

				// Handle frame buffer errors
				if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				    DebugLog(LOG_ERROR, "Frame Buffer object not complete.", true);
				} else {
					this->renderToTexture = true;
				}
			}
		}

		void SetTextureResolution(glm::vec2 newResolution) {
			this->resolution = newResolution;
			this->SetRenderToTexture(false);
			if (this->renderToTexture) {
				this->SetRenderToTexture(this->renderToTexture);
			}
		}

		void Render() {
			if (this->renderToTexture) {
				if (this->texture.textureID == 0) {
					DebugLog(LOG_ERROR, "Camera: " << this->id << " have no texture generated.", true);
				}
				if (this->glFrameBufferID == 0) {
					DebugLog(LOG_ERROR, "Camera: " << this->id << " have no framebuffer generated.", true);
				}
				
				if (this->texture.textureID != 0 && this->glFrameBufferID != 0) {
					glBindFramebuffer(GL_FRAMEBUFFER, this->glFrameBufferID);
				}
			}
		}

		/// Make sure that the camera do not let something binded
		void ResetRender() {
			if (this->renderToTexture) {
 				glBindFramebuffer(GL_FRAMEBUFFER, 0);
 			}
		}

		Texture* GetTexture() {
			return &this->texture;
		}

	public:
		Texture texture;
		bool renderToTexture = false;
		bool active = false;
		size_t id = CreateCameraID();
		Projection* projection = &defaultProjection;
		glm::vec3 colorFilter = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec2 resolution = glm::vec2(600, 800);

	private:
		unsigned int glFrameBufferID;
	};
}
