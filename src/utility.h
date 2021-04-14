#include <chrono>

static float sfrand(int& seed)
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
static double time_now() noexcept
{
    return static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count()) / 10e9;
}