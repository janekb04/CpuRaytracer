#ifndef CPP_MALLOC_H
#define CPP_MALLOC_H

void* cpp_malloc(size_t bytes);
void cpp_free(void* data) noexcept;
void* cpp_realloc_sized(void* data, size_t old_size, size_t new_size);

#endif // CPP_MALLOC_H
