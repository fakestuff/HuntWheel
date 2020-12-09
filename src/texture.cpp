#include "texture.h"

namespace TF
{
    Texture::Texture(int width, int height, CPUTextureFormat cpuFormat, unsigned char* pixels)
    {
        Width = width;
        Height = height;
        CPUFormat = cpuFormat;
        
        switch (cpuFormat)
        {
            case CPUTextureFormat::R8:
                TexelSize = 1;
                break;
            case CPUTextureFormat::R8G8:
                TexelSize = 2;
                break;
            case CPUTextureFormat::R8G8B8:
                TexelSize = 3;
                break;
            case CPUTextureFormat::R8G8B8A8:
                TexelSize = 4;
                break;
            default:
                TexelSize = 4;
        }
        Pixels.reserve(Width*Height*TexelSize);
        memcpy(Pixels.data(), pixels, Width*Height*TexelSize);
    }

}