#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <memory>
#include <glm/glm.hpp>
#include "array_wrapper.h"

class framebuffer
{
	std::unique_ptr<glm::vec4[]> m_buffer;
	size_t m_width, m_height;
public:
	void update_size(size_t new_width, size_t new_height)
	{
		if (new_width == m_width && new_height == m_height) return;
		m_width = new_width;
		m_height = new_height;
		m_buffer = std::make_unique<glm::vec4[]>(m_width * m_height);
	}
	[[nodiscard]] auto buffer() const
	{
		return array_wrapper<glm::vec4, 2>{m_buffer.get(), m_height, m_width};
	}
};
#endif // FRAMEBUFFER_H
