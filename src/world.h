#ifndef WORLD_H
#define WORLD_H
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "ray.h"
#include "raytraceable.h"

class world
{
	std::vector<std::unique_ptr<raytraceable>> objects;

	struct trace_result
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec4 color;
		std::optional<ray> scattered;
	};
	
	[[nodiscard]] static glm::vec4 backdrop(const glm::vec3& dir) noexcept
	{
		float t = 0.5 * (dir.y + 1.0);
		return { (1.0f - t) * glm::vec3(1.0, 1.0, 1.0) + t * glm::vec3(0.5, 0.7, 1.0), 1.0 };
	}
	[[nodiscard]] trace_result trace_single(const ray& r, float min_t, float max_t, int seed) const noexcept
	{
		raytraceable::hit_info hit_info{ max_t , nullptr };
		for (auto&& obj : objects)
		{
			hit_info = obj->intersect(r, min_t, hit_info.t).value_or(hit_info);
		}
		if (!hit_info.hit)
		{
			return {
				{},
				{},
				backdrop(r.direction),
				std::nullopt
			};
		}

		const auto geometry_info = hit_info.hit->hit(r, hit_info);
		const auto position = r.at(hit_info.t);
		return {
			position,
			geometry_info.normal,
			hit_info.hit->mat->shade(
				position,
				geometry_info.normal,
				r.direction
			),
			ray(position,normalize(geometry_info.normal + random_unit_sphere_vector(seed)))
		};
	}
public:
	[[nodiscard]] glm::vec4 raytrace(const ray& r, int depth, int& seed) const noexcept
	{
		if (depth <= 0)
			return glm::vec4(0, 0, 0, 1);
		
		const auto trace_result = trace_single(r, 0.001f, std::numeric_limits<float>::infinity(), seed);
		if (trace_result.scattered)
		{
			return 0.5 * raytrace(*trace_result.scattered, depth - 1, seed);
		}
		return trace_result.color;
	}
	void add(raytraceable* object)
	{
		objects.emplace_back(object);
	}
};
#endif // WORLD_H
