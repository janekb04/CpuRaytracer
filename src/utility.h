#ifndef UTILITY_H
#define UTILITY_H
#include <chrono>

inline float sfrand(int& seed)
{
    // from https://www.iquilezles.org/www/articles/sfrand/sfrand.htm

    union
    {
        float f_res;
        unsigned int i_res;
    };
	
    seed *= 16807;
    i_res = (static_cast<unsigned>(seed) >> 9) | 0x40000000;
    return(f_res - 3.0f);
}
inline double time_now() noexcept
{
    return static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count()) / 10e9;
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
#endif // UTILITY_H
