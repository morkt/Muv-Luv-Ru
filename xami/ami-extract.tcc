// -*- C++ -*-
//! \file       ami-extract.tcc
//! \date       Mon Feb 17 22:50:42 2014
//! \brief      ami file extraction templates.
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

namespace xami {

template<class Writer> bool extractor<Writer>::
extract_entry (unsigned seq)
{
    assert (seq < this->count() && "Archive entry number out of range");
    const uint32_t* entry = header() + seq * 4;

    uint32_t id = bin::little_dword (entry[0]);
    uint32_t offset = bin::little_dword (entry[1]);
    size_t unpacked_size = bin::little_dword (entry[2]);
    size_t packed_size = bin::little_dword (entry[3]);
    size_t view_size = packed_size ? packed_size : unpacked_size;
    sys::mapping::const_view<char> data (m_in, offset, view_size);
    bool result = true;
    if (packed_size)
    {
        m_out_data.clear();
        m_out_data.reserve (unpacked_size);
        memory_inflate (data.begin(), view_size, m_out_data);

        if (m_out_data.size() > 12 && 0 == std::memcmp (&m_out_data[0], "GRP", 4))
            result = m_writer.write_image (id, &m_out_data[0], m_out_data.size());
        else 
            result = m_writer.write_raw (id, &m_out_data[0], m_out_data.size());
    }
    else if (data.size() > 12 && 0 == std::memcmp (&data[0], "SCR", 4))
        result = m_writer.write_script (id, data.begin(), view_size);
    else
        result = m_writer.write_raw (id, data.begin(), view_size);

    return result;
}

template<class Writer> bool extractor<Writer>::
extract (uint32_t id)
{
    const uint32_t* entry = m_header.begin();
    for (unsigned i = 0; i < m_count; ++i)
    {
        uint32_t entry_id = bin::little_dword (entry[0]);
        if (entry_id == id)
            return extract_entry (i);
        entry += 4;
    }
    return false;
}

template<class Writer> unsigned extractor<Writer>::
extract ()
{
    unsigned i;
    for (i = 0; i < this->count(); ++i)
        if (!extract_entry (i))
            break;
    return i;
}

} // namespace xami
