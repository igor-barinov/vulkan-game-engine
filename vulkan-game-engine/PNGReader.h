#pragma once

#include <png.h>

#include <string>
#include <memory>
#include <vector>

class PNGReader
{
public:

	PNGReader();
	PNGReader(const std::string& filepath);
	~PNGReader();

	void read(const std::string& filepath);

private:

	using byte_t = png_byte;

	static constexpr size_t _HEADER_BYTES = 8;

	byte_t** _rows;
	uint32_t _imgWidth;
	uint32_t _imgHeight;
	byte_t _colorType;
	byte_t _bitDepth;
	
	void _alloc_rows(const png_structp pPng, const png_infop pInfo);
	void _free_rows();
	void _open_file(FILE** ppFile, const char* filepath) const;
	void _create_png_structs(png_structpp ppPng, png_infopp ppInfo, png_infopp ppEndInfo) const;
	void _convert_to_8bit_rgba(png_structp pPng, png_infop pInfo) const;
};

