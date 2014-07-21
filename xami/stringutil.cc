// -*- C++ -*-
//! \file       stringutil.cc
//! \date       Mon Feb 10 23:20:43 2014
//! \brief      useful string utilities.
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

#include "stringutil.hpp"

namespace ext {

int
mbstowcs (const char* cstr, size_t cstr_len, std::wstring& wstr, unsigned codepage)
{
    local_buffer<wchar_t> wbuf (cstr_len);
    int count = ::MultiByteToWideChar (codepage, 0, cstr, cstr_len,
				       wbuf.get(), wbuf.size());
    if (!count)
    {
	int err = ::GetLastError();
	if (err != ERROR_INSUFFICIENT_BUFFER)
	    return 0;
	count = ::MultiByteToWideChar (codepage, 0, cstr, cstr_len,
				       wbuf.get(), 0);
	wbuf.reserve (count);
	count = ::MultiByteToWideChar (codepage, 0, cstr, cstr_len,
				       wbuf.get(), wbuf.size());
	if (!count) return 0;
    }
    wstr.assign (wbuf.get(), count);
    return wstr.size();
}

int
wcstombs (const wchar_t* wstr, size_t wstr_len, std::string& cstr, unsigned codepage)
{
    // loosely attempt to predict output size
    local_buffer<char> cbuf (wstr_len * 2);
    int count = ::WideCharToMultiByte (codepage, 0, wstr, wstr_len,
				       cbuf.get(), cbuf.size(), 0, 0);
    if (!count)
    {
	int err = ::GetLastError();
	if (err != ERROR_INSUFFICIENT_BUFFER)
	    return 0;
	count = ::WideCharToMultiByte (codepage, 0, wstr, wstr_len,
				       cbuf.get(), 0, 0, 0);
	cbuf.reserve (count);
	count = ::WideCharToMultiByte (codepage, 0, wstr, wstr_len,
				       cbuf.get(), cbuf.size(), 0, 0);
	if (!count) return 0;
    }
    cstr.assign (cbuf.get(), count);
    return cstr.size();
}

} // namespace ext
