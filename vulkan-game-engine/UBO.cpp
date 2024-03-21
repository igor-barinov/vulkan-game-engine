#include "UBO.h"

UBO::UBO()
	: _model({}),
	_view({}),
	_projection({})
{
}

UBO::UBO(const model_mat_t& model, const view_mat_t& view, const proj_mat_t& projection)
	: _model(model),
	_view(view),
	_projection(projection)
{

}

UBO::~UBO()
{
}

void UBO::update(const model_mat_t& model, const view_mat_t& view, const proj_mat_t& projection)
{
	_model = model;
	_view = view;
	_projection = projection;
}