#ifndef RAYTRACEABLE_H
#define RAYTRACEABLE_H

#include <optional>
#include <glm/glm.hpp>
#include "ray.h"
#include "material.h"

class raytraceable
{
public:
	struct hit_info
	{
		float depth;
		glm::vec3 pos;
		glm::vec3 local_pos;
		const raytraceable* hit;
		ray transformed_ray;
	};
	struct geometry_info
	{
		glm::vec3 normal;
	};
	const material* mat;
	const transform trans;
private:
	const glm::mat4 inv_trans;
public:
	raytraceable(const material& m, const transform& trans) :
		mat{ &m },
		trans{ trans },
		inv_trans{inverse(trans.to_mat4())}
	{
	}
	
	[[nodiscard]] std::optional<hit_info> intersect(const ray& r, float t_min, float t_max) const noexcept
	{ // TODO: this function takes 75% of all processing time. optimize (maybe with SIMD??)
		const auto transformed_ray = inv_trans * r;
		const auto local_pos_opt = _intersect(transformed_ray);

		if(local_pos_opt)
		{
			const auto pos = glm::vec3{ trans.to_mat4() * glm::vec4{ *local_pos_opt, 1.0f } };
			const auto depth = signed_length2(pos - r.origin, r.direction);
			if (depth < t_min || depth > t_max)
				return std::nullopt;
			return hit_info{ depth, pos, *local_pos_opt, this, transformed_ray };
		}
		
		return std::nullopt;
	}
	[[nodiscard]] geometry_info hit(const hit_info& hit) const noexcept
	{
		return { normalize(trans.to_mat3() * _hit(hit)) };
	}
	virtual ~raytraceable() = default;
protected:
	[[nodiscard]] virtual std::optional<glm::vec3> _intersect(const ray& r) const noexcept = 0;
	[[nodiscard]] virtual glm::vec3 _hit(const hit_info& hit) const noexcept = 0;
};

class sphere final : public raytraceable
{
public:
	sphere(const material& mat, const transform& trans) :
		raytraceable{mat, trans}
	{
	}
private:
	[[nodiscard]] std::optional<glm::vec3> _intersect(const ray& r) const noexcept override
	{
		const auto oc = r.origin /* - center */;
		const auto a = 1.0f;
		const auto half_b = dot(oc, r.direction);
		const auto c = dot(oc, oc) - 1.0f /* (radius * radius) */;
		const auto discriminant = half_b * half_b - a * c;

		if (discriminant < 0) {
			return std::nullopt;
		}
		return r.at((-half_b - sqrt(discriminant)) / a);
	}
	[[nodiscard]] glm::vec3 _hit(const hit_info& hit) const noexcept override
	{
		return hit.local_pos;
	}
};

class plane : public raytraceable
{
public:
	plane(const material& mat, const transform& trans) :
		raytraceable{mat, trans}
	{
	}
protected:
	[[nodiscard]] std::optional<glm::vec3> _intersect(const ray& r) const noexcept override
	{
		const auto front_facing = -r.direction.y; // dot(normal, r.direction)
		if (front_facing < 0) 
		{
			const auto dir = /* position */ -r.origin;
			const auto t = -dir.y / front_facing; // dot(dir, normal) / front_facing
			return r.at(t);
		}

		return std::nullopt;
	}
	[[nodiscard]] glm::vec3 _hit(const hit_info& hit) const noexcept override
	{
		return { 0, -1, 0}; // normal
	}
};
#endif // RAYTRACEABLE_H