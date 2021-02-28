#include "TextureManager.h"
#include <vulkan/vulkan.h>
#include <stdexcept>

#include <stb_image.h>
namespace TF
{
TextureManager::TextureManager()
{

} 

TextureManager* TextureManager::Get()
{
    if (m_instance == nullptr)
    {
        m_instance = new TextureManager();
    }
     return m_instance;
}

std::shared_ptr<Texture> TextureManager::LoadTexture(const std::string path)
{
    int texWidth, texHeight, texChannels;
    // will this cause memory leak


    unsigned char* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    //Texture* tex = new Texture();
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }
    m_texturePool[path] = std::make_shared<Texture>(texWidth, texHeight, CPUTextureFormat::R8G8B8A8, pixels);
    stbi_image_free(pixels);
    return m_texturePool[path];
} 

std::shared_ptr<Texture> TextureManager::GetTexture(const std::string path)
{
    return m_texturePool[path];
}
} // namespace TF

