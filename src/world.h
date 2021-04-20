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
		glm::vec3 color;
		std::optional<ray> scattered;
	};
	
	[[nodiscard]] static glm::vec4 backdrop(const glm::vec3& dir) noexcept
	{
		const auto t = 0.5f * (dir.y + 1.0f);
		return { (1.0f - t) * glm::vec3(1.0, 1.0, 1.0) + t * glm::vec3(0.5, 0.7, 1.0), 1.0 };
	}
	[[nodiscard]] trace_result trace_single(const ray& r, float min_t, float max_t, int& seed) const noexcept
	{
		raytraceable::hit_info hit_info{ max_t };
		for (auto&& obj : objects)
		{
			hit_info = obj->intersect(r, min_t, hit_info.depth).value_or(hit_info);
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

		const auto geometry_info = hit_info.hit->hit(hit_info);
		const auto normal = geometry_info.normal;
		const auto position = hit_info.pos;
		const auto shade_info = hit_info.hit->mat->shade(
			position,
			normal,
			r.direction,
			seed
		);
		return {
			position,
			normal,
			shade_info.attenuation,
			shade_info.scattered
		};
	}
public:
	[[nodiscard]] glm::vec3 raytrace(const ray& r, int depth, int& seed) const noexcept
	{
		if (depth <= 0)
			return glm::vec3(0, 0, 0);
		
		const auto trace_result = trace_single(r, 0, std::numeric_limits<float>::infinity(), seed);
		if (trace_result.scattered)
		{
			const auto ray_origin = trace_result.scattered->origin + trace_result.scattered->direction * 0.005f;
			return trace_result.color * raytrace(ray{ ray_origin, trace_result.scattered->direction }, depth - 1, seed);
		}
		return trace_result.color;
	}
	void add(raytraceable* object)
	{
		objects.emplace_back(object);
	}
};
#endif // WORLD_H
