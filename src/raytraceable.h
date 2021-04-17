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
		float t;
		const raytraceable* hit;
	};
	struct geometry_info
	{
		glm::vec3 normal;
	};
	std::unique_ptr<material> mat{ static_cast<material*>(new lambertian_material) };
	
	[[nodiscard]] std::optional<hit_info> intersect(const ray& r, float t_min, float t_max) const noexcept
	{
		const auto t = _intersect(r);
		if (t < t_min || t > t_max)
			return std::nullopt;
		return hit_info{ t, this };
	}
	[[nodiscard]] geometry_info hit(const ray& r, const hit_info& hit) const noexcept
	{
		return { _hit(r, hit) };
	}
	virtual ~raytraceable() = default;
protected:
	[[nodiscard]] virtual float _intersect(const ray& r) const noexcept = 0;
	[[nodiscard]] virtual glm::vec3 _hit(const ray& r, const hit_info& hit) const noexcept = 0;
};

class sphere final : public raytraceable
{
public:
	glm::vec3 center{};
	float radius{ 1.0f };

	sphere(const glm::vec3& center, float radius) :
		center{ center },
		radius{ radius }
	{
	}
private:
	[[nodiscard]] float _intersect(const ray& r) const noexcept override
	{
		const auto oc = r.origin - center;
		const auto a = 1.0f;
		const auto half_b = dot(oc, r.direction);
		const auto c = dot(oc, oc) - radius * radius;
		const auto discriminant = half_b * half_b - a * c;

		if (discriminant < 0) {
			return -1.0f;
		}
		return (-half_b - sqrt(discriminant)) / a;
	}
	[[nodiscard]] glm::vec3 _hit(const ray& r, const hit_info& hit) const noexcept override
	{
		return normalize(r.at(hit.t) - center);
	}
};

class plane final : public raytraceable
{
public:
	glm::vec3 position{};
	glm::vec3 normal{0, -1, 0};
private:
	[[nodiscard]] float _intersect(const ray& r) const noexcept override
	{
		const auto front_facing = dot(normal, r.direction);
		if (front_facing < 0) 
		{
			const auto dir = position - r.origin;
			const auto t = dot(dir, normal) / front_facing;
			return t;
		}

		return -1.0f;
	}
	[[nodiscard]] glm::vec3 _hit(const ray& r, const hit_info& hit) const noexcept override
	{
		return normal;
	}
};

#endif // RAYTRACEABLE_H
