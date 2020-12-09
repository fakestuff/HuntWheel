#pragma once
#include <memory>
#include <vector>
namespace TF
{
    enum CPUTextureFormat
    {
        R8,
        R8G8,
        R8G8B8,
        R8G8B8A8,
        FLOAT16X3,
        FLOAT16X4,
    };
    class Texture
    {
        public:
            Texture(int width, int height, CPUTextureFormat cpuFormat, unsigned char* pixels);
            CPUTextureFormat CPUFormat;
            //GPU Format
            //GPU HANDLE
            int Width;
            int Height;
            int TexelSize; // how many bytes per texel
            
            std::vector<unsigned char> Pixels;

        private:
    };
} // namespace TF

