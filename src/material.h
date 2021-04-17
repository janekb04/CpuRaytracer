#ifndef MATERIAL_H
#define MATERIAL_H
#include <glm/glm.hpp>

class material
{
public:
	[[nodiscard]] virtual glm::vec4 shade(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& view) const noexcept = 0;
	virtual ~material() = default;
};
class normal_debug_material : public material
{
	[[nodiscard]] glm::vec4 shade(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& view) const noexcept override
	{
		return 0.5f * glm::vec4(normal.x + 1, normal.y + 1, normal.z + 1, 2.0);
	}
};
class pos_debug_material : public material
{
	[[nodiscard]] glm::vec4 shade(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& view) const noexcept override
	{
		auto hit_point = abs(position);
		hit_point -= glm::ivec3{ hit_point };
		hit_point = abs(hit_point);

		return {
				static_cast<float>(hit_point.x >= 0.9 || hit_point.x <= 0.1),
				static_cast<float>(hit_point.y >= 0.9 || hit_point.y <= 0.1),
				static_cast<float>(hit_point.z >= 0.9 || hit_point.z <= 0.1),
				1.0
		};
	}
};
class mirror_material : public material
{
	[[nodiscard]] glm::vec4 shade(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& view) const noexcept override
	{
		auto hit_point = abs(position);
		hit_point -= glm::ivec3{ hit_point };
		hit_point = abs(hit_point);

		return {
				static_cast<float>(hit_point.x >= 0.9 || hit_point.x <= 0.1),
				static_cast<float>(hit_point.y >= 0.9 || hit_point.y <= 0.1),
				static_cast<float>(hit_point.z >= 0.9 || hit_point.z <= 0.1),
				1.0
		};
	}
};
#endif // MATERIAL_H
