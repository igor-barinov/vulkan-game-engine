#include "PNGReader.h"

#include <stdexcept>

PNGReader::PNGReader()
	: _rows(nullptr),
	_imgWidth(0),
	_imgHeight(0),
	_colorType(0),
	_bitDepth(0)
{
}
PNGReader::PNGReader(const std::string& filepath)
	: _rows(nullptr),
	_imgWidth(0),
	_imgHeight(0),
	_colorType(0),
	_bitDepth(0)
{
	read(filepath);
}
PNGReader::~PNGReader()
{
	if (_rows != nullptr)
	{
		_free_rows();
	}
}


void PNGReader::read(const std::string& filepath)
{
	FILE* pFile;
	_open_file(&pFile, filepath.c_str());

	png_structp pPng{};
	png_infop pInfo{};
	png_infop pEndInfo{};

	_create_png_structs(&pPng, &pInfo, &pEndInfo);

	if (setjmp(png_jmpbuf(pPng)))
	{
		png_destroy_read_struct(&pPng, &pInfo, &pEndInfo);
		fclose(pFile);
		throw std::runtime_error("Error while reading PNG file");
	}

	png_init_io(pPng, pFile);
	png_set_sig_bytes(pPng, _HEADER_BYTES);

	png_read_info(pPng, pInfo);

	_imgWidth = png_get_image_width(pPng, pInfo);
	_imgHeight = png_get_image_height(pPng, pInfo);
	_colorType = png_get_color_type(pPng, pInfo);
	_bitDepth = png_get_bit_depth(pPng, pInfo);

	_convert_to_8bit_rgba(pPng, pInfo);

	png_read_update_info(pPng, pInfo);

	if (_rows != nullptr)
	{
		_free_rows();
	}

	_alloc_rows(pPng, pInfo);
	png_read_image(pPng, _rows);

	png_read_end(pPng, pInfo);
	fclose(pFile);
	png_destroy_read_struct(&pPng, &pInfo, NULL);
}

void PNGReader::_alloc_rows(const png_structp pPng, const png_infop pInfo)
{
	_rows = new byte_t* [_imgHeight];
	for (size_t i = 0; i < _imgHeight; ++i) 
	{
		size_t rowSize = png_get_rowbytes(pPng, pInfo);
		_rows[i] = new byte_t[rowSize];
		memset(_rows[i], 0, sizeof(byte_t) * rowSize);
	}
}

void PNGReader::_free_rows()
{
	for (size_t i = 0; i < _imgHeight; ++i)
	{
		delete[] _rows[i];
	}
	delete[] _rows;
}

void PNGReader::_open_file(FILE** ppFile, const char* filepath) const
{
	errno_t err{};
	err = fopen_s(ppFile, filepath, "rb");
	if (err != 0)
	{
		throw std::runtime_error("Failed to open PNG file");
	}

	byte_t header[_HEADER_BYTES];
	fread(header, sizeof(byte_t), _HEADER_BYTES, *ppFile);
	bool isPNG = !png_sig_cmp(header, 0, _HEADER_BYTES);
	if (!isPNG)
	{
		fclose(*ppFile);
		throw std::runtime_error("File is not a valid PNG file");
	}
}

void PNGReader::_create_png_structs(png_structpp ppPng, png_infopp ppInfo, png_infopp ppEndInfo) const
{
	*ppPng = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, (png_error_ptr)NULL, (png_error_ptr)NULL);
	if (!*ppPng)
	{
		throw std::runtime_error("Failed to create PNG pointer");
	}

	*ppInfo = png_create_info_struct(*ppPng);
	if (!*ppInfo)
	{
		png_destroy_read_struct(ppPng, NULL, NULL);
		throw std::runtime_error("Failed to create PNG information pointer");
	}

	*ppEndInfo = png_create_info_struct(*ppPng);
	if (!*ppEndInfo)
	{
		png_destroy_read_struct(ppPng, ppInfo, NULL);
		throw std::runtime_error("Failed to create PNG end information pointer");
	}
}

void PNGReader::_convert_to_8bit_rgba(png_structp pPng, png_infop pInfo) const
{
	if (_bitDepth == 16)
	{
		png_set_strip_16(pPng);
	}

	if (_colorType == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb(pPng);
	}

	if (_colorType == PNG_COLOR_TYPE_GRAY && _bitDepth < 8)
	{
		png_set_expand_gray_1_2_4_to_8(pPng);
	}

	if (png_get_valid(pPng, pInfo, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(pPng);
	}

	if (_colorType == PNG_COLOR_TYPE_RGB ||
		_colorType == PNG_COLOR_TYPE_GRAY ||
		_colorType == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_filler(pPng, 0xFF, PNG_FILLER_AFTER);
	}

	if (_colorType == PNG_COLOR_TYPE_GRAY ||
		_colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		png_set_gray_to_rgb(pPng);
	}
}