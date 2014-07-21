// -*- C++ -*-
//! \file       xami-util.cc
//! \date       Thu Jan 23 05:37:49 2014
//! \brief      utilities for handling Amaterasu translations Muv-Luv data files.
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
//

#include <zlib.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstring>
#include "xami-util.hpp"
#include "sysmemmap.h"
#include "png-convert.hpp"

namespace xami {

size_t
memory_inflate (const char* zdata, size_t zsize, std::vector<char>& out)
{
    if (!zsize) return 0;
    z_stream z_str = { 0 };
    z_str.next_in = (Byte*) zdata;
    z_str.avail_in = zsize;

    int z_err = inflateInit (&z_str);
    if (z_err != Z_OK)
        return 0;

    const size_t buf_size = 1024;
    char buf[buf_size];
    while (z_err != Z_STREAM_END)
    {
        z_str.next_out = (Byte*) buf;
        z_str.avail_out = buf_size;

        z_err = ::inflate (&z_str, Z_NO_FLUSH);
        if (Z_NEED_DICT == z_err || Z_DATA_ERROR ==  z_err || Z_MEM_ERROR == z_err)
            break;
        if (size_t have = buf_size - z_str.avail_out)
            out.insert (out.end(), buf, buf+have);
    }
    inflateEnd (&z_str);
    if (Z_STREAM_END != z_err)
        throw std::runtime_error ("Invalid compressed data stream.");
    return z_str.total_out;
}

bool
write_png (const tstring& filename, const char* grp_data, size_t size)
{
    assert (size > 12 && "Invalid GRP image data");

    const uint8_t* pixel_data = reinterpret_cast<const uint8_t*> (grp_data);
    const int ref_x = get_grp_ref_x (pixel_data);
    const int ref_y = get_grp_ref_y (pixel_data);
    const size_t width  = get_grp_width (pixel_data);
    const size_t height = get_grp_height (pixel_data);
    const size_t total = width * height * 4;
    if (total+12u > size)
    {
        TCLOG << filename << _T(": invalid image dimensions (")
            << width << _T('x') << height << _T(")\n");
        return false; // throw std::runtime_error ("invalid image dimensions");
    }
    if (ref_x || ref_y)
        TCOUT << filename << _T(' ') << ref_x << _T(' ') << ref_y << std::endl;
    pixel_data += 12;
    png::error rc = png::encode (filename, pixel_data, width, height, ref_x, ref_y);
    if (png::error::none != rc)
        TCLOG << filename << _T(": ") << png::get_error_text (rc) << std::endl;
    return png::error::none == rc;
}

bool
read_file_list (const char* input_name, std::vector<std::string>& file_list)
{
    std::ifstream in (input_name);
    if (!in)
        return false;
    int count = 0;
    std::string line;
    do
    {
        getline (in, line);
        if (line.empty())
            continue;
        file_list.push_back (line);
        ++count;
    }
    while (in);
    return true;
}

unsigned
get_id_from_name (const tstring& name)
{
    size_t start_pos = name.find_last_of (_T(":\\/"));
    if (tstring::npos == start_pos)
        start_pos = 0;
    else
        ++start_pos;
    return _tcstoul (&name[start_pos], 0, 16);
}

file_info
get_file_info (const TCHAR* filename)
{
    WIN32_FIND_DATA find_data;
    HANDLE fff = ::FindFirstFile (filename, &find_data);
    if (INVALID_HANDLE_VALUE == fff)
    {
        TCERR << filename << _T(": file not found.\n");
        throw std::runtime_error ("File not found.");
    }
    ::FindClose (fff);
    if (find_data.nFileSizeHigh)
    {
        TCERR << filename << _T(": file is too long.\n");
        throw std::runtime_error ("File is too long.");
    }
    file_type type = file_raw;
    if (const TCHAR* dot = _tcsrchr (filename, _T('.')))
    {
        if (0 == icase::strcmp (dot, _T(".png")))
            type = file_png;
        else if (0 == icase::strcmp (dot, _T(".zgrp")))
            type = file_zgrp;
    }
    return file_info (find_data, type);
}

size_t
deflate_data (std::ostream& out, const uint8_t* input, size_t size)
{
    z_stream z_str = { 0 };
    z_str.next_in = (Byte*) input;
    z_str.avail_in = size;
    int z_err = deflateInit (&z_str, 9);
    if (z_err != Z_OK)
        return 0;

    const size_t buf_size = 512;
    char buf[buf_size];
    int flush = Z_NO_FLUSH;
    while (Z_FINISH != flush)
    {
        flush = z_str.avail_in ? Z_NO_FLUSH : Z_FINISH;
        do
        {
            z_str.next_out = (Byte*) buf;
            z_str.avail_out = buf_size;
            deflate (&z_str, flush);
            if (size_t have = buf_size - z_str.avail_out)
                out.write (buf, have);
        } while (0 == z_str.avail_out);
        assert (0 == z_str.avail_in);
    }
    deflateEnd (&z_str);
    return z_str.total_out;
}

size_t
convert_png (const tstring& filename, std::ostream& out, size_t& compressed_size)
{
    std::vector<uint8_t> image (GRP_HEADER_SIZE);
    unsigned width, height;
    int x, y;
    png::error rc = png::decode (filename, image, &width, &height, &x, &y);
    if (png::error::none != rc)
    {
        TCLOG << filename << _T(": ") << png::get_error_text (rc) << std::endl;
        throw std::runtime_error ("Error reading PNG image.");
    }
    if (width > 0x7fff || height > 0x7fff)
    {
        TCLOG << filename << _T(": image resolution is too high (")
            << width << _T('x') << height << _T(")\n");
        throw std::runtime_error ("Unsupported image resolution.");
    }
    int16_t* header = reinterpret_cast<int16_t*> (image.data());
    header[0] = bin::little_word (0x5247);
    header[1] = bin::little_word (0x0050);
    header[2] = bin::little_word (x);
    header[3] = bin::little_word (y);
    header[4] = bin::little_word (width);
    header[5] = bin::little_word (height);
    compressed_size = deflate_data (out, image.data(), image.size());
    return image.size();
}

size_t
deflate_file (const tstring& filename, std::ostream& out, size_t& compressed_size)
{
    sys::mapping::readonly in (filename);
    sys::mapping::const_view<uint8_t> data (in);
    compressed_size = deflate_data (out, data.begin(), data.size());
    return data.size();
}

bool
write_raw (const tstring& filename, const char* data, size_t size)
{
    std::ofstream out (filename, std::ios::out|std::ios::trunc|std::ios::binary);
    if (out)
        out.write (data, size);
    return !out.fail();
}

size_t
copy_file (const tstring& filename, std::ostream& out)
{
    sys::mapping::readonly in (filename);
    sys::mapping::const_view<char> data (in);
    out.write (data.begin(), data.size());
    return data.size();
}

size_t
copy_zgrp (const tstring& filename, std::ostream& out)
{
    sys::mapping::readonly in (filename);
    sys::mapping::const_view<char> data (in);
    if (data.size() <= ZGRP_HEADER_SIZE)
        throw std::runtime_error ("invalid compressed image");

    size_t unpacked = bin::little_dword (*(uint32_t*)&data[0]);
    out.write (data.begin()+ZGRP_HEADER_SIZE, data.size()-ZGRP_HEADER_SIZE);
    return unpacked;
}

} // namespace xami
