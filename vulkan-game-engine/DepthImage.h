#pragma once

#include <algorithm>

#include "Image.h"
#include "SwapChain.h"

class DepthImage : public Image
{
public:

	DepthImage();
	DepthImage(const Device& device, const SwapChain& swapChain);
	DepthImage(const DepthImage& other);
	DepthImage(DepthImage&& other) noexcept;
	DepthImage& operator=(DepthImage other);
	~DepthImage();
};

