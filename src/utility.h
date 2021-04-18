#ifndef UTILITY_H
#define UTILITY_H
#include <chrono>

[[nodiscard]] inline double time_now() noexcept
{
    return static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count()) / 10e9;
}

[[nodiscard]] int init_seed() noexcept
{
    return 0x00269ec3;
}
[[nodiscard]] inline float sfrand(int& seed) noexcept
{
    // from https://www.iquilezles.org/www/articles/sfrand/sfrand.htm
    union
    {
        float f_res;
        unsigned int i_res;
    };

    seed *= 16807;
    i_res = (static_cast<unsigned>(seed) >> 9) | 0x40000000;
    return f_res - 3.0f;
}
[[nodiscard]] inline float frand(int& seed) noexcept
{
    // from https://www.iquilezles.org/www/articles/sfrand/sfrand.htm
    union
    {
        float f_res;
        unsigned int i_res;
    };

    seed *= 16807;
    i_res = (static_cast<unsigned>(seed) >> 9) | 0x3f800000;
    return f_res - 1.0f;
}
inline constexpr long double pi = 3.14159265358979323846264338327950288419716939937510L;
template <typename Arithmetic>
Arithmetic degToRad(const Arithmetic& x)
{
    return x * static_cast<Arithmetic>(pi / 180.0L);
}
template <typename Arithmetic>
Arithmetic radToDeg(const Arithmetic& x)
{
    return x * static_cast<Arithmetic>(180.0L / pi);
}
[[nodiscard]] inline float fast_sqrt(float x) noexcept
{
	// from https://stackoverflow.com/a/18662665/12501684
    auto i = *reinterpret_cast<unsigned*>(&x);
    i += 127 << 23;
    i >>= 1;
    return *reinterpret_cast<float*>(&i);
}
[[nodiscard]] inline float fast_acos(float a) noexcept
{
	// from https://stackoverflow.com/a/48157547/12501684
    const float C = 0.10501094f;
    const auto t = (a < 0) ? (-a) : a;  // handle negative arguments
    const auto u = 1.0f - t;
    const auto s = fast_sqrt(u + u);
    auto r = C * u * s + s;  // or fmaf (C * u, s, s) if FMA support in hardware
    if (a < 0) r = static_cast<float>(pi) - r;  // handle negative arguments
    return r;
}
template<typename Arithmetic>
Arithmetic fast_cos(Arithmetic x) noexcept
{
	// from https://stackoverflow.com/a/28050328/12501684
    constexpr Arithmetic tp = 1. / (2. * pi);
    x *= tp;
    x -= Arithmetic(.25) + std::floor(x + Arithmetic(.25));
    x *= Arithmetic(16.) * (std::abs(x) - Arithmetic(.5));
    return x;
}
// Random vector on a unit hemisphere oriented along with normal (0, 1, 0)
[[nodiscard]] inline glm::vec3 random_hemisphere_vector(int& seed) noexcept
{
	// loosely based on http://corysimon.github.io/articles/uniformdistn-on-sphere/
    const auto cos_theta = fast_cos(2.0f * static_cast<float>(pi) * frand(seed));
    const auto sin_theta = fast_sqrt(1.0f - cos_theta * cos_theta);

    const auto cos_phi = sfrand(seed);
    const auto sin_phi = fast_sqrt(1.0f - cos_phi * cos_phi);

    const auto x = sin_phi * cos_theta;
    const auto y = sin_phi * sin_theta;
    const auto z = cos_phi;

    return { x, y, z };
}
[[nodiscard]] inline glm::vec3 random_unit_sphere_vector(int& seed) noexcept
{
    auto result = random_hemisphere_vector(seed);
    *reinterpret_cast<int*>(&result.y) |= (seed & (1 << 24)) << 7; // randomly flip sign bit
    return result;
}
template <typename Vec>
[[nodiscard]] typename Vec::value_type signed_length2(const Vec& v, const Vec& dir)
{
    return copysign(length2(v), dot(v, dir));
}
template <typename Vec>
[[nodiscard]] typename Vec::value_type signed_length(const Vec& v, const Vec& dir)
{
    return copysign(length(v), dot(v, dir));
}
#endif // UTILITY_H
