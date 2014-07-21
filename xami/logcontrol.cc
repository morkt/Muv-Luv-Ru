// -*- C++ -*-
//! \file       logcontrol.cc
//! \date       Mon Feb 17 10:17:14 2014
//! \brief      edit control as backend for stream buffer
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

#include "logcontrol.hpp"
#include <tchar.h>

namespace xami {

window_buf::int_type window_buf::
overflow (int_type c)
{
    if (traits_type::eq_int_type (c, traits_type::eof()))
	return traits_type::not_eof (c);

    if (!traits_type::eq_int_type (c, '\n'))
    {
        m_buffer += traits_type::to_char_type (c);
        return (c);
    }
    ::SetWindowText (m_edit, m_buffer.c_str());
    ::SendMessage (m_edit, EM_SETSEL, m_buffer.size(), m_buffer.size());
    ::SendMessage (m_edit, EM_SCROLLCARET, 0, 0);

    m_buffer += traits_type::to_char_type ('\r');
    m_buffer += traits_type::to_char_type ('\n');

    return (c);
}

} // namespace xami
