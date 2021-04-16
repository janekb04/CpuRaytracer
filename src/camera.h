#ifndef CAMERA_H
#define CAMERA_H
#include <glm/glm.hpp>
#include "transform.h"
#include "ray.h"
#include "utility.h"

class camera
{
	glm::vec3 lower_left_corner;
	glm::vec3 horizontal;
	glm::vec3 vertical;
public:
	transform trans;
	
	camera() = default;
	
	void update(float vertical_fov, float aspect_ratio)
	{
		float theta = degToRad(vertical_fov);
		float h = tan(theta / 2.0f);
		float viewport_height = 2.0f * h;
		float viewport_width = aspect_ratio * viewport_height;

		horizontal = viewport_width * trans.right();
		vertical = viewport_height * trans.up();
		lower_left_corner = -horizontal / 2.0f - vertical / 2.0f - trans.forward();
	}
	ray get_ray(float u, float v) const
	{
		return ray(trans.get_position(), glm::normalize(lower_left_corner + u * horizontal + v * vertical));
	}
};
#endif // CAMERA_H
