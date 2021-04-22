#ifndef MATERIAL_H
#define MATERIAL_H
#include <glm/glm.hpp>
#include "ray.h"

class material
{
public:
	struct shade_info
	{
		glm::vec3 attenuation;
		std::optional<ray> scattered;
	};
	[[nodiscard]] virtual shade_info shade(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& view, bool front_facing, int& seed) const noexcept = 0;
	[[nodiscard]] virtual glm::vec3 emission(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& view, bool front_facing, int& seed) const noexcept
	{
		return glm::vec3{ 0, 0, 0 };
	}
	virtual ~material() = default;
};
class lambertian_material : public material
{
public:
	glm::vec3 albedo;

	explicit lambertian_material(const glm::vec3& albedo) :
		albedo{albedo}
	{
	}
private:
	[[nodiscard]] shade_info shade(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& view, bool front_facing, int& seed) const noexcept override
	{
		const auto scatter_dir = normalize(normal + random_unit_sphere_vector(seed));
		return {
			albedo,
			ray{position, scatter_dir}
		};
	}
};
class metallic_material : public material
{
public:
	glm::vec3 albedo;
	float roughness;

	metallic_material(const glm::vec3& albedo, float roughness) :
		albedo{albedo},
		roughness{roughness}
	{
	}
private:
	[[nodiscard]] shade_info shade(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& view, bool front_facing, int& seed) const noexcept override
	{
		const auto scatter_dir = reflect(view, normal) + roughness * random_hemisphere_vector(seed);
		return {
			albedo,
			ray{position, scatter_dir}
		};
	}
};
class portal_material : public material
{
	glm::mat4 from_to_transform;
public:
	portal_material(const transform& from, const transform& to) :
		from_to_transform{to.to_mat4() * inverse(from.to_mat4())}
	{
	}
private:
	[[nodiscard]] shade_info shade(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& view, bool front_facing, int& seed) const noexcept override
	{
		return {
			glm::vec3{1,1,1},
			from_to_transform * ray{position, view}
		};
	}
};
class emmisive_material : public material
{
public:
	glm::vec3 color;

	emmisive_material(const glm::vec3& color) :
		color{ color }
	{
	}
protected:
	[[nodiscard]] shade_info shade(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& view, bool front_facing, int& seed) const noexcept override
	{
		return {
			glm::vec3{1,1,1},
			std::nullopt
		};
	}
	[[nodiscard]] glm::vec3 emission(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& view, bool front_facing, int& seed) const noexcept override
	{
		return color;
	}
};
class dielectric_material : public material
{
public:
	float ior;

	dielectric_material(const float ior) :
		ior{ior}
	{
	}
protected:
	[[nodiscard]] shade_info shade(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& view, bool front_facing, int& seed) const noexcept override
	{
		const auto refraction_ratio = front_facing ? (1.0f / ior) : ior;

		const auto cos_theta = fmin(dot(-view, normal), 1.0f);
		const auto sin_theta = sqrt(1.0f - cos_theta * cos_theta);

		bool cannot_refract = refraction_ratio * sin_theta > 1.0f;
		glm::vec3 direction;

		if (cannot_refract || reflectance(cos_theta, refraction_ratio) > frand(seed))
			direction = reflect(view, normal);
		else
			direction = ::refract(view, normal, refraction_ratio);
		
		return {
			glm::vec3{1,1,1},
			ray{position, direction}
		};
	}
private:
	static double reflectance(float cosine, float ref_idx) {
		// Use Schlick's approximation for reflectance.
		auto r0 = (1 - ref_idx) / (1 + ref_idx);
		r0 = r0 * r0;
		return r0 + (1 - r0) * pow((1 - cosine), 5);
	}
};
#endif // MATERIAL_H
