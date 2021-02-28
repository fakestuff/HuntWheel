#include <TerrainSystem.h>
#include <CommonUtl.h>
#include <Vfs.h>
#include <ObjModel.h>
#include <VulkanPipelineHelper.h>

void TerrainSystem::Init(TF::TFVulkanDevice tfVulkanDevice, VkDescriptorPool descriptorPool ,VkRenderPass renderPass, VkExtent2D rtSize,VkPipelineLayout pipelineLayout, VkDescriptorSetLayout descriptorSetLayout)
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
    
    p = Vfs::GetResPath();
    p.append("model");p.append("Terrain");p.append("sampleTerrainColor.png");
    m_baseMap.FromFile(p.generic_string(), &m_vulkanDevice, m_vulkanDevice.graphicsQueue,VK_FILTER_LINEAR,4U,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,TF::TextureUsage::Albedo);
    p = Vfs::GetResPath();
    p.append("model");p.append("Terrain");p.append("sampleTerrainNormal.png");
    m_normalMap.FromFile(p.generic_string(), &m_vulkanDevice, m_vulkanDevice.graphicsQueue,VK_FILTER_LINEAR,4U,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,TF::TextureUsage::Normal);
    {
        VkDescriptorSetAllocateInfo samplerAllocInfo{};
        samplerAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        samplerAllocInfo.descriptorPool = descriptorPool;
        samplerAllocInfo.descriptorSetCount = 1;
        samplerAllocInfo.pSetLayouts = &descriptorSetLayout;
        vkAllocateDescriptorSets(m_vulkanDevice.logicalDevice, &samplerAllocInfo, &m_descriptorSet);
        std::vector<VkWriteDescriptorSet> writeDescriptorSets = 
        {
            WriteDescriptorSet(m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &m_baseMap.m_descriptor),
            WriteDescriptorSet(m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &m_normalMap.m_descriptor),
        };
        vkUpdateDescriptorSets(m_vulkanDevice.logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

    }
}

void TerrainSystem::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
    VkBuffer vertexBuffers[] = {m_objModel->m_gpuVertexBuffer.buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_terrainRenderingPipeline);
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    float4x4 modelMatrix = Matrix::CreateScale(float3(-1,1,1));
    
    modelMatrix *= Matrix::CreateRotationY(3.1415926*0.5f);
    modelMatrix *= Matrix::CreateTranslation(float3(-512,0,512));
    //modelMatrix *= Matrix::CreateTranslation(float3(-512,0,512));
        vkCmdPushConstants(
                            commandBuffer,
                            pipelineLayout,
                            VK_SHADER_STAGE_VERTEX_BIT,
                            0,
                            sizeof(float4x4),
                            &modelMatrix);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &m_descriptorSet, 0, nullptr);
    vkCmdDraw(commandBuffer, m_objModel->m_vertexBuffer.size(),1,0,0);
}

void TerrainSystem::CleanUp()
{
    m_objModel->CleanUp();
    m_baseMap.Destroy();
    vkDestroyPipeline(m_vulkanDevice.logicalDevice, m_terrainRenderingPipeline, nullptr);
}