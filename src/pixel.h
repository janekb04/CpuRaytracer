#ifndef PIXEL_H
#define PIXEL_H

struct pixel {
    uint8_t b, g, r, a;
private:
    using component_t = decltype(a);
    constexpr static float conversion_factor = static_cast<float>(std::numeric_limits<component_t>::max());
public:
    pixel() = default;
    constexpr pixel(component_t r, component_t g, component_t b, component_t a) noexcept :
        b{ b },
        g{ g },
        r{ r },
        a{ a } {
    }
    constexpr explicit pixel(const glm::vec4& col) noexcept :
        b{ static_cast<component_t>(col.b * conversion_factor) },
        g{ static_cast<component_t>(col.g * conversion_factor) },
        r{ static_cast<component_t>(col.r * conversion_factor) },
        a{ static_cast<component_t>(col.a * conversion_factor) } {
    }
    explicit operator glm::vec4() const noexcept {
        return glm::vec4{ r, g, b, a } / conversion_factor;
    }
	constexpr pixel& operator=(const glm::vec4& col) noexcept
    {
        return *this = pixel{ col };
    }
	constexpr pixel to_rgba() const noexcept
    {
        return pixel{ b, g, r, a };
    }
};
#endif // PIXEL_H
