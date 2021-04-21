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
		bool front_facing;
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
		const auto intersect_info = _intersect(transformed_ray);

		if(intersect_info)
		{
			const auto& [local_pos, front_facing] = *intersect_info;
			
			const auto pos = glm::vec3{ trans.to_mat4() * glm::vec4{ local_pos, 1.0f } };
			const auto depth = signed_length2(pos - r.origin, r.direction);
			if (depth < t_min || depth > t_max)
				return std::nullopt;
			return hit_info{ depth, pos, local_pos, front_facing, this, transformed_ray };
		}
		
		return std::nullopt;
	}
	[[nodiscard]] geometry_info hit(const hit_info& hit) const noexcept
	{
		auto normal = normalize(trans.to_mat3() * _hit(hit));
		if (!hit.front_facing)
			normal *= -1;
		return { normal };
	}
	virtual ~raytraceable() = default;
protected:
	struct intersect_info
	{
		glm::vec3 local_pos;
		bool front_facing;
	};
	[[nodiscard]] virtual std::optional<intersect_info> _intersect(const ray& r) const noexcept = 0;
	[[nodiscard]] virtual glm::vec3 _hit(const hit_info& hit) const noexcept = 0;
};

class sphere final : public raytraceable
{
public:
	using raytraceable::raytraceable;
protected:
	[[nodiscard]] std::optional<intersect_info> _intersect(const ray& r) const noexcept override
	{
		const auto oc = r.origin /* - center */;
		const auto a = 1.0f;
		const auto half_b = dot(oc, r.direction);
		const auto c = dot(oc, oc) - 1.0f /* (radius * radius) */;
		const auto discriminant = half_b * half_b - a * c;

		if (discriminant < 0) {
			return std::nullopt;
		}

		const auto front_facing = length2(r.origin) >= 1.0f;
		const auto sqrt_disc = front_facing ? -sqrt(discriminant) : sqrt(discriminant);
		return intersect_info{
			r.at((-half_b + sqrt_disc) / a),
			front_facing
		};
	}
	[[nodiscard]] glm::vec3 _hit(const hit_info& hit) const noexcept override
	{
		return hit.local_pos;
	}
};

class plane : public raytraceable
{
public:
	using raytraceable::raytraceable;
protected:
	[[nodiscard]] std::optional<intersect_info> _intersect(const ray& r) const noexcept override
	{
		const auto cos_theta = -r.direction.y; // dot(normal, r.direction)
		const auto dir = /* position */ -r.origin;
		const auto t = -dir.y / cos_theta; // dot(dir, normal) / front_facing
		return intersect_info{
			r.at(t),
			cos_theta < 0.0f
		};
	}
	[[nodiscard]] glm::vec3 _hit(const hit_info& hit) const noexcept override
	{
		return { 0, -1, 0}; // normal
	}
};
class rectangle : public plane
{
public:
	using plane::plane;
protected:
	[[nodiscard]] std::optional<intersect_info> _intersect(const ray& r) const noexcept override
	{
		const auto intersect_info = plane::_intersect(r);
		if (intersect_info)
		{
			const auto& [local_pos, front_facing] = *intersect_info;
			if (local_pos.x >= -1 && local_pos.x <= 1 && local_pos.z >= -1 && local_pos.z <= 1)
			{
				return intersect_info;
			}
		}
		return std::nullopt;
	}
};

template <typename Raytraceable>
class single_sided : public Raytraceable
{
public:
	using Raytraceable::Raytraceable;
protected:
	[[nodiscard]] std::optional<raytraceable::intersect_info> _intersect(const ray& r) const noexcept override
	{
		const auto intersect_info = plane::_intersect(r);
		if (intersect_info)
		{
			const auto& [local_pos, front_facing] = *intersect_info;
			if (front_facing)
			{
				return intersect_info;
			}
		}
		return std::nullopt;
	}
};

#endif // RAYTRACEABLE_H