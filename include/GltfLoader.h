// Define these only in *one* .cc file.
#pragma once
#include <vector>
#include <string>

#include <vulkan/vulkan.hpp>

#include "SimpleMath.h"
#include "tiny_gltf.h"
#include <TFVulkanDevice.h>
#include <Texture.h>
using namespace DirectX::SimpleMath;
using float2 = DirectX::SimpleMath::Vector2;
using float3 = DirectX::SimpleMath::Vector3;
using float4 = DirectX::SimpleMath::Vector4;
using float4x4 = DirectX::SimpleMath::Matrix;
void TestLoadModel();

class GltfModel
{
public:
    

    struct Vertex
    {
        float3 pos;
        float3 normal;
        float3 tangent;
        float2 uv;
    };

    struct PrimitiveVertexBuffer
    {
        VkBuffer buffer;
        VkDeviceMemory memory;
    };

    struct PrimitiveIndexBuffer
    {
        int count;
        VkBuffer buffer;
        VkDeviceMemory memory;
    };

    struct Primitive
    {
        uint32_t firstIndex;
        uint32_t indexCount;
        int32_t materialIndex;
    };

    struct Mesh
    {
        std::vector<Primitive> primitives;
    };

    // A glTF material stores information in e.g. the texture that is attached to it and colors
	struct Material {
		//glm::vec4 baseColorFactor = glm::vec4(1.0f);
		uint32_t baseColorTextureIndex;
        uint32_t normalTextureIndex;
        uint32_t metallicRoughnessTextureIndex;
        VkDescriptorSet descriptorSet;
	};

    // A glTF texture stores a reference to the image and a sampler
	// In this sample, we are only interested in the image
	struct Texture {
		int32_t imageIndex;
	};

    struct Image
    {
        TF::Texture2D texture;
        //VkDescriptorSet descriptorSet;
    };

    struct SceneNode
    {
        SceneNode* parent;
        std::vector<SceneNode> children;
        Mesh mesh;
        Matrix matrix; // model matrix
    };


    GltfModel(TF::TFVulkanDevice vulkanDevice, std::string path, float scale);
    ~GltfModel();
    void LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& modelAsset, 
                    SceneNode* parent,std::vector<Vertex>& vertexBuffer, std::vector<uint32_t>& indexBuffer);
    void LoadImages(tinygltf::Model& modelAsset);
    void LoadTextures(tinygltf::Model& modelAsset);
    void LoadMaterials(tinygltf::Model& modelAsset);
    void UploadModel(const std::vector<Vertex>& uploadingVertexBuffer,const std::vector<uint32_t>& uploadingIndexBuffer);
    void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
    void DrawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, const SceneNode& node);

    
    
    TF::TFVulkanDevice m_vulkanDevice;
    std::vector<SceneNode> m_nodes;
    std::vector<Image> m_images;
    std::vector<Texture> m_textures; // map textures
    std::vector<Material> m_materials;
    PrimitiveVertexBuffer m_vertexBuffer;
    PrimitiveIndexBuffer m_indexBuffer;
    float m_scale;

    
};
