// -*- C++ -*-
//! \file       ami-archive.hpp
//! \date       Sun Jan 26 16:16:21 2014
//! \brief      AMI file reader class.
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

#ifndef AMI_ARCHIVE_HPP
#define AMI_ARCHIVE_HPP

#include <iostream>
#include <vector>
#include <tchar.h>
#include "sysmemmap.h"
#include "bindata.h"
#include "xami-util.hpp"

namespace xami {

struct entry
{
    uint32_t    id;
    uint32_t    offset;
    size_t      packed_size;
    size_t      unpacked_size;
};

class file_reader
{
protected:
    sys::mapping::readonly              m_in;
    sys::mapping::const_view<uint32_t>  m_header;
    unsigned                            m_count;

public:
    typedef std::vector<entry> content_type;

    template <typename CharT>
    explicit file_reader (const CharT* filename);

    unsigned count () const { return m_count; }
    const uint32_t* header () const { return m_header.begin(); }

    void read_content (content_type& content);

    size_t copy_to (unsigned seq, std::ostream& out);
};

void write_ami_header (const file_reader::content_type& content, std::ostream& out);
void write_ami_entry (const file_info& file, entry& entry, std::ostream& out);

class converter
{
public:
    static tstring format_filename (uint32_t id, const TCHAR* ext);

    bool write_raw (uint32_t id, const char* buffer, size_t size);
    bool write_script (uint32_t id, const char* scr_data, size_t size);
    bool write_image (uint32_t id, const char* grp_data, size_t size);

    enum action
    {
        action_abort,
        action_skip,
        action_ok,
    };
};

template <class Writer>
class extractor : public file_reader
{
    Writer              m_writer;

public:
    template <typename CharT>
    explicit extractor (const CharT* filename)
        : file_reader (filename), m_writer()
    { }

    template <typename CharT, class Arg>
    extractor (const CharT* filename, const Arg& arg)
        : file_reader (filename), m_writer (arg)
    { }

    unsigned extract ();
    bool extract (uint32_t id);

    const Writer& writer () const { return m_writer; }

private:
    bool extract_entry (unsigned seq);

    std::vector<char>       m_out_data;
};

template <typename CharT> file_reader::
file_reader (const CharT* filename)
    : m_in (filename)
    , m_header (m_in, 0, 4)
{
    if (bin::little_dword (m_header[0]) != 0x494d41) // 'AMI'
        throw sys::file_error (filename, _T("file format not recognized"));
    m_count = bin::little_dword (m_header[1]);
    m_header.remap (m_in, 0x10, m_count*4);
}

} // namespace xami

#include "ami-extract.tcc"

#endif /* AMI_ARCHIVE_HPP */
