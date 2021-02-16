#pragma once
#include <vulkan/vulkan.hpp>
#include <VulkanCoreHelper.h>
#include <vector>
VkShaderModule CreateShaderModule(VkDevice logicalDevice, const std::vector<char>& code);
VkPipeline CreateGraphicsPipeline(VkDevice logicalDevice, VkPipelineLayout pipelineLayout, VkRenderPass renderPass, PipelineParameter pipelineParameter, const std::vector<char>& vsCode, const std::vector<char>& fsCode);