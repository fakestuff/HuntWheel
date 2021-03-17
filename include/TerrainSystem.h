#pragma once
#include <TFVulkanDevice.h>
#include <memory>
#include <Texture.h>
class ObjModel;
class TerrainSystem
{
public:
    void Init(TF::TFVkGfxDevice tfVulkanDevice, VkDescriptorPool descriptorPool ,VkRenderPass renderPass, VkExtent2D rtSize,VkPipelineLayout pipelineLayout, VkDescriptorSetLayout descriptorSetLayout);
    void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout); 
    void CleanUp();

    VkDescriptorSet m_descriptorSet;
    TF::Texture2D m_baseMap;
    TF::Texture2D m_normalMap;
private:
    void creatPipeline();
    TF::TFVkGfxDevice m_vulkanDevice;
     
    VkPipeline m_terrainRenderingPipeline;
    std::unique_ptr<ObjModel> m_objModel;
    
    
};