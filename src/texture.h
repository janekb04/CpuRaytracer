#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <memory>
#include <glm/glm.hpp>
#include "array_wrapper.h"

template <typename Color>
class texture
{
	std::unique_ptr<Color[]> m_buffer{};
	size_t m_width{}, m_height{};
public:
	texture() noexcept = default;
	
	texture(array_wrapper<Color, 2>&& data) noexcept :
		m_width{ data.extents[1] },
		m_height{ data.extents[0] },
		m_buffer{ data.data }
	{
	}

	[[nodiscard]] Color sample(float u, float v) const noexcept
	{
		const auto x = std::min(static_cast<size_t>(u * m_width), m_width - 1);
		const auto y = std::min(static_cast<size_t>(v * m_height), m_height - 1);
		return buffer()[y][x];
	}
	
	void update_size(size_t new_width, size_t new_height)
	{
		m_width = new_width;
		m_height = new_height;
		m_buffer = std::make_unique<Color[]>(m_width * m_height);
	}
	[[nodiscard]] auto buffer() const
	{
		return array_wrapper<Color, 2>{m_buffer.get(), m_height, m_width};
	}
	[[nodiscard]] size_t width() const
	{
		return m_width;
	}
	[[nodiscard]] size_t height() const
	{
		return m_height;
	}
};
#endif // FRAMEBUFFER_H
