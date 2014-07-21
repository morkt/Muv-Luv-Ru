// -*- C++ -*-
//! \file       fileutil.cc
//! \date       Tue Feb 18 10:21:19 2014
//! \brief      useful filesystem utilities implementation.
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

#include "fileutil.hpp"
#include "syshandle.h"

namespace ext {

bool
is_same_file (const TCHAR* lhs, const TCHAR* rhs)
{
    sys::file_handle f1 (::CreateFile (lhs, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING,
                                       FILE_ATTRIBUTE_NORMAL, 0));
    if (!f1) return false;
    sys::file_handle f2 (::CreateFile (rhs, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING,
                                       FILE_ATTRIBUTE_NORMAL, 0));
    if (!f2) return false;

    BY_HANDLE_FILE_INFORMATION info1;
    BY_HANDLE_FILE_INFORMATION info2;
    if (::GetFileInformationByHandle (f1, &info1) && ::GetFileInformationByHandle (f2, &info2))
        return info1.dwVolumeSerialNumber == info2.dwVolumeSerialNumber
               && info1.nFileIndexHigh == info2.nFileIndexHigh
               && info1.nFileIndexLow == info2.nFileIndexLow;
    return false;
}

} // namespace icase
