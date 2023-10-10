#pragma once

#include <PrettyEngine/transform.hpp>

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
	bool mainCamera = false;
	size_t id = CreateCameraID();
	Projection* projection = &defaultProjection;
	glm::vec3 colorFilter = glm::vec3(1.0f, 1.0f, 1.0f);
};

}
