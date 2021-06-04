#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC(sz) cpp_malloc(sz)
#define STBI_FREE(p) cpp_free(p)
#define STBI_REALLOC_SIZED(p, sz1, sz2) cpp_realloc_sized(p, sz1, sz2)
#include "cpp_malloc.h"
#include <stb_image.h>