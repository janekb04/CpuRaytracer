//
// Created by Jan on 13-Apr-21.
//

#ifndef ENGINE_RAY_H
#define ENGINE_RAY_H

#include <glm/glm.hpp>

struct ray {
    glm::vec3 origin, direction;

    [[nodiscard]] glm::vec3 at(float t) const {
        return origin + t * direction;
    }
};

#endif //ENGINE_RAY_H
