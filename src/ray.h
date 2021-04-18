#ifndef ENGINE_RAY_H
#define ENGINE_RAY_H

#include <glm/glm.hpp>

struct ray {
    glm::vec3 origin, direction;

    [[nodiscard]] glm::vec3 at(float t) const {
        return origin + t * direction;
    }
};

inline ray operator*(const glm::mat4& trans, const ray& r) noexcept
{
    return ray{
		trans * glm::vec4{r.origin, 1.0f},
    	normalize(trans * glm::vec4{r.direction, 0.0f})
    };
}

#endif //ENGINE_RAY_H
