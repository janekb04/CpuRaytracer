#ifndef COLOR_H
#define COLOR_H

#include <glm/glm.hpp>

template <typename Arithmetic>
Arithmetic rgb_to_srgb(const Arithmetic& col) {
    //return col <= 0.0031308
    //    ? col * 12.92
    //    : pow(col, 1.0 / 2.4) * 1.055 - 0.055;
    return pow(col, Arithmetic(1.0 / 2.2));
}
template <typename Arithmetic>
Arithmetic srgb_to_rgb(const Arithmetic& col) {
    //return col <= 0.04045
    //    ? col / 12.92
    //    : pow((col + 0.055) / 1.055, 2.4);
    return pow(col, Arithmetic(2.2));
}

glm::vec3 RRTAndODTFit(const glm::vec3& v)
{
	glm::vec3 a = v * (v + 0.0245786f) - 0.000090537f;
	glm::vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

glm::vec3 ACESFitted(glm::vec3 color)
{
    //from https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
    const glm::mat3 ACESInputMat =
    {
        {0.59719f, 0.35458f, 0.04823f},
        {0.07600f, 0.90834f, 0.01566f},
        {0.02840f, 0.13383f, 0.83777f}
    };
    const glm::mat3 ACESOutputMat =
    {
        { 1.60475f, -0.53108f, -0.07367f},
        {-0.10208f,  1.10813f, -0.00605f},
        {-0.00327f, -0.07276f,  1.07602f}
    };

    color = color * ACESInputMat;
    color = RRTAndODTFit(color);
    color = color * ACESOutputMat;
    color = clamp(color, glm::vec3{ 0,0,0 }, glm::vec3{ 1,1,1 });
    return color;
}

glm::vec3 rgb_to_hsv(glm::vec3 c)
{
    glm::vec4 K = glm::vec4(0.0f, -1.0f / 3.0f, 2.0f / 3.0f, -1.0f);
    glm::vec4 p = mix(glm::vec4(c.b, c.g, K.w, K.z), glm::vec4(c.g, c.b, K.x, K.y), glm::step(c.b, c.g));
    glm::vec4 q = mix(glm::vec4(p.x, p.y, p.w, c.r), glm::vec4(c.r, p.y, p.z, p.x), glm::step(p.x, c.r));

    float d = q.x - glm::min(q.w, q.y);
    float e = 1.0e-10;
    return glm::vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

glm::vec3 hsv_to_rgb(glm::vec3 c)
{
    glm::vec4 K = glm::vec4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
    glm::vec3 p = abs(glm::fract(glm::vec3{ c.x } + glm::vec3{ K }) * 6.0f - glm::vec3{ K.w });
    return c.z * glm::mix(glm::vec3{ K.x }, glm::clamp(p - glm::vec3{ K.x }, 0.0f, 1.0f), c.y);
}


glm::vec4 tonemap(const glm::vec4& col)
{
    return glm::vec4{ ACESFitted(rgb_to_srgb(col)), col.w };
}


#endif // COLOR_H
