// -*- C++ -*-
//! \file       ami-reader.cc
//! \date       Sun Jan 26 16:15:23 2014
//! \brief      AMI file reader class implementation.
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

#include "ami-archive.hpp"
#include "xami-util.hpp"
#include <sstream>
#include <iomanip>

namespace xami {

void file_reader::
read_content (content_type& content)
{
    const uint32_t* ent = m_header.begin();
    for (unsigned i = 0; i < m_count; ++i)
    {
        entry data;
        data.id = bin::little_dword (ent[0]);
        data.offset = bin::little_dword (ent[1]);
        data.unpacked_size = bin::little_dword (ent[2]);
        data.packed_size = bin::little_dword (ent[3]);
        content.push_back (data);
        ent += 4;
    }
}

size_t file_reader::
copy_to (unsigned seq, std::ostream& out)
{
    assert (seq < m_count && "Archive record index is out of range");
    const uint32_t* entry = m_header.begin() + seq * 4;

    uint32_t offset = bin::little_dword (entry[1]);
    size_t unpacked_size = bin::little_dword (entry[2]);
    size_t packed_size = bin::little_dword (entry[3]);
    size_t view_size = packed_size ? packed_size : unpacked_size;
    sys::mapping::const_view<char> in (m_in, offset, view_size);
    out.write (in.begin(), view_size);
    return view_size;
}

tstring converter::
format_filename (uint32_t id, const TCHAR* ext)
{
    ext::tostringstream os;
    os << std::hex << std::setw(8) << std::setfill(_T('0')) << id << _T('.') << ext;
    return os.str();
}

bool converter::
write_raw (uint32_t id, const char* buffer, size_t size)
{
    tstring filename = format_filename (id, _T("dat"));
//    sys::ofstream out;
//    if (action_ok == open_stream (out, filename))
//        out.write (buffer, size);
    xami::write_raw (filename, buffer, size);
    return true;
}

bool converter::
write_script (uint32_t id, const char* scr_data, size_t size)
{
    tstring filename = format_filename (id, _T("mlt"));
    xami::write_script (filename, id, scr_data, size);
    return true;
}

bool converter::
write_image (uint32_t id, const char* grp_data, size_t size)
{
    tstring filename = format_filename (id, _T("png"));
    xami::write_png (filename, grp_data, size);
    return true;
}

} // namespace xami
