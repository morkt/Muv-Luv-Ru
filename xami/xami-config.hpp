// -*- C++ -*-
//! \file       xami-config.hpp
//! \date       Wed Feb 19 02:59:20 2014
//! \brief      xAMI configuration class declaration.
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

#ifndef XAMI_CONFIG_HPP
#define XAMI_CONFIG_HPP

#include "xami.hpp"
#include "stringutil.hpp"
#include <sstream>
#include <tchar.h>

namespace xami {

struct settings
{
    int         window_x;
    int         window_y;
    tstring     extract_source_archive;
    tstring     extract_target_folder;
    tstring     extract_script_encoding;
    tstring     extract_image_format;
    bool        extract_texts;
    bool        extract_images;
    tstring     pack_source_folder;
    tstring     pack_target_archive;
    bool        copy_from_source_archive;

    bool read ();
    bool save () const;

    static settings& instance () 
    {
        static settings s_settings;
        return s_settings;
    }

private:
    settings () { }

    static bool create_config_dir ();

    static int read_int (const TCHAR* section, const TCHAR* key, int default_value)
    {
        return ::GetPrivateProfileInt (section, key, default_value, s_ini_file.c_str());
    }
    static tstring read_string (const TCHAR* section, const TCHAR* key, const tstring& default_value)
    {
        TCHAR buf[MAX_PATH];
        size_t rc = ::GetPrivateProfileString (section, key, default_value.c_str(),
                                               buf, MAX_PATH, s_ini_file.c_str());
        return tstring (buf, rc);
    }

    static bool write_value (const TCHAR* section, const TCHAR* name, const tstring& value)
    {
        return ::WritePrivateProfileString (section, name, value.c_str(), s_ini_file.c_str());
    }
    static bool write_value (const TCHAR* section, const TCHAR* name, bool value)
    {
        TCHAR bool_value[2] = { value ? _T('1') : _T('0'), 0 };
        return ::WritePrivateProfileString (section, name, bool_value, s_ini_file.c_str());
    }
    static bool write_value (const TCHAR* section, const TCHAR* name, int value)
    {
        ext::tostringstream out;
        out << value;
        return ::WritePrivateProfileString (section, name, out.str().c_str(), s_ini_file.c_str());
    }
    static void delete_value (const TCHAR* section, const TCHAR* name)
    {
        ::WritePrivateProfileString (section, name, 0, s_ini_file.c_str());
    }

    static tstring s_ini_file;
};

void export_settings (HWND hWnd, settings& config);
void import_settings (HWND hWnd, const settings& config);

} // namespace xami

#endif /* XAMI_CONFIG_HPP */
