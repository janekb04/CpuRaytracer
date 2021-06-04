#ifndef HDRI_H
#define HDRI_H

#include <stdexcept>
#include <glm/glm.hpp>
#include <string>
#include <stb_image.h>

#include "texture.h"

texture<glm::vec3> read_hdri(const std::string& path)
{
	int width, height, channels;
	float* image = stbi_loadf(path.c_str(), &width, &height, &channels, 3);
	if (!image)
	{
		throw std::runtime_error("Couldn't read HDRI file");
	}
	texture<glm::vec3> t{ array_wrapper<glm::vec3, 2>{reinterpret_cast<glm::vec3*>(image), static_cast<size_t>(height), static_cast<size_t>(width)} };
	if (channels != 3)
	{
		throw std::runtime_error("Incorrect number of channels in HDRI");
	}
	return t;
}

#endif // HDRI_H
