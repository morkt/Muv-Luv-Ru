// -*- C++ -*-
//! \file       logcontrol.hpp
//! \date       Mon Feb 17 10:37:25 2014
//! \brief      edit control as backend for stream buffer.
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

#ifndef XAMI_LOGCONTROL_HPP
#define XAMI_LOGCONTROL_HPP

#include <streambuf>
#include <windows.h>
#include "stringutil.hpp"

namespace xami {

using ext::tstreambuf;

class window_buf : public tstreambuf
{
public:
    typedef TCHAR       			char_type;
    typedef std::char_traits<TCHAR>		traits_type;
    typedef std::basic_string<TCHAR>            string_type;
    typedef traits_type::int_type 	        int_type;

    explicit window_buf (HWND edit_ctl) : m_edit (edit_ctl) { }

protected: // virtual methods

    int_type overflow (int_type c);

private: // data

    HWND                m_edit;
    string_type         m_buffer;
};

} // namespace xami

#endif /* XAMI_LOGCONTROL_HPP */
