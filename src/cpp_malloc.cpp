#include <cstddef>
#include <memory>
#include "cpp_malloc.h"

void* cpp_malloc(size_t bytes)
{
	char* data = new char[bytes];
	return static_cast<void*>(data);
}

void cpp_free(void* data) noexcept
{
	char* as_chr_ptr = static_cast<char*>(data);
	delete[] as_chr_ptr;
}

void* cpp_realloc_sized(void* data, size_t old_size, size_t new_size)
{
	void* new_data = cpp_malloc(new_size);
	memcpy(new_data, data, std::min(old_size, new_size));
	cpp_free(data);
	return new_data;
}