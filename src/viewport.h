#ifndef VIEWPORT_H
#define VIEWPORT_H
#include <glm/glm.hpp>

struct viewport {
    double aspect_ratio;
    const double height = 2.0;
    double width;
    const double focal_length = 1.0;
    const glm::vec3 origin{ 0, 0, 0 };
    glm::vec3 horizontal;
    const glm::vec3 vertical{ 0, height, 0 };
    glm::vec3 lower_left_corner;

    viewport() noexcept = default;

    void resize(size_t _width, size_t _height) {
        aspect_ratio = static_cast<double>(_width) / _height;
        width = aspect_ratio * height;
        horizontal = { width, 0, 0 };
        lower_left_corner = origin - horizontal * 0.5f - vertical * 0.5f - glm::vec3{ 0, 0, focal_length };
    }

    glm::vec3 ray_from_uv(float u, float v) {
        return glm::vec3(lower_left_corner + u * horizontal + v * vertical - origin);
    }
} viewport;
#endif // VIEWPORT_H
