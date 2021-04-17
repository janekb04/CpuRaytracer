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
	glm::vec3 albedo{0.7, 0.7, 0.7};

	[[nodiscard]] shade_info shade(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& view, int& seed) const noexcept override
	{
		const auto scatter_dir = normalize(normal + random_unit_sphere_vector(seed));
		return {
			albedo,
			ray{position, scatter_dir}
		};
	}
};
#endif // MATERIAL_H
