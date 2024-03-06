#pragma once

#include <vulkan/vulkan.h>

#include <algorithm>
#include <vector>

#include "Shader.h"
#include "SwapChain.h"

/*
* Class containing Vulkan graphics pipeline methods
*/
class GraphicsPipeline
{
public:

	using Handle = VkPipeline;

	friend void swap(GraphicsPipeline& pipelineA, GraphicsPipeline& pipelineB)
	{
		using std::swap;

		swap(pipelineA._layout, pipelineB._layout);
		swap(pipelineA._pipeline, pipelineB._pipeline);
		swap(pipelineA._renderPass, pipelineB._renderPass);
		swap(pipelineA._deviceHandle, pipelineB._deviceHandle);
	}

	GraphicsPipeline();
	GraphicsPipeline(const VulkanDevice& device, const SwapChain& swapChain, const std::vector<Shader>& shaders);
	GraphicsPipeline(const GraphicsPipeline& other);
	GraphicsPipeline(GraphicsPipeline&& other) noexcept;
	GraphicsPipeline& operator=(GraphicsPipeline other);
	~GraphicsPipeline();

	inline Handle handle() const { return _pipeline; }
	inline VkRenderPass render_pass() const { return _renderPass; }

private:
	enum _ShaderStageType
	{
		VERTEX,
		FRAGMENT,
		NONE
	};

	VkPipelineLayout _layout;
	VkPipeline _pipeline;
	VkRenderPass _renderPass;
	VkDevice _deviceHandle;

	void _configure_shader_stage(VkPipelineShaderStageCreateInfo* pCreateInfo, const Shader& shader, const char* name) const;
	void _configure_vertex_input(VkPipelineVertexInputStateCreateInfo* pCreateInfo) const;
	void _configure_pipeline_input(VkPipelineInputAssemblyStateCreateInfo* pCreateInfo) const;
	void _configure_pipeline_viewport(VkPipelineViewportStateCreateInfo* pCreateInfo) const;
	void _configure_pipeline_rasterization(VkPipelineRasterizationStateCreateInfo* pCreateInfo) const;
	void _configure_pipeline_multisampling(VkPipelineMultisampleStateCreateInfo* pCreateInfo) const;
	void _configure_pipeline_colorblend(VkPipelineColorBlendStateCreateInfo* pStateInfo, VkPipelineColorBlendAttachmentState* pAttachInfo) const;
	void _configure_pipeline_dynamic_state(VkPipelineDynamicStateCreateInfo* pCreateInfo, const std::vector<VkDynamicState>& dynamicStates) const;
	void _configure_pipeline_layout(VkPipelineLayoutCreateInfo* pCreateInfo) const;
	void _configure_color_attachment(VkAttachmentDescription* pCreateInfo, VkAttachmentReference* pRefInfo, VkFormat format, uint32_t index) const;
	void _configure_subpass(VkSubpassDescription* pCreateInfo, const std::vector<VkAttachmentReference>& attachmentRefs) const;
	void _configure_subpass_dependency(VkSubpassDependency* pCreateInfo) const;
	void _configure_render_pass(
		VkRenderPassCreateInfo* pCreateInfo,
		const std::vector<VkAttachmentDescription>& colorAttachments,
		const std::vector<VkSubpassDescription>& subpasses,
		const std::vector<VkSubpassDependency>& subpassDeps
	) const;
	void _configure_pipeline(
		VkGraphicsPipelineCreateInfo* pCreateInfo,
		const std::vector<VkPipelineShaderStageCreateInfo>& shaderStages,
		VkPipelineVertexInputStateCreateInfo* pVertexInput,
		VkPipelineInputAssemblyStateCreateInfo* pPipelineInput,
		VkPipelineViewportStateCreateInfo* pPipelineViewport,
		VkPipelineRasterizationStateCreateInfo* pRasterization,
		VkPipelineMultisampleStateCreateInfo* pMultisampling,
		VkPipelineColorBlendStateCreateInfo* pColorblend,
		VkPipelineDynamicStateCreateInfo* pDynamicState
	) const;
};