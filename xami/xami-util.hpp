// -*- C++ -*-
//! \file       xami-util.hpp
//! \date       Thu Jan 23 05:35:33 2014
//! \brief      utilities for handling Amateraus translations Muv-Luv data files.
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

#ifndef XAMI_UTIL_HPP
#define XAMI_UTIL_HPP

#include <string>
#include <vector>
#include <tchar.h>
#include "bindata.h"
#include "xami-types.hpp"

namespace xami {

const size_t ZGRP_HEADER_SIZE = 4u;
const size_t GRP_HEADER_SIZE  = 12u;

enum file_type
{
    file_raw,
    file_png,
    file_grp,
    file_zgrp,
    file_mlt,
    file_txt,
    file_xml,
};

struct file_info
{
    tstring     name;
    size_t      size;
    file_type   type;
    FILETIME    time;

    file_info () { }
    file_info (const TCHAR* n, size_t s, file_type t = file_raw)
        : name (n), size (s), type (t)
    { }
    file_info (const WIN32_FIND_DATA& fd, file_type tp) { assign (fd, tp); }

    void assign (const WIN32_FIND_DATA& fd, file_type tp)
    {
        name = fd.cFileName;
        size = fd.nFileSizeLow;
        type = tp;
        time = fd.ftLastWriteTime;
    }
};

// inflate data stream stored into ZDATA, ZSIZE bytes length and put result into OUT.
size_t memory_inflate (const char* zdata, size_t zsize, std::vector<char>& out);

// read raw RGBA data stored within GRP_DATA in muv-luv GRP format and write it into
// file FILENAME in PNG format.
bool write_png (const tstring& filename, const char* grp_data, size_t size);

// read SCR text script data from SCR_DATA and write it into FILENAME in MLT format.
bool write_script (const tstring& filename, uint32_t id, const char* scr_data,
                   size_t size, encoding_id enc = enc_shift_jis);

bool write_script_mlt (std::ostream& out, uint32_t file_id, const char* scr_data,
                       size_t size, encoding_id enc);
bool write_script_txt (std::ostream& out, uint32_t file_id, const char* scr_data,
                       size_t size, encoding_id enc);
bool write_script_xml (std::ostream& out, uint32_t file_id, const char* scr_data,
                       size_t size, encoding_id enc);

bool write_raw (const tstring& filename, const char* data, size_t size);

// get info about FILENAME without opening it (size, name and type)
file_info get_file_info (const TCHAR* filename);

// convert filename in the form XXXXXXXX.EXT into corresponding integer.
unsigned get_id_from_name (const tstring& name);

// convert PNG image stored in FILENAME into compressed muv-luv grp stream and write
// it into OUT.  size of the stream is stored into COMPRESSED_SIZE.
// Returns: size of the uncompressed stream.
size_t convert_png (const tstring& filename, std::ostream& out, size_t& compressed_size);

// read compressed stream stream from FILENAME and copy it into OUT.
// first 4 bytes of the stream represent its uncompressed size and are returned to
// caller.
size_t copy_zgrp (const tstring& filename, std::ostream& out);

// copy file from FILENAME into stream OUT
// Returns: size of the copied file.
size_t copy_file (const tstring& filename, std::ostream& out);

// deflate data stored in the FILENAME and write deflated stream into OUT.  size of the
// stream is stored into COMPRESSED_SIZE.
// Returns: original size of FILENAME
size_t deflate_file (const tstring& filename, std::ostream& out, size_t& compressed_size);

// prepend muv-luv GRP header to the raw RGBA image data supplied into INPUT using WIDTH
// and HEIGHT as image dimensions, and REF_X and REF_Y as GRP reference points. after
// that, deflate whole data and write it into OUT.
// Returns: size of resulting compressed stream.
size_t write_grp_zstream (std::ostream& out, const uint8_t* input, uint16_t width, uint16_t height,
                          int16_t ref_x = 0, int16_t ref_y = 0);

// read file names from the file INPUT_NAME and put them into FILE_LIST
// Returns: FALSE if INPUT_NAME cannot be opened, TRUE otherwise
bool read_file_list (const char* input_name, std::vector<std::string>& file_list);

template <size_t N>
inline int16_t get_grp_word (const uint8_t* grp_data)
{
    return bin::little_word (*(const int16_t*)&grp_data[N]);
}

inline int get_grp_ref_x (const uint8_t* grp_data)
{
    return get_grp_word<4> (grp_data);
}

inline int get_grp_ref_y (const uint8_t* grp_data)
{
    return get_grp_word<6> (grp_data);
}

inline int get_grp_width (const uint8_t* grp_data)
{
    return get_grp_word<8> (grp_data);
}

inline int get_grp_height (const uint8_t* grp_data)
{
    return get_grp_word<10> (grp_data);
}

template <typename CharT>
const CharT* encoding_name (encoding_id enc);

template <>
inline const char* encoding_name (encoding_id enc)
{
    return enc_utf8 == enc ? "UTF-8" : "Shift-JIS";
}

template <>
inline const wchar_t* encoding_name (encoding_id enc)
{
    return enc_utf8 == enc ? L"UTF-8" : L"Shift-JIS";
}

inline encoding_id encoding_from_name (const tstring& name)
{
    if (0 == icase::strcmp (_T("Shift-JIS"), name.c_str())) return enc_shift_jis;
    if (0 == icase::strcmp (_T("UTF-8"),     name.c_str())) return enc_utf8;
    return enc_default;
}

} // namespace xami

#endif /* XAMI_UTIL_HPP */
