#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <MathHelper.h>
#include <TFVulkanDevice.h>
#include <VulkanHelper.h>
#include <VulkanBufferHelper.h>
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

struct GpuBuffer
{
    VkBuffer buffer;
    VkDeviceMemory memory;
};

class Mesh 
{
    public:
        inline static std::unordered_map<std::string, std::shared_ptr<Mesh>> SMeshMap;
        inline static bool GetMesh(std::string name, std::shared_ptr<Mesh>& meshPtr)
        {
            if (SMeshMap.find(name) == SMeshMap.end())
            {
                return false;
            }
            else
            {
                meshPtr = SMeshMap[name];
                return true;
            }
        }

        inline static std::shared_ptr<Mesh> GetOrCreateMesh(std::string name, const std::vector<Vertex>& vertexBuffer)
        {
            std::shared_ptr<Mesh> meshPtr;
            if (!GetMesh(name, meshPtr))
            {
                meshPtr = std::make_shared<Mesh>(name, vertexBuffer);
            }
            return meshPtr;
        }

        inline static std::shared_ptr<Mesh> GetOrCreateMesh(std::string name, const std::vector<Vertex>& vertexBuffer, const std::vector<uint32_t>& indexBuffer)
        {
            std::shared_ptr<Mesh> meshPtr;
            if (!GetMesh(name, meshPtr))
            {
                meshPtr = std::make_shared<Mesh>(name, vertexBuffer, indexBuffer);
            }
            return meshPtr;
        }
        //unsigned int m_vertex_data_mask; currently forcing the following format
        //
        
        std::string m_meshName;
        std::vector<Vertex> m_cpuVertexBuffer;
        std::vector<uint32_t> m_cpuIndexBuffer;
        GpuBuffer m_gpuVertexBuffer;
        GpuBuffer m_gpuIndexBuffer;
        bool m_hasIndexBuffer;
        Mesh(std::string name, const std::vector<Vertex>& vertexBuffer, const std::vector<uint32_t>& indexBuffer)
        {
            m_meshName = name;
            m_hasIndexBuffer = true;
            m_cpuVertexBuffer = vertexBuffer;
            m_cpuIndexBuffer = indexBuffer;
            UploadingBuffer(vertexBuffer, m_gpuVertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
            UploadingBuffer(indexBuffer, m_gpuIndexBuffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        }

        Mesh(std::string name, const std::vector<Vertex>& vertexBuffer)
        {
            m_meshName = name;
            m_cpuVertexBuffer = vertexBuffer;
            UploadingBuffer(vertexBuffer, m_gpuVertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        }
        
        ~Mesh()
        {
            SMeshMap.erase(m_meshName);
            CleanUp();
        }
        
    private:
        template <typename T>
        void UploadingBuffer(const std::vector<T>& buffer, GpuBuffer& gpuBuffer,VkBufferUsageFlags bufferUsage)
        {
            size_t bufferSize = buffer.size() * sizeof(T);
            GpuBuffer stagingBuffer;
            CreateBuffer(TFVkGfxDevice::Get().physicsDevice, TFVkGfxDevice::Get().logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer.buffer, stagingBuffer.memory);
            
            void* data;
            vkMapMemory(TFVkGfxDevice::Get().logicalDevice, stagingBuffer.memory, 0, bufferSize, 0, &data);
            memcpy(data, buffer.data(), bufferSize);
            vkUnmapMemory(TFVkGfxDevice::Get().logicalDevice, stagingBuffer.memory);

            CreateBuffer(TFVkGfxDevice::Get().physicsDevice, TFVkGfxDevice::Get().logicalDevice, bufferSize, bufferUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, gpuBuffer.buffer, gpuBuffer.memory);
            CopyBuffer(TFVkGfxDevice::Get().logicalDevice, TFVkGfxDevice::Get().graphicsQueue, TFVkGfxDevice::Get().cmdPool, stagingBuffer.buffer, gpuBuffer.buffer, bufferSize);
            
            vkDestroyBuffer(TFVkGfxDevice::Get().logicalDevice, stagingBuffer.buffer, nullptr);
            vkFreeMemory(TFVkGfxDevice::Get().logicalDevice, stagingBuffer.memory, nullptr);
        }
        
        void CleanUp()
        {
            // delete vertex buffer
            vkDestroyBuffer(TFVkGfxDevice::Get().logicalDevice, m_gpuVertexBuffer.buffer, nullptr);
            vkFreeMemory(TFVkGfxDevice::Get().logicalDevice, m_gpuVertexBuffer.memory, nullptr);
            // delete index buffer
            if (m_hasIndexBuffer)
            {
                vkDestroyBuffer(TFVkGfxDevice::Get().logicalDevice, m_gpuIndexBuffer.buffer, nullptr);
                vkFreeMemory(TFVkGfxDevice::Get().logicalDevice, m_gpuIndexBuffer.memory, nullptr);
            }
        }
};

}