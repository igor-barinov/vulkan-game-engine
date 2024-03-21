#pragma once

#include <glm/glm.hpp>

class UBO
{
public:

	using model_mat_t = glm::mat4;
	using view_mat_t = glm::mat4;
	using proj_mat_t = glm::mat4;

	UBO();
	UBO(const model_mat_t& model, const view_mat_t& view, const proj_mat_t& projection);
	~UBO();

	void update(const model_mat_t& model, const view_mat_t& view, const proj_mat_t& projection);

	inline static constexpr size_t object_size() { return sizeof(UBO); }

private:
	alignas(16) model_mat_t _model;
	alignas(16) view_mat_t _view;
	alignas(16) proj_mat_t _projection;
};