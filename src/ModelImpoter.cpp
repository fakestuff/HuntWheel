#define TINYGLTF_IMPLEMENTATION
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#define TINYGLTF_USE_RAPIDJSON

#include <ModelImporter.h>

#include <tiny_gltf.h>

using namespace tinygltf;

namespace TF
{
    void LoadNode(const tinygltf::Model& glTFModel, const tinygltf::Node& inputNode)
    {
        std::cout << inputNode.name << std::endl;
        for (auto i = 0; i < inputNode.children.size(); i++)
        {
            LoadNode(glTFModel, glTFModel.nodes[inputNode.children[i]]);
        }
    }
    void LoadGltfFModel(TFVkGfxDevice tfVulkanDevice, World* world, fs::path gltfPath)
    {

        Model glTFModel;
        TinyGLTF loader;
        std::string err;
        std::string warn;

        std::vector<uint32_t> indexBuffer; // prepare cpu data for uploading
        std::vector<Vertex> vertexBuffer;

        bool fileLoaded = loader.LoadASCIIFromFile(&glTFModel, &err, &warn, gltfPath.generic_string());
        if (!fileLoaded)
        {
            throw std::runtime_error("failed to open gltf file!" + gltfPath.generic_string());
        }
        
        // Load Mesh
        
        // Load Texture
        for (size_t i = 0; i < glTFModel.images.size(); i++)
        {
            tinygltf::Image& glTFImage = glTFModel.images[i];
            std::cout << glTFImage.name << "\t" <<glTFImage.uri << std::endl;
        }

        for (auto sid = 0; sid < glTFModel.scenes.size(); sid++)
        {
            const Scene& scene = glTFModel.scenes[sid];
            for (auto i = 0; i < scene.nodes.size(); i++)
            {
                const Node node = glTFModel.nodes[scene.nodes[i]];
                LoadNode(glTFModel, node);
            }
        }
        
        // Register Shader To RenderManager
        world->CreateEntity("");
    }
}