#pragma once
#include <MathHelper.h>

struct Vertex {
    float3 pos;
    float3 normal;
    float3 tangent;
    float2 texCoord;
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, normal);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, tangent);

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
}
};

struct PipelineParameter
{
public:
    VkExtent2D RtSize;
    VkCullModeFlags CullMode;
    VkBool32 DepthTest;
    VkBool32 DepthWrite;
    VkCompareOp DepthCompareOp;
    VkBool32 BlendEnable;
    VkBlendFactor SrcColorBlendFactor;
    VkBlendFactor DstColorBlendFactor;
    VkBlendOp ColorBlendOp;
    VkBlendFactor SrcAlphaBlendFactor;
    VkBlendFactor DstAlphaBlendFactor;
    VkBlendOp AlphaBlendOp;

public:
    PipelineParameter(VkExtent2D rtSize)
    {
        RtSize = rtSize;
        CullMode = VK_CULL_MODE_NONE;
        DepthTest = VK_TRUE;
        DepthWrite = VK_TRUE;
        DepthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        BlendEnable = VK_FALSE;
        SrcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
        DstColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
        ColorBlendOp = VK_BLEND_OP_ADD;
        SrcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        DstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
        AlphaBlendOp = VK_BLEND_OP_ADD;
    }

    
};

inline VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool)
{
    VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
}

inline void EndSingleTimeCommands(VkDevice device, VkQueue queue, VkCommandPool commandPool,VkCommandBuffer commandBuffer) 
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}