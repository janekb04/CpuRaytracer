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
	[[nodiscard]] virtual shade_info shade(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& view, int& seed) const noexcept = 0;
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
	[[nodiscard]] shade_info shade(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& view, int& seed) const noexcept override
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
	[[nodiscard]] shade_info shade(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& view, int& seed) const noexcept override
	{
		const auto scatter_dir = reflect(view, normal) + roughness * random_hemisphere_vector(seed);
		return {
			albedo,
			ray{position, scatter_dir}
		};
	}
};
#endif // MATERIAL_H
