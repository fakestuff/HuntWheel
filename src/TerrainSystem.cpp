#include <TerrainSystem.h>
#include <CommonUtl.h>
#include <Vfs.h>
#include <ObjModel.h>
#include <VulkanPipelineHelper.h>

void TerrainSystem::Init(TF::TFVulkanDevice tfVulkanDevice, VkRenderPass renderPass, VkExtent2D rtSize,VkPipelineLayout pipelineLayout)
{
    m_vulkanDevice = tfVulkanDevice;
    auto p = Vfs::GetResPath();
    p.append("model");
    p.append("Terrain");
    p.append("sampleTerrain.obj");
    m_objModel = std::make_unique<ObjModel>(p, tfVulkanDevice);
    
    auto shaderPath = Vfs::GetShaderPath();
    auto vertShaderCode = ReadFile(Vfs::ConcatPath(shaderPath, "Terrain.vert.spv").generic_string());
    auto fragShaderCode = ReadFile(Vfs::ConcatPath(shaderPath, "Terrain.frag.spv").generic_string());
    
    PipelineParameter pipelineParameter = PipelineParameter(rtSize);

    m_terrainRenderingPipeline = CreateGraphicsPipeline(tfVulkanDevice.logicalDevice, pipelineLayout, renderPass, pipelineParameter, vertShaderCode, fragShaderCode);  

}

void TerrainSystem::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
    VkBuffer vertexBuffers[] = {m_objModel->m_gpuVertexBuffer.buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_terrainRenderingPipeline);
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    float4x4 modelMatrix = Matrix::CreateTranslation(float3(1024,0,-1024));
        vkCmdPushConstants(
                            commandBuffer,
                            pipelineLayout,
                            VK_SHADER_STAGE_VERTEX_BIT,
                            0,
                            sizeof(float4x4),
                            &modelMatrix);
    vkCmdDraw(commandBuffer, m_objModel->m_vertexBuffer.size(),1,0,0);
}

void TerrainSystem::CleanUp()
{
    m_objModel->CleanUp();
    vkDestroyPipeline(m_vulkanDevice.logicalDevice, m_terrainRenderingPipeline, nullptr);
}