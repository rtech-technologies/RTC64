/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_GIF
#define STBI_NO_PSD
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_NO_HDR
#define STBI_NO_TGA
#define STBI_NO_BMP
/* Modified by Sovereign: Dedicated STB Image implementation for kernel splash & wallpaper engine */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "external/stb_image.h"
#pragma GCC diagnostic pop
