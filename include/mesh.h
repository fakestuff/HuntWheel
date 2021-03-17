#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <MathHelper.h>
#include <TFVulkanDevice.h>
namespace TF
{


// Mesh
// vertex buffer
// index buffer
// material


// Mesh Contains data like
// vertices
// indices
// uv
// normal tanget bitangent
// vertex color
enum EVertexAttribute
{
    VertexPosition  = 0,
    VertexNormal    = 1,
    VertexTangent   = 2,
    VertexBiTangent = 3,
    VertexColor     = 4,
    UV0             = 5,
    UV1             = 6,
    UV2             = 7,
    UV3             = 8,
};

struct Vertex
{
        float3 pos;
        float3 normal;
        float3 tangent;
        float2 uv;
};

struct GpuVertexBuffer
{
    VkBuffer buffer;
    VkDeviceMemory memory;
};

struct GpuIndexBuffer
{
    VkBuffer buffer;
    VkDeviceMemory memory;
};

class Mesh 
{
    public:
        static std::unordered_map<std::string, std::shared_ptr<Mesh>> SMeshMap;
        //unsigned int m_vertex_data_mask; currently forcing the following format
        //
        
        TFVkGfxDevice m_tfVulkanDevice;
        std::string m_meshName;
        std::vector<Vertex> m_cpuVertexBuffer;
        std::vector<int> m_cpuIndexBuffer;
        GpuVertexBuffer m_gpuVertexBuffer;
        GpuIndexBuffer m_gpuIndexBuffer;
        bool m_hasIndexBuffer;
        Mesh()
        {
            
        }
        ~Mesh()
        {
            CleanUp();
        }
        
    private:
        void CleanUp()
        {
            // delete vertex buffer
            vkDestroyBuffer(m_tfVulkanDevice.logicalDevice, m_gpuVertexBuffer.buffer, nullptr);
            vkFreeMemory(m_tfVulkanDevice.logicalDevice, m_gpuVertexBuffer.memory, nullptr);
            // delete index buffer
            if (m_hasIndexBuffer)
            {
                vkDestroyBuffer(m_tfVulkanDevice.logicalDevice, m_gpuIndexBuffer.buffer, nullptr);
                vkFreeMemory(m_tfVulkanDevice.logicalDevice, m_gpuIndexBuffer.memory, nullptr);
            }
        }
};

}