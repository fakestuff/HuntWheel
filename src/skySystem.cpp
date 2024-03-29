#include <SkySystem.h>
#include <CommonUtl.h>
#include <Vfs.h>
#include <ObjModel.h>
#include <VulkanPipelineHelper.h>

void SkySystem::Init( VkRenderPass renderPass, VkExtent2D rtSize,VkPipelineLayout pipelineLayout)
{
    auto p = Vfs::GetResPath();
    p.append("model");
    p.append("Sphere");
    p.append("sphere.obj");
    m_objModel = std::make_unique<ObjModel>(p);
    
    auto shaderPath = Vfs::GetShaderPath();
    auto vertShaderCode = ReadFile(Vfs::ConcatPath(shaderPath, "sky.vert.spv").generic_string());
    auto fragShaderCode = ReadFile(Vfs::ConcatPath(shaderPath, "sky.frag.spv").generic_string());
    
    PipelineParameter pipelineParameter = PipelineParameter(rtSize);

    m_skyRenderingPipeline = CreateGraphicsPipeline(TF::TFVkGfxDevice::Get().logicalDevice, pipelineLayout, renderPass, pipelineParameter, vertShaderCode, fragShaderCode);  

}

void SkySystem::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
    VkBuffer vertexBuffers[] = {m_objModel->m_gpuVertexBuffer.buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_skyRenderingPipeline);
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    float4x4 modelMatrix = Matrix::CreateScale(5000.0f);
        vkCmdPushConstants(
                            commandBuffer,
                            pipelineLayout,
                            VK_SHADER_STAGE_VERTEX_BIT,
                            0,
                            sizeof(float4x4),
                            &modelMatrix);
    vkCmdDraw(commandBuffer, m_objModel->m_vertexBuffer.size(),1,0,0);
}

void SkySystem::CleanUp()
{
    m_objModel->CleanUp();
    vkDestroyPipeline(TF::TFVkGfxDevice::Get().logicalDevice, m_skyRenderingPipeline, nullptr);
}

void SkySystem::creatPipeline()
{
    // auto shaderPath = Vfs::GetShaderPath();
    // auto vertShaderCode = ReadFile(Vfs::ConcatPath(shaderPath, "Sky.vert.spv").generic_string());
    // auto fragShaderCode = ReadFile(Vfs::ConcatPath(shaderPath, "Sky.frag.spv").generic_string());
    // VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    // VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    // VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    // vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    // vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    // vertShaderStageInfo.module = vertShaderModule;
    // vertShaderStageInfo.pName = "main";
    // vertShaderStageInfo.pSpecializationInfo = nullptr;

    // VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    // fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    // fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    // fragShaderStageInfo.module = fragShaderModule;
    // fragShaderStageInfo.pName = "main";
    // fragShaderStageInfo.pSpecializationInfo = nullptr;
    // VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};


    // auto bindingDescription = Vertex::getBindingDescription();
    // auto attributeDescriptions = Vertex::getAttributeDescriptions();

    // VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    // vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    // vertexInputInfo.vertexBindingDescriptionCount = 1;
    // vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    // vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    // vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    // inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    // inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    // inputAssembly.primitiveRestartEnable = VK_FALSE;

    // VkViewport viewport{};
    // viewport.x = 0.0f;
    // viewport.y = 0.0f;
    // viewport.width = (float) m_swapChainExtent.width;
    // viewport.height = (float) m_swapChainExtent.height;
    // viewport.minDepth = 0.0f;
    // viewport.maxDepth = 1.0f;

    // VkRect2D scissor{};
    // scissor.offset = {0, 0};
    // scissor.extent = m_swapChainExtent;

    // VkPipelineViewportStateCreateInfo viewportState{};
    // viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    // viewportState.viewportCount = 1;
    // viewportState.pViewports = &viewport;
    // viewportState.scissorCount = 1;
    // viewportState.pScissors = &scissor;

    // VkPipelineRasterizationStateCreateInfo rasterizer{};
    // rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    // rasterizer.depthClampEnable = VK_FALSE;
    // rasterizer.rasterizerDiscardEnable = VK_FALSE;
    // rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    // rasterizer.lineWidth = 1.0f;
    // rasterizer.cullMode = VK_CULL_MODE_NONE;//VK_CULL_MODE_BACK_BIT;
    // rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    // rasterizer.depthBiasEnable = VK_FALSE;
    // rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    // rasterizer.depthBiasClamp = 0.0f; // Optional
    // rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    // VkPipelineDepthStencilStateCreateInfo depthStencil{};
    // depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    // depthStencil.depthTestEnable = VK_TRUE;
    // depthStencil.depthWriteEnable = VK_TRUE;
    // depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    // depthStencil.depthBoundsTestEnable = VK_FALSE;
    // depthStencil.minDepthBounds = 0.0f; // Optional
    // depthStencil.maxDepthBounds = 1.0f; // Optional
    // depthStencil.stencilTestEnable = VK_FALSE;
    // depthStencil.front = {}; // Optional
    // depthStencil.back = {}; // Optional

    // VkPipelineMultisampleStateCreateInfo multisampling{};
    // multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    // multisampling.sampleShadingEnable = VK_FALSE;
    // multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    // multisampling.minSampleShading = 1.0f; // Optional
    // multisampling.pSampleMask = nullptr; // Optional
    // multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    // multisampling.alphaToOneEnable = VK_FALSE; // Optional

    // VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    // colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    // colorBlendAttachment.blendEnable = VK_FALSE;
    // colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    // colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    // colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    // colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    // colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    // colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    // VkPipelineColorBlendStateCreateInfo colorBlending{};
    // colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    // colorBlending.logicOpEnable = VK_FALSE;
    // colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    // colorBlending.attachmentCount = 1;
    // colorBlending.pAttachments = &colorBlendAttachment;
    // colorBlending.blendConstants[0] = 0.0f; // Optional
    // colorBlending.blendConstants[1] = 0.0f; // Optional
    // colorBlending.blendConstants[2] = 0.0f; // Optional
    // colorBlending.blendConstants[3] = 0.0f; // Optional

    // VkDynamicState dynamicStates[] = {
    //     VK_DYNAMIC_STATE_VIEWPORT,
    //     VK_DYNAMIC_STATE_LINE_WIDTH
    // };

    // VkPipelineDynamicStateCreateInfo dynamicState{};
    // dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    // dynamicState.dynamicStateCount = 2;
    // dynamicState.pDynamicStates = dynamicStates;

    // // Define the push constant range used by the pipeline layout
    // // Note that the spec only requires a minimum of 128 bytes, so for passing larger blocks of data you'd use UBOs or SSBOs
    // VkPushConstantRange pushConstantRange{};
    // pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    // pushConstantRange.offset = 0;
    // pushConstantRange.size = sizeof(float4x4);


    // std::array<VkDescriptorSetLayout, 2> setLayouts = { m_descriptorSetLayoutMatrices, m_descriptorSetLayoutTextures };
    // VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    // pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // pipelineLayoutInfo.setLayoutCount = 2; // Optional
    // pipelineLayoutInfo.pSetLayouts = setLayouts.data(); // Optional
    // pipelineLayoutInfo.pushConstantRangeCount = 1; // Optional
    // pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange; // Optional

    // if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
    // {
    //     throw std::runtime_error("failed to create pipeline layout!");
    // }

    // VkGraphicsPipelineCreateInfo  pipelineInfo{};
    // pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    // pipelineInfo.stageCount = 2;
    // pipelineInfo.pStages = shaderStages;
    // pipelineInfo.pVertexInputState = &vertexInputInfo;
    // pipelineInfo.pInputAssemblyState = &inputAssembly;
    // pipelineInfo.pViewportState = &viewportState;
    // pipelineInfo.pRasterizationState = &rasterizer;
    // pipelineInfo.pMultisampleState = &multisampling;
    // pipelineInfo.pDepthStencilState = &depthStencil;
    // pipelineInfo.pColorBlendState = &colorBlending;
    // pipelineInfo.pDynamicState = nullptr;
    // pipelineInfo.layout = m_pipelineLayout;
    // pipelineInfo.renderPass = m_renderPass;
    // pipelineInfo.subpass = 0;
    // pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    // pipelineInfo.basePipelineIndex = -1;

    // if (vkCreateGraphicsPipelines(m_vulkanDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) 
    // {
    //     throw std::runtime_error("failed to create graphics pipeline!");
    // }

    // vkDestroyShaderModule(m_vulkanDevice.logicalDevice, fragShaderModule, nullptr);
    // vkDestroyShaderModule(m_vulkanDevice.logicalDevice, vertShaderModule, nullptr);
}