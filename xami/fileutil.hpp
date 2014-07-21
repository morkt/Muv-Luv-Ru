// -*- C++ -*-
//! \file       fileutil.hpp
//! \date       Tue Feb 18 09:42:38 2014
//! \brief      useful filesystem utility functions.
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

#include <windows.h>
#include <tchar.h>

#ifndef EXT_FILEUTIL_HPP
#define EXT_FILEUTIL_HPP

#include "stringutil.hpp"

namespace ext {

inline bool file_exists (const char* filename)
{
    DWORD rc = ::GetFileAttributesA (filename);
    return (INVALID_FILE_ATTRIBUTES != rc && !(FILE_ATTRIBUTE_DIRECTORY & rc));
}

inline bool file_exists (const wchar_t* filename)
{
    DWORD rc = ::GetFileAttributesW (filename);
    return (INVALID_FILE_ATTRIBUTES != rc && !(FILE_ATTRIBUTE_DIRECTORY & rc));
}

inline TCHAR*
get_filename_part (TCHAR* path)
{
    TCHAR* filename = _tcsrchr (path, _T('\\'));
    if (filename)
        return filename+1;
    else
        return path;
}

inline const TCHAR*
get_filename_part (const TCHAR* path)
{
    return get_filename_part (const_cast<TCHAR*> (path));
}

inline bool
compare_filename (const TCHAR* fullname, const TCHAR* filename)
{
    return 0 == icase::strcmp (get_filename_part (fullname), filename);
}

bool is_same_file (const TCHAR* lhs, const TCHAR* rhs);

} // namespace icase

#endif /* EXT_FILEUTIL_HPP */
