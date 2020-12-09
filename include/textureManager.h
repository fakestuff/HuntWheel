#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "texture.h"
namespace TF
{
class TextureManager
{
    public:
        static TextureManager* Get();
        // method
        TextureManager();
        std::shared_ptr<Texture> LoadTexture(const std::string path);
        std::shared_ptr<Texture> GetTexture(const std::string path);
        void CleanUp();

        // member
        std::unordered_map<std::string, std::shared_ptr<Texture>> m_texturePool;
    private:
        inline static TextureManager* m_instance = nullptr;

};
}
