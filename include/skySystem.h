#pragma once
#include <TFVulkanDevice.h>
#include <memory>
class ObjModel;
class SkySystem
{
public:
    void Init(VkRenderPass renderPass, VkExtent2D rtSize, VkPipelineLayout pipelineLayout);
    void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout); 
    void CleanUp();
private:
    void creatPipeline();
    VkPipeline m_skyRenderingPipeline;
    std::unique_ptr<ObjModel> m_objModel;
    
};