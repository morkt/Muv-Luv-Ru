// -*- C++ -*-
//! \file       png-convert.cc
//! \date       Mon Feb 03 00:10:48 2014
//! \brief      convert RGBA data to/from PNG format.
//
// Copyright (C) 2014 morkt and the MuvLuvRu project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ---------------------------------------------------------------------------
//
// The oFFs chunk contains:
//   X position:     4 bytes (signed integer)
//   Y position:     4 bytes (signed integer)
//   Unit specifier: 1 byte
//
// The following values are legal for the unit specifier:
//   0: unit is the pixel (true dimensions unspecified)
//   1: unit is the micrometer
//

#include <fstream>
#include <png.h>
#include <setjmp.h>
#include <tchar.h>
#include "png-convert.hpp"

namespace png {

void
read_stream (png_structp png_ptr, png_bytep data, png_size_t length)
{
    std::istream* in = static_cast<std::istream*> (png_get_io_ptr (png_ptr));
    if (!in->read ((char*)data, length) || static_cast<size_t> (in->gcount()) != length)
        png_error (png_ptr, "file read error");
}

void
write_stream (png_structp png_ptr, png_bytep data, png_size_t length)
{
    void* io_ptr = png_get_io_ptr (png_ptr);
    static_cast<std::ostream*> (io_ptr)->write ((char*)data, length);
}

void
flush_stream (png_structp png_ptr)
{
    void* io_ptr = png_get_io_ptr (png_ptr);
    static_cast<std::ostream*> (io_ptr)->flush();
}

bool
has_transparency (const uint8_t* pixel_data, size_t width, size_t height)
{
    for (auto data_end = pixel_data + width*height*4; pixel_data != data_end; pixel_data += 4)
        if (0xff != pixel_data[3])
            return true;
    return false;
}

struct io_struct
{
    png_structp png_ptr;
    png_infop   info_ptr;

    io_struct () : png_ptr (0), info_ptr (0) { }

    png_structp png () const { return png_ptr; }
    png_infop info () const { return info_ptr; }
};

struct write_struct : io_struct
{
    write_struct () : io_struct() { }

    bool create ()
    {
        png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png_ptr)
            return false;

        info_ptr = png_create_info_struct (png_ptr);
        if (!info_ptr)
            return false;
        return true;
    }

    ~write_struct ()
    {
        if (info_ptr)
            png_destroy_write_struct (&png_ptr, &info_ptr);
        else if (png_ptr)
            png_destroy_write_struct (&png_ptr, 0);
    }

    using io_struct::png;
    using io_struct::info;
};

class read_struct : io_struct
{
    png_infop   end_info;

public:
    read_struct () : io_struct(), end_info (0) { }

    bool create ()
    {
        png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png_ptr)
            return false;

        info_ptr = png_create_info_struct (png_ptr);
        if (!info_ptr)
            return false;
        
        return true;
    }

    bool create_with_end ()
    {
        if (!create())
            return false;
        
        end_info = png_create_info_struct (png_ptr);
        if (!end_info)
            return false;

        return true;
    }

    ~read_struct ()
    {
        if (end_info)
            png_destroy_read_struct (&png_ptr, &info_ptr, &end_info);
        else if (info_ptr)
            png_destroy_read_struct (&png_ptr, &info_ptr, 0);
        else if (png_ptr)
            png_destroy_read_struct (&png_ptr, 0, 0);
    }

    using io_struct::png;
    using io_struct::info;
    png_infop end () const { return end_info; }
};

error
encode (const tstring& filename, const uint8_t* const pixel_data,
        unsigned width, unsigned height, int off_x, int off_y)
{
    if (!width || !height)
        return error::params;

    write_struct png;
    if (!png.create())
        return error::init;

    std::ofstream out (filename, std::ios::out|std::ios::binary|std::ios::trunc);
    if (!out)
        return error::io;

    // ---------------------------------------------------------------------------
    // no local objects should be declared below this point
    //
    if (setjmp (png_jmpbuf (png.png_ptr)))
        return error::failure;

    // size of the IDAT chunks
    png_set_compression_buffer_size (png.png_ptr, 256*1024);

    int color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    if (!has_transparency (pixel_data, width, height))
        color_type = PNG_COLOR_TYPE_RGB;

    png_set_write_fn (png.png_ptr, &out, write_stream, flush_stream);
    png_set_IHDR (png.png_ptr, png.info_ptr, width, height, 8, color_type,
                  PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    if (off_x || off_y)
        png_set_oFFs (png.png_ptr, png.info_ptr, off_x, off_y, PNG_OFFSET_PIXEL);
    png_write_info (png.png_ptr, png.info_ptr);

    png_set_bgr (png.png_ptr);
    if (!(PNG_COLOR_MASK_ALPHA & color_type))
        png_set_filler (png.png_ptr, 0, PNG_FILLER_AFTER);

    const uint8_t* image_ptr = pixel_data + 4*width*(height-1);
    for (size_t i = 0; i < height; ++i)
    {
        png_write_row (png.png_ptr, const_cast<uint8_t*> (image_ptr));
        image_ptr -= 4*width;
    }
    png_write_end (png.png_ptr, NULL);

    return error::none;
}

error
decode (const tstring& filename, std::vector<uint8_t>& bgr_data,
        unsigned* const width, unsigned* const height, int* const off_x, int* const off_y)
{
    if (!width || !height)
        return error::params;

    std::ifstream in (filename, std::ios::in|std::ios::binary);
    if (!in)
        return error::io;

    char header[8];
    if (!in.read (header, 8) || in.gcount() != 8
        || 0 != png_sig_cmp ((png_bytep)header, 0, 8))
        return error::format;

    read_struct read;
    if (!read.create())
        return error::init;

    // ---------------------------------------------------------------------------
    // no local objects should be declared below this point
    //
    if (setjmp (png_jmpbuf (read.png())))
        return error::failure;

    png_set_read_fn (read.png(), &in, read_stream);
    png_set_sig_bytes (read.png(), 8);

    png_read_info (read.png(), read.info());

    *width  = png_get_image_width (read.png(), read.info());
    *height = png_get_image_height (read.png(), read.info());

    if (!*width || !*height)
        return error::format;

    if (PNG_INTERLACE_NONE != png_get_interlace_type (read.png(), read.info()))
        return error::interlace;

    int color_type = png_get_color_type (read.png(), read.info());
    int bit_depth = png_get_bit_depth (read.png(), read.info());

    if (off_x) *off_x = png_get_x_offset_pixels (read.png(), read.info());
    if (off_y) *off_y = png_get_y_offset_pixels (read.png(), read.info());

    if (PNG_COLOR_TYPE_PALETTE == color_type)
        png_set_palette_to_rgb (read.png());
    else if (PNG_COLOR_TYPE_GRAY == color_type || PNG_COLOR_TYPE_GRAY_ALPHA == color_type)
        png_set_gray_to_rgb (read.png());
    if (png_get_valid (read.png(), read.info(), PNG_INFO_tRNS))
        png_set_tRNS_to_alpha (read.png());
    else if (!(PNG_COLOR_MASK_ALPHA & color_type))
        png_set_filler (read.png(), 0xff, PNG_FILLER_AFTER);
    if (16 == bit_depth)
        png_set_strip_16 (read.png());

    png_set_bgr (read.png());

    png_read_update_info (read.png(), read.info());

    const size_t data_offset = bgr_data.size();
    const size_t row_size = *width * 4;
    bgr_data.resize (data_offset + row_size * *height);
    uint8_t* row_ptr = &bgr_data[data_offset + row_size * (*height-1)];
    for (size_t row = *height; row > 0; --row)
    {
         png_read_row (read.png(), row_ptr, NULL);
         row_ptr -= row_size;
    }
    png_read_end (read.png(), 0);

    return error::none;
}

const TCHAR*
get_error_text (error num)
{
    switch (num)
    {
    case error::none:       return _T("no error");
    case error::io:         return _T("i/o error");
    case error::init:       return _T("initialization error");
    case error::failure:    return _T("unknown error");
    case error::format:     return _T("invalid PNG format");
    case error::params:     return _T("unexpected parameters");
    case error::interlace:  return _T("interlaced images not supported");
    }
    return _T("PNG library error");
}

} // namespace png
