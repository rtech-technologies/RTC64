#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_ONLY_PNG
/* Modified by Sovereign: Dedicated STB Image implementation for kernel splash */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "external/stb_image.h"
#pragma GCC diagnostic pop
