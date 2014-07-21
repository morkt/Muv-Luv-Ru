// -*- C++ -*-
//! \file       stringutil.hpp
//! \date       Mon Feb 10 22:44:36 2014
//! \brief      useful string utilities declarations.
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

#ifndef EXT_STRINGUTIL_HPP
#define EXT_STRINGUTIL_HPP

#include <string>
#include <cstring>
#include <cstdint>
#include <iosfwd>
#include <cctype>	// for std::toupper/tolower
#include <algorithm>
#include <windows.h>
#include <cassert>

#if defined(UNICODE) || defined(_UNICODE)
#   define TCOUT std::wcout
#   define TCERR std::wcerr
#   define TCLOG std::wclog
#   define to_tstring to_wstring
#else
#   define TCOUT std::cout
#   define TCERR std::cerr
#   define TCLOG std::clog
#   define to_tstring to_string
#endif

namespace ext {

typedef std::basic_string<TCHAR>        tstring;
typedef std::char_traits<TCHAR>         tchar_traits;
typedef std::basic_streambuf<TCHAR>     tstreambuf;
typedef std::basic_ostream<TCHAR>       tostream;
typedef std::basic_ostringstream<TCHAR> tostringstream;
typedef std::basic_istringstream<TCHAR> tistringstream;

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;

struct cstr_less
{
    bool operator() (const char* lhs, const char* rhs)
        { return std::strcmp (lhs, rhs) < 0; }
};

// ---------------------------------------------------------------------------
// Unicode conversion functions

int wcstombs (const wchar_t* wstr, size_t size, std::string& cstr, unsigned codepage);
int mbstowcs (const char* cstr, size_t size, std::wstring& wstr, unsigned codepage);

inline int wcstombs (const wchar_t* wstr, std::string& cstr, unsigned codepage)
{
    return wstr ? wcstombs (wstr, std::wcslen (wstr), cstr, codepage) : 0;
}

inline int mbstowcs (const char* cstr, std::wstring& wstr, unsigned codepage)
{
    return cstr ? mbstowcs (cstr, std::strlen (cstr), wstr, codepage) : 0;
}

inline int wcstombs (const std::wstring& wstr, std::string& cstr, unsigned codepage)
{
    return wcstombs (wstr.data(), wstr.size(), cstr, codepage);
}

inline int mbstowcs (const std::string& cstr, std::wstring& wstr, unsigned codepage)
{
    return mbstowcs (cstr.data(), cstr.size(), wstr, codepage);
}

// (From MSDN)
// WideCharToMultiByte does not null-terminate an output string if the input string
// length is explicitly specified without a terminating null character.
// To null-terminate an output string for this function, the application should pass
// in -1 or explicitly count the terminating null character for the input string.

inline int wcstombs (const wchar_t* wstr, char* dst, size_t dst_size, unsigned codepage)
{
    *dst = 0;
    if (!wstr || !*wstr) return 0;
    return ::WideCharToMultiByte (codepage, 0, wstr, -1, dst, dst_size, 0, 0);
}

inline int mbstowcs (const char* cstr, wchar_t* dst, size_t dst_size, unsigned codepage)
{
    *dst = 0;
    if (!cstr || !*cstr) return 0;
    return ::MultiByteToWideChar (codepage, 0, cstr, -1, dst, dst_size);
}

// u8tou32 (FIRST, LAST)
// convert single UTF-8 character into Unicode code point.
// Requires: FIRST != LAST
// Returns: converted unicode character or -1 if invalid UTF-8 sequence was
//          encountered.
// Posteffects: iterator FIRST points to the byte past the end of the converted
//              sequence.

template <class Iterator>
uint32_t u8tou32 (Iterator& first, Iterator last)
{
    assert (("Empty UTF-8 sequence", first != last));

    uint8_t ch = *first++;
    if (ch <= 0x7f) // ASCII character
	return ch;

    if ((ch & 0xc0) != 0xc0) // invalid UTF-8 sequence
	return ch;

    uint32_t code_point = 0;
    int code_length = 0;
    char mask = 0x40;
    for (;;)
    {
	if (first == last) // incomplete UTF-8 sequence
	{
	    code_point = -1;
	    break;
	}
	uint8_t next = *first;
	if ((next & 0xc0) != 0x80)
	{
	    code_point = -1;
	    break;
	}
	++first;
	code_point = (code_point << 6) | (next & 0x3f);
	++code_length;
	if ((mask >>= 1) == 1) // too long UTF-8 sequence, discard it
	{
	    code_point = -1;
	    break;
	}
	if (0 == (ch & mask))
	{
	    code_point |= static_cast<uint32_t> (ch & (mask - 1)) << (6 * code_length);
	    break;
	}
    }
    return code_point;
}

// u32tou8 (CODE, OUT)
// convert Unicode code point into UTF-8 sequence and put it into OUT iterator
// Returns: number of bytes in resulting UTF-8 sequence

template <class Iterator>
int u32tou8 (uint32_t code, Iterator& out)
{
    if (code > 0x10ffff)
	code = 0xfffd; // code point for invalid characters
    if (code <= 0x7f)
    {
	*out++ = static_cast<char> (code);
	return 1;
    }
    else if (code <= 0x7ff)
    {
	*out++ = static_cast<char> (0xc0 | (code >> 6));
	*out++ = static_cast<char> (0x80 | (code & 0x3f));
	return 2;
    }
    else if (code <= 0xffff)
    {
	*out++ = static_cast<char> (0xe0 | (code >> 12));
	*out++ = static_cast<char> (0x80 | ((code >> 6) & 0x3f));
	*out++ = static_cast<char> (0x80 | (code & 0x3f));
	return 3;
    }
    else
    {
	*out++ = static_cast<char> (0xf0 | (code >> 18));
	*out++ = static_cast<char> (0x80 | ((code >> 12) & 0x3f));
	*out++ = static_cast<char> (0x80 | ((code >> 6)  & 0x3f));
	*out++ = static_cast<char> (0x80 | (code & 0x3f));
	return 4;
    }
}

// u16tou32 (FIRST, LAST)
// convert single UTF-16 character into Unicode code point.

template <class Iterator>
inline uint32_t u16tou32 (Iterator& first, Iterator last)
{
    uint32_t code_point = static_cast<uint16_t> (*first++);
    if (code_point >= 0xd800 && code_point <= 0xdbff)
    {
	if (first != last)
	{
	    uint32_t next = static_cast<uint16_t> (*first);
	    if (next >= 0xdc00 && next <= 0xdfff)
	    {
		++first;
		code_point = (code_point - 0xd800) << 10;
		code_point |= next - 0xdc00;
		code_point += 0x10000;
	    }
	}
    }
    return code_point;
}

// ---------------------------------------------------------------------------
/// \class local_buffer
/// \brief Buffer that uses stack for small allocations and dynamic memory for larger ones.

template <class T, size_t default_size = ((255 - sizeof(T*)*2) / sizeof(T) + 1)>
class local_buffer
{
public:
    typedef T		value_type;
    typedef size_t	size_type;
    typedef T*          iterator;
    typedef const T*    const_iterator;

    local_buffer () : m_size (default_size) { m_ptr = m_buf; }

    explicit local_buffer (size_t initial_size)
	{
	    if (initial_size > default_size)
	    {
		m_ptr = new value_type[initial_size];
		m_size = initial_size;
	    }
	    else
	    {
		m_ptr = m_buf;
		m_size = default_size;
	    }
	}

    ~local_buffer () { if (m_ptr != m_buf) delete[] m_ptr; }

    /// make sure buffer is enough to hold SIZE objects, reallocating if necessary.
    /// old contents is lost after reallocation. 
    void reserve (size_t size)
	{
	    if (size > m_size)
	    {
		value_type* new_ptr = new value_type[size];
		if (m_ptr != m_buf) delete[] m_ptr;
		m_ptr = new_ptr;
		m_size = size;
	    }
	}

    size_type size () const { return m_size; }

    value_type* get () { return m_ptr; }
    const value_type* get () const { return m_ptr; }

    value_type& operator[] (std::ptrdiff_t i) { return m_ptr[i]; }
    const value_type& operator[] (std::ptrdiff_t i) const { return m_ptr[i]; }

private:

    local_buffer (const local_buffer&);		// not defined
    local_buffer& operator= (const local_buffer&);

    size_type		m_size;
    value_type*		m_ptr;
    value_type		m_buf[default_size];
};

} // namespace ext

// ---------------------------------------------------------------------------

#if defined(__MINGW32__) && !defined(LOCALE_INVARIANT)
#define LOCALE_INVARIANT                                                      \
          (MAKELCID(MAKELANGID(LANG_INVARIANT, SUBLANG_NEUTRAL), SORT_DEFAULT))
#endif

namespace icase {

inline int strcmp (const char* lhs, const char* rhs)
{
    return ::CompareStringA (LOCALE_INVARIANT, SORT_STRINGSORT|NORM_IGNORECASE,
                             lhs, -1, rhs, -1) - 2;
}

inline int strcmp (const wchar_t* lhs, const wchar_t* rhs)
{
    return ::CompareStringW (LOCALE_INVARIANT, SORT_STRINGSORT|NORM_IGNORECASE,
                             lhs, -1, rhs, -1) - 2;
}

inline int strncmp (const char* lhs, const char* rhs, size_t length)
{
    return ::CompareStringA (LOCALE_INVARIANT, SORT_STRINGSORT|NORM_IGNORECASE,
                             lhs, length, rhs, length) - 2;
}

inline int strncmp (const wchar_t* lhs, const wchar_t* rhs, size_t length)
{
    return ::CompareStringW (LOCALE_INVARIANT, SORT_STRINGSORT|NORM_IGNORECASE,
                             lhs, length, rhs, length) - 2;
}

/// locase
/// \brief functor providing conversion to lower case

struct locase
{
    char operator() (char s) const {
       	return std::char_traits<char>::to_char_type (std::tolower (std::char_traits<char>::to_int_type (s)));
    }
};

/// tolower
/// \brief overloaded functions for lower-case conversion

template <class Iterator>
inline Iterator tolower (Iterator first, Iterator last)
{
    return std::transform (first, last, first, icase::locase());
}

inline std::string& tolower (std::string& s)
{
    tolower (s.begin(), s.end());
    return s;
}

} // namespace icase

#endif /* EXT_STRINGUTIL_HPP */
