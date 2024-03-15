#include "PNGImage.h"

#include <algorithm>

PNGImage::pixel_bits_t PNGImage::_bits_from_struct(png::rgba_pixel pixelStruct)
{
	pixel_bits_t redBits = ((pixel_bits_t)pixelStruct.red);
	pixel_bits_t greenBits = ((pixel_bits_t)pixelStruct.green) << 8;
	pixel_bits_t blueBits = ((pixel_bits_t)pixelStruct.blue) << 16;
	pixel_bits_t alphaBits = ((pixel_bits_t)pixelStruct.alpha) << 24;

	return redBits | greenBits | blueBits | alphaBits;
}

PNGImage::PNGImage()
	: _pixels({})
{
}

PNGImage::PNGImage(const std::string& filepath)
	: PNGImage()
{
	read(filepath);
}

PNGImage::~PNGImage()
{
}

void PNGImage::read(const std::string& filepath)
{
	_image.read(filepath);
	_load_pixels();
}

void PNGImage::_load_pixels()
{
	auto& buf = _image.get_pixbuf();
	auto numRows = buf.get_height();

	for (size_t i = 0; i < numRows; ++i)
	{
		auto& row = buf[i];
		std::transform(row.begin(), row.end(), std::back_inserter(_pixels), &PNGImage::_bits_from_struct);
	}
}