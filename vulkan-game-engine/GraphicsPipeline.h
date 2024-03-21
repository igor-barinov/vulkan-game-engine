#pragma once

#include <vulkan/vulkan.h>

#include <algorithm>
#include <vector>

#include "Shader.h"
#include "SwapChain.h"
#include "DescriptorPool.h"

/*
* Class implementing Vulkan graphics pipeline
*/
class GraphicsPipeline
{
public:

	/*
	* TYPEDEFS
	*/

	using Handle = VkPipeline;



	/*
	* PUBLIC FRIEND METHODS
	*/

	/* @brief Swap implementation for GraphicsPipeline class 
	*/
	friend void swap(GraphicsPipeline& pipelineA, GraphicsPipeline& pipelineB)
	{
		using std::swap;

		swap(pipelineA._layout, pipelineB._layout);
		swap(pipelineA._pipeline, pipelineB._pipeline);
		swap(pipelineA._renderPass, pipelineB._renderPass);
		swap(pipelineA._deviceHandle, pipelineB._deviceHandle);
	}



	/*
	* CTORS / ASSIGNMENT
	*/

	GraphicsPipeline();
	/*
	* @param device Device being used
	* @param swapChain Swap chain being used with this pipeline
	* @param shaders List of shaders to be used
	*/
	GraphicsPipeline(const Device& device, const SwapChain& swapChain, const std::vector<Shader>& shaders, const DescriptorPool& descriptors);
	GraphicsPipeline(const GraphicsPipeline& other);
	GraphicsPipeline(GraphicsPipeline&& other) noexcept;
	GraphicsPipeline& operator=(GraphicsPipeline other);
	~GraphicsPipeline();



	/*
	* PUBLIC CONST METHODS
	*/

	/* @brief Returns the handle to the pipeline
	*/
	inline Handle handle() const { return _pipeline; }

	/* @brief Returns handle to pipeline layout object
	*/
	inline VkPipelineLayout layout_handle() const { return _layout; }

	/* @brief Returns the handle to the render pass
	*/
	inline VkRenderPass render_pass() const { return _renderPass; }

private:

	/*
	* PRIVATE ENUMS
	*/

	enum _ShaderStageType
	{
		VERTEX,
		FRAGMENT,
		NONE
	};



	/*
	* PRIVATE MEMBERS
	*/

	/* Hande to pipeline layout object
	*/
	VkPipelineLayout _layout;

	/* Handle to pipeline
	*/
	VkPipeline _pipeline;

	/* Handle to render pass object
	*/
	VkRenderPass _renderPass;

	/* Handle to device being used
	*/
	VkDevice _deviceHandle;



	/*
	* PRIVATE CONST METHODS
	*/

	/* @brief Fills struct with info necessary for creating a shader stage
	* 
	* @param[out] pCreateInfo The struct to fill
	* @param shader Shader information
	* @param name The stage name
	*/
	void _configure_shader_stage(VkPipelineShaderStageCreateInfo* pCreateInfo, const Shader& shader, const char* name) const;

	/* @brief Fills struct with info necessary for creating the vertex input state
	*/
	void _configure_vertex_input(
		VkPipelineVertexInputStateCreateInfo* pCreateInfo,
		const std::array<VkVertexInputAttributeDescription, 3>& attributeDescriptions,
		VkVertexInputBindingDescription bindingDescription
	) const;

	/* @brief Fills struct with info necessary for creating the input assembly state
	*/
	void _configure_pipeline_input_assembly(VkPipelineInputAssemblyStateCreateInfo* pCreateInfo) const;

	/* @brief Fills struct with info necessary for creating the viewport state
	*/
	void _configure_pipeline_viewport(VkPipelineViewportStateCreateInfo* pCreateInfo) const;

	/* @brief Fills struct with info necessary for creating the rasterization state
	*/
	void _configure_pipeline_rasterization(VkPipelineRasterizationStateCreateInfo* pCreateInfo) const;

	/* @brief Fills struct with info necessary for creating the multisampling state
	*/
	void _configure_pipeline_multisampling(VkPipelineMultisampleStateCreateInfo* pCreateInfo) const;

	/* @brief Fills struct with info necessary for creating the colorblend state
	*/
	void _configure_pipeline_colorblend(VkPipelineColorBlendStateCreateInfo* pStateInfo, VkPipelineColorBlendAttachmentState* pAttachInfo) const;

	/* @brief Fills struct with info necessary for creating the dynamic states
	*/
	void _configure_pipeline_dynamic_state(VkPipelineDynamicStateCreateInfo* pCreateInfo, const std::vector<VkDynamicState>& dynamicStates) const;

	/* @brief Fills struct with info necessary for creating the pipeline layout
	*/
	void _configure_pipeline_layout(VkPipelineLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayout* pSetLayouts) const;

	/* @brief Fills struct with info necessary for creating a color attachment
	*/
	void _configure_color_attachment(VkAttachmentDescription* pCreateInfo, VkAttachmentReference* pRefInfo, VkFormat format, uint32_t index) const;

	/* @brief Fills struct with info necessary for creating a subpass
	*/
	void _configure_subpass(VkSubpassDescription* pCreateInfo, const std::vector<VkAttachmentReference>& attachmentRefs) const;

	/* @brief Fills struct with info necessary for creating a subpass dependency
	*/
	void _configure_subpass_dependency(VkSubpassDependency* pCreateInfo) const;

	/* @brief Fills struct with info necessary for creating a render pass
	*/
	void _configure_render_pass(
		VkRenderPassCreateInfo* pCreateInfo,
		const std::vector<VkAttachmentDescription>& colorAttachments,
		const std::vector<VkSubpassDescription>& subpasses,
		const std::vector<VkSubpassDependency>& subpassDeps
	) const;

	/* @brief Fills struct with info necessary for creating a pipeline
	*/
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