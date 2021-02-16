#pragma once
#include <TFVulkanDevice.h>
#include <memory>
class ObjModel;
class SkySystem
{
public:
    void Init(TF::TFVulkanDevice tfVulkanDevice, VkRenderPass renderPass, VkExtent2D rtSize, VkPipelineLayout pipelineLayout);
    void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout); 
    void CleanUp();
private:
    void creatPipeline();
    TF::TFVulkanDevice m_vulkanDevice;
    VkPipeline m_skyRenderingPipeline;
    std::unique_ptr<ObjModel> m_objModel;
    
};