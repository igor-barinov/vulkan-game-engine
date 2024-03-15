#pragma once

#include <png++/png.hpp>

#include <string>
#include <vector>

class PNGImage
{
public:

	using pixel_bits_t = uint32_t;

	PNGImage();
	PNGImage(const std::string& filepath);
	~PNGImage();

	void read(const std::string& filepath);
	inline pixel_bits_t* data() { return _pixels.data(); }

	inline uint32_t width() const { return _image.get_width(); }
	inline uint32_t height() const { return _image.get_height(); }
	inline constexpr uint32_t channels() const { return _NUM_CHANNELS; }

private:

	static constexpr uint32_t _NUM_CHANNELS = 4;

	

	png::image<png::rgba_pixel> _image;

	std::vector<pixel_bits_t> _pixels;

	void _load_pixels();

	static pixel_bits_t _bits_from_struct(png::rgba_pixel pixelStruct);
	
};

