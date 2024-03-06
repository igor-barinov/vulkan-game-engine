#include "GraphicsPipeline.h"

GraphicsPipeline::GraphicsPipeline()
	: _layout(VK_NULL_HANDLE),
	_pipeline(VK_NULL_HANDLE),
	_renderPass(VK_NULL_HANDLE),
	_deviceHandle(VK_NULL_HANDLE)
{
}

GraphicsPipeline::GraphicsPipeline(const VulkanDevice& device, const SwapChain& swapChain, const std::vector<Shader>& shaders)
	: _layout(VK_NULL_HANDLE),
	_pipeline(VK_NULL_HANDLE),
	_renderPass(VK_NULL_HANDLE),
	_deviceHandle(device.handle())
{
	// Create render pass
	std::vector<VkAttachmentDescription> colorAttachments(1);
	std::vector<VkAttachmentReference> attachmentRefs(1);
	for (size_t i = 0; i < colorAttachments.size(); ++i)
	{
		_configure_color_attachment(&colorAttachments[i], &attachmentRefs[i], swapChain.surface_image_format(), i);
	}

	std::vector<VkSubpassDescription> subpasses(1);
	_configure_subpass(&subpasses[0], attachmentRefs);

	std::vector<VkSubpassDependency> subpassDeps(1);
	_configure_subpass_dependency(&subpassDeps[0]);

	VkRenderPassCreateInfo renderPassInfo;
	_configure_render_pass(&renderPassInfo, colorAttachments, subpasses, subpassDeps);

	if (vkCreateRenderPass(_deviceHandle, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass");
	}


	// Create pipeline
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages(shaders.size());
	for (size_t i = 0; i < shaderStages.size(); ++i)
	{
		_configure_shader_stage(&shaderStages[i], shaders[i], "main");
	}

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	_configure_vertex_input(&vertexInputInfo);

	VkPipelineInputAssemblyStateCreateInfo pipelineInput{};
	_configure_pipeline_input(&pipelineInput);

	VkPipelineViewportStateCreateInfo pipelineViewport{};
	_configure_pipeline_viewport(&pipelineViewport);

	VkPipelineRasterizationStateCreateInfo rasterization{};
	_configure_pipeline_rasterization(&rasterization);

	VkPipelineMultisampleStateCreateInfo multisampling{};
	_configure_pipeline_multisampling(&multisampling);

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	VkPipelineColorBlendStateCreateInfo colorBlend{};
	_configure_pipeline_colorblend(&colorBlend, &colorBlendAttachment);

	VkPipelineDynamicStateCreateInfo dynamicStateInfo;
	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	_configure_pipeline_dynamic_state(&dynamicStateInfo, dynamicStates);

	VkPipelineLayoutCreateInfo layoutInfo;
	_configure_pipeline_layout(&layoutInfo);

	if (vkCreatePipelineLayout(_deviceHandle, &layoutInfo, nullptr, &_layout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	_configure_pipeline(&pipelineInfo, shaderStages, &vertexInputInfo, &pipelineInput, &pipelineViewport, &rasterization, &multisampling, &colorBlend, &dynamicStateInfo);

	if (vkCreateGraphicsPipelines(_deviceHandle, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_pipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline");
	}
}

GraphicsPipeline::GraphicsPipeline(const GraphicsPipeline& other)
	: _layout(other._layout),
	_pipeline(other._pipeline),
	_renderPass(other._renderPass),
	_deviceHandle(other._deviceHandle)
{
}
GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& other) noexcept
	: GraphicsPipeline()
{
	swap(*this, other);
}
GraphicsPipeline& GraphicsPipeline::operator=(GraphicsPipeline other)
{
	swap(*this, other);
	return *this;
}

GraphicsPipeline::~GraphicsPipeline()
{
	if (_pipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(_deviceHandle, _pipeline, nullptr);
		vkDestroyPipelineLayout(_deviceHandle, _layout, nullptr);
		vkDestroyRenderPass(_deviceHandle, _renderPass, nullptr);
	}
}

void GraphicsPipeline::_configure_shader_stage(VkPipelineShaderStageCreateInfo* pCreateInfo, const Shader& shader, const char* name) const
{
	memset(pCreateInfo, 0, sizeof(VkPipelineShaderStageCreateInfo));

	VkShaderStageFlagBits shaderFlag{};

	switch (shader.shader_type())
	{
	case Shader::FRAGMENT:
		shaderFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
		break;
	case Shader::VERTEX:
		shaderFlag = VK_SHADER_STAGE_VERTEX_BIT;
		break;
	case Shader::COMPUTE:
		shaderFlag = VK_SHADER_STAGE_COMPUTE_BIT;
		break;
	}

	pCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pCreateInfo->stage = shaderFlag;
	pCreateInfo->module = shader.handle();
	pCreateInfo->pName = name;
}

void GraphicsPipeline::_configure_vertex_input(VkPipelineVertexInputStateCreateInfo* pCreateInfo) const
{
	memset(pCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pCreateInfo->vertexBindingDescriptionCount = 0;
	pCreateInfo->vertexAttributeDescriptionCount = 0;
}

void GraphicsPipeline::_configure_pipeline_input(VkPipelineInputAssemblyStateCreateInfo* pCreateInfo) const
{
	memset(pCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pCreateInfo->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pCreateInfo->primitiveRestartEnable = VK_FALSE;
}

void GraphicsPipeline::_configure_pipeline_viewport(VkPipelineViewportStateCreateInfo* pCreateInfo) const
{
	memset(pCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pCreateInfo->viewportCount = 1;
	pCreateInfo->scissorCount = 1;
}

void GraphicsPipeline::_configure_pipeline_rasterization(VkPipelineRasterizationStateCreateInfo* pCreateInfo) const
{
	memset(pCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pCreateInfo->depthClampEnable = VK_FALSE;
	pCreateInfo->rasterizerDiscardEnable = VK_FALSE;
	pCreateInfo->polygonMode = VK_POLYGON_MODE_FILL;
	pCreateInfo->lineWidth = 1.0f;
	pCreateInfo->cullMode = VK_CULL_MODE_BACK_BIT;
	pCreateInfo->frontFace = VK_FRONT_FACE_CLOCKWISE;
	pCreateInfo->depthBiasEnable = VK_FALSE;
}

void GraphicsPipeline::_configure_pipeline_multisampling(VkPipelineMultisampleStateCreateInfo* pCreateInfo) const
{
	memset(pCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pCreateInfo->sampleShadingEnable = VK_FALSE;
	pCreateInfo->rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
}

void GraphicsPipeline::_configure_pipeline_colorblend(VkPipelineColorBlendStateCreateInfo* pStateInfo, VkPipelineColorBlendAttachmentState* pAttachInfo) const
{
	memset(pAttachInfo, 0, sizeof(VkPipelineColorBlendAttachmentState));
	pAttachInfo->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	pAttachInfo->blendEnable = VK_FALSE;

	memset(pStateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
	pStateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pStateInfo->logicOpEnable = VK_FALSE;
	pStateInfo->logicOp = VK_LOGIC_OP_COPY;
	pStateInfo->attachmentCount = 1;
	pStateInfo->pAttachments = pAttachInfo;
	pStateInfo->blendConstants[0] = 0.0f;
	pStateInfo->blendConstants[1] = 0.0f;
	pStateInfo->blendConstants[2] = 0.0f;
	pStateInfo->blendConstants[3] = 0.0f;
}

void GraphicsPipeline::_configure_pipeline_dynamic_state(VkPipelineDynamicStateCreateInfo* pCreateInfo, const std::vector<VkDynamicState>& dynamicStates) const
{
	memset(pCreateInfo, 0, sizeof(VkPipelineDynamicStateCreateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	pCreateInfo->dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	pCreateInfo->pDynamicStates = dynamicStates.data();
}

void GraphicsPipeline::_configure_pipeline_layout(VkPipelineLayoutCreateInfo* pCreateInfo) const
{
	memset(pCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pCreateInfo->setLayoutCount = 0;
	pCreateInfo->pushConstantRangeCount = 0;
}

void GraphicsPipeline::_configure_color_attachment(VkAttachmentDescription* pCreateInfo, VkAttachmentReference* pRefInfo, VkFormat format, uint32_t index) const
{
	memset(pCreateInfo, 0, sizeof(VkAttachmentDescription));
	pCreateInfo->format = format;
	pCreateInfo->samples = VK_SAMPLE_COUNT_1_BIT;
	pCreateInfo->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	pCreateInfo->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	pCreateInfo->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	pCreateInfo->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	pCreateInfo->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	pCreateInfo->finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	memset(pRefInfo, 0, sizeof(VkAttachmentReference));
	pRefInfo->attachment = index;
	pRefInfo->layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}

void GraphicsPipeline::_configure_subpass(VkSubpassDescription* pCreateInfo, const std::vector<VkAttachmentReference>& attachmentRefs) const
{
	memset(pCreateInfo, 0, sizeof(VkSubpassDescription));
	pCreateInfo->pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	pCreateInfo->colorAttachmentCount = attachmentRefs.size();
	pCreateInfo->pColorAttachments = attachmentRefs.data();
}

void GraphicsPipeline::_configure_subpass_dependency(VkSubpassDependency* pCreateInfo) const
{
	memset(pCreateInfo, 0, sizeof(VkSubpassDependency));
	pCreateInfo->srcSubpass = VK_SUBPASS_EXTERNAL;
	pCreateInfo->dstSubpass = 0;
	pCreateInfo->srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	pCreateInfo->srcAccessMask = 0;
	pCreateInfo->dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	pCreateInfo->dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
}

void GraphicsPipeline::_configure_render_pass(
	VkRenderPassCreateInfo* pCreateInfo,
	const std::vector<VkAttachmentDescription>& colorAttachments,
	const std::vector<VkSubpassDescription>& subpasses,
	const std::vector<VkSubpassDependency>& subpassDeps
) const
{
	memset(pCreateInfo, 0, sizeof(VkRenderPassCreateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	pCreateInfo->attachmentCount = colorAttachments.size();
	pCreateInfo->pAttachments = colorAttachments.data();
	pCreateInfo->subpassCount = subpasses.size();
	pCreateInfo->pSubpasses = subpasses.data();
	pCreateInfo->dependencyCount = subpassDeps.size();
	pCreateInfo->pDependencies = subpassDeps.data();
}

void GraphicsPipeline::_configure_pipeline(
	VkGraphicsPipelineCreateInfo* pCreateInfo,
	const std::vector<VkPipelineShaderStageCreateInfo>& shaderStages,
	VkPipelineVertexInputStateCreateInfo* pVertexInput,
	VkPipelineInputAssemblyStateCreateInfo* pPipelineInput,
	VkPipelineViewportStateCreateInfo* pPipelineViewport,
	VkPipelineRasterizationStateCreateInfo* pRasterization,
	VkPipelineMultisampleStateCreateInfo* pMultisampling,
	VkPipelineColorBlendStateCreateInfo* pColorblend,
	VkPipelineDynamicStateCreateInfo* pDynamicState
) const
{
	memset(pCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
	pCreateInfo->sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pCreateInfo->stageCount = shaderStages.size();
	pCreateInfo->pStages = shaderStages.data();
	pCreateInfo->pVertexInputState = pVertexInput;
	pCreateInfo->pInputAssemblyState = pPipelineInput;
	pCreateInfo->pViewportState = pPipelineViewport;
	pCreateInfo->pRasterizationState = pRasterization;
	pCreateInfo->pMultisampleState = pMultisampling;
	pCreateInfo->pColorBlendState = pColorblend;
	pCreateInfo->pDynamicState = pDynamicState;
	pCreateInfo->layout = _layout;
	pCreateInfo->renderPass = _renderPass;
	pCreateInfo->subpass = 0;
	pCreateInfo->basePipelineHandle = VK_NULL_HANDLE;
}