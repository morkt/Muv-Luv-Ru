// -*- C++ -*-
//! \file       xami-config.cc
//! \date       Wed Feb 19 03:00:21 2014
//! \brief      xAMI configuration class implementation.
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

#include "xami-config.hpp"
#include "xami-util.hpp"
#include <Shlobj.h>

namespace xami {

tstring settings::s_ini_file;

bool settings::
create_config_dir ()
{
    if (!s_ini_file.empty())
        return true;
    TCHAR buf[MAX_PATH];
    if (S_OK != ::SHGetFolderPath (NULL, CSIDL_APPDATA, NULL, 0, buf))
        return false;
    tstring config_path (buf);
    size_t buf_len = ::GetModuleFileName (0, buf, MAX_PATH);
    if (!buf_len)
        return false;
    if (_T('\\') != config_path.back())
        config_path += _T('\\');

    tstring self (buf, buf_len);
    size_t name_pos = self.rfind (_T('\\'));
    if (tstring::npos == name_pos)
        name_pos = 0;
    else
        name_pos++;
    size_t ext_pos = self.rfind (_T('.'));
    if (tstring::npos == ext_pos)
        ext_pos = self.size();

    config_path.append (self, name_pos, ext_pos-name_pos);
    ::CreateDirectory (config_path.c_str(), 0);

    config_path += _T('\\');
    s_ini_file = config_path + _T("settings.ini");
    return true;
}

bool settings::
read ()
{
    if (!create_config_dir())
        return false;

    window_x = read_int (_T("Window"), _T("X"), -1);
    window_y = read_int (_T("Window"), _T("Y"), -1);

    extract_source_archive = read_string (_T("Extract"), _T("SourceArchive"), extract_source_archive);
    extract_target_folder = read_string (_T("Extract"), _T("TargetFolder"), extract_target_folder);
    extract_script_encoding = read_string (_T("Extract"), _T("ScriptEncoding"),
                                           encoding_name<TCHAR> (enc_default));
    extract_image_format = read_string (_T("Extract"), _T("ImageFormat"), _T("PNG"));
    extract_texts = read_int (_T("Extract"), _T("ExtractTexts"), 1);
    extract_images = read_int (_T("Extract"), _T("ExtractImages"), 1);

    pack_source_folder = read_string (_T("Pack"), _T("SourceFolder"), pack_source_folder);
    pack_target_archive = read_string (_T("Pack"), _T("TargetArchive"), pack_target_archive);
    copy_from_source_archive = read_int (_T("Pack"), _T("CopyFromSource"), 1);

    return true;
}

bool settings::
save () const
{
    if (!create_config_dir())
        return false;

    if (!write_value (_T("Extract"), _T("SourceArchive"), extract_source_archive))
        return false;

    write_value (_T("Extract"), _T("TargetFolder"), extract_target_folder);
    if (!extract_script_encoding.empty())
        write_value (_T("Extract"), _T("ScriptEncoding"), extract_script_encoding);
    write_value (_T("Extract"), _T("ImageFormat"), extract_image_format);
    write_value (_T("Extract"), _T("ExtractTexts"), extract_texts);
    write_value (_T("Extract"), _T("ExtractImages"), extract_images);

    write_value (_T("Pack"), _T("SourceFolder"), pack_source_folder);
    write_value (_T("Pack"), _T("TargetArchive"), pack_target_archive);
    write_value (_T("Pack"), _T("CopyFromSource"), copy_from_source_archive);

    if (-1 != window_x && -1 != window_y)
    {
        write_value (_T("Window"), _T("X"), window_x);
        write_value (_T("Window"), _T("Y"), window_y);
    }
    return true;
}

void
export_settings (HWND hWnd, settings& config)
{
    TCHAR path[MAX_PATH];
    int rc = ::GetDlgItemText (hWnd, IDC_SOURCE_AMI, path, MAX_PATH);
    config.extract_source_archive.assign (path, rc);

    rc = ::GetDlgItemText (hWnd, IDC_TARGET_DIR, path, MAX_PATH);
    config.extract_target_folder.assign (path, rc);

    config.extract_texts = BST_CHECKED == ::IsDlgButtonChecked (hWnd, IDC_EXTRACT_TEXTS);
    config.extract_images = BST_CHECKED == ::IsDlgButtonChecked (hWnd, IDC_EXTRACT_IMAGES);

    rc = ::GetDlgItemText (hWnd, IDC_SOURCE_DIR, path, MAX_PATH);
    config.pack_source_folder.assign (path, rc);

    rc = ::GetDlgItemText (hWnd, IDC_TARGET_AMI , path, MAX_PATH);
    config.pack_target_archive.assign (path, rc);

    config.copy_from_source_archive = BST_CHECKED == ::IsDlgButtonChecked (hWnd, IDC_MISSING_FILES);

    RECT rect;
    if (::GetWindowRect (hWnd, &rect))
    {
        config.window_x = rect.left;
        config.window_y = rect.top;
    }
    else
    {
        config.window_x = -1;
        config.window_y = -1;
    }

    rc = ::SendDlgItemMessage (hWnd, IDC_SCRIPT_ENCODING, CB_GETCURSEL, 0, 0);
    switch (rc)
    {
    default:    config.extract_script_encoding = encoding_name<TCHAR> (enc_default); break;
    case 0:     config.extract_script_encoding = encoding_name<TCHAR> (enc_shift_jis); break;
    case 1:     config.extract_script_encoding = encoding_name<TCHAR> (enc_utf8); break;
    }
    rc = ::SendDlgItemMessage (hWnd, IDC_IMAGE_FORMAT, CB_GETCURSEL, 0, 0);
    switch (rc)
    {
    case 0: default:        config.extract_image_format = _T("PNG"); break;
    case 1:                 config.extract_image_format = _T("GRP"); break;
    }
}

void
import_settings (HWND hWnd, const settings& config)
{
    ::SetDlgItemText (hWnd, IDC_SOURCE_AMI, config.extract_source_archive.c_str());
    ::SetDlgItemText (hWnd, IDC_TARGET_DIR, config.extract_target_folder.c_str());
    ::SetDlgItemText (hWnd, IDC_SOURCE_DIR, config.pack_source_folder.c_str());
    ::SetDlgItemText (hWnd, IDC_TARGET_AMI, config.pack_target_archive.c_str());

    ::SendDlgItemMessage (hWnd, IDC_EXTRACT_TEXTS, BM_SETCHECK,
                          config.extract_texts ? BST_CHECKED : BST_UNCHECKED, 0);
    ::SendDlgItemMessage (hWnd, IDC_EXTRACT_IMAGES, BM_SETCHECK,
                          config.extract_images ? BST_CHECKED : BST_UNCHECKED, 0);

    if (!config.extract_script_encoding.empty())
    {
        encoding_id enc = encoding_from_name (config.extract_script_encoding);
        ::SendDlgItemMessage (hWnd, IDC_SCRIPT_ENCODING, CB_SETCURSEL, enc, 0);
    }
    if (!config.extract_image_format.empty())
    {
        int fmt = 1;
        if (_T("GRP") == config.extract_image_format)
            fmt = 2;
        ::SendDlgItemMessage (hWnd, IDC_IMAGE_FORMAT, CB_SETCURSEL, fmt-1, 0);
    }

    ::SendDlgItemMessage (hWnd, IDC_MISSING_FILES, BM_SETCHECK,
                          config.copy_from_source_archive ? BST_CHECKED : BST_UNCHECKED, 0);

    if (-1 != config.window_x && -1 != config.window_y)
        ::SetWindowPos (hWnd, HWND_TOP, config.window_x, config.window_y,
                        0, 0, SWP_NOSIZE|SWP_NOZORDER);
}

} // namespace xami
