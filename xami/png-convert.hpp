// -*- C++ -*-
//! \file       png-convert.hpp
//! \date       Mon Feb 03 01:16:09 2014
//! \brief      convert PNG images to and from BGRA pixel data.
//
// BGRA -> reversed RGB + alpha channel, image rows start from the bottom.
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

#ifndef PNG_CONVERT_HPP
#define PNG_CONVERT_HPP

#include <cstdint>
#include <vector>
#include "stringutil.hpp"

namespace png {

using ext::tstring;

using std::uint8_t;

enum error {
    none,
    io,
    init,
    failure,
    format,
    params,
    interlace,
};

error encode (const tstring& to_file, const uint8_t* const bgr_data,
              size_t width, size_t height, int off_x = 0, int off_y = 0);

error decode (const tstring& from_file, std::vector<uint8_t>& bgr_data,
              unsigned* const width, unsigned* const height,
              int* const off_x = 0, int* const off_y = 0);

const TCHAR* get_error_text (error num);

} // namespace png

#endif /* PNG_CONVERT_HPP */
