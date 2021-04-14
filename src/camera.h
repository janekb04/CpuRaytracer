#ifndef CAMERA_H
#define CAMERA_H
#include <glm/glm.hpp>
#include "transform.h"
#include "ray.h"

class camera
{
	transform trans;
	double fov; // vertical fov
	double aspect_ratio;

	// cached
public:
};
#endif // CAMERA_H
