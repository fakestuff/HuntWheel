#include <Vfs.h>
#include <TFVulkanDevice.h>
#include <VulkanBufferHelper.h>
#include "SimpleMath.h"
#include <string>
#include <vector>
using namespace DirectX::SimpleMath;
using float2 = DirectX::SimpleMath::Vector2;
using float3 = DirectX::SimpleMath::Vector3;
using float4 = DirectX::SimpleMath::Vector4;
using float4x4 = DirectX::SimpleMath::Matrix;
class ObjModel
{
public:
    struct Vertex
    {
        float3 pos;
        float3 normal;
        float3 tangent;
        float2 uv;
    };

    struct GPUVertexBuffer
    {
        VkBuffer buffer;
        VkDeviceMemory memory;
    };
    
    ObjModel(fs::path objPath, TF::TFVulkanDevice tfVulkanDevice);
    void CleanUp();
    std::string name;
    std::vector<Vertex> m_vertexBuffer;
    TF::TFVulkanDevice m_vulkanDevice;
    GPUVertexBuffer m_gpuVertexBuffer;

};