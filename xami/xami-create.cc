// -*- C++ -*-
//! \file       xami-create.cc
//! \date       Tue Feb 18 18:13:35 2014
//! \brief      xAMI archive creation methods implementation.
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

#include "xami.hpp"
#include "xami-progress.hpp"
#include "xami-popup.hpp"
#include "tregex.hpp"
#include <map>
#include <fstream>
#include <iostream>
#include <cassert>
#include "ami-archive.hpp"
#include "mltcomp.hpp"
#include "fileutil.hpp"
#include "binio.h"

namespace xami {

using ext::tregex;

typedef std::map<unsigned, file_info> map_type;

template <class ScriptCompiler> size_t
convert_script (const tstring& input, std::ostream& out)
{
    std::ifstream in (input);
    if (!in)
    {
        int err = ::GetLastError();
        TCLOG << input << _T(": ");
        if (NO_ERROR != err)
            TCLOG << get_error_text (err);
        else
            TCLOG << _T("unable to open file.\n");
        return 0;
    }
    ScriptCompiler script;
    script.set_filename (input);
    if (!script.read_stream (in))
        return 0;
    return script.compile_data (out);
}

void
write_ami_header (const file_reader::content_type& content, std::ostream& out)
{
    assert (!content.empty() && "Empty AMI archive");
    out.write ("AMI", 4);
    bin::write32bit (out, content.size());
    bin::write32bit (out, content[0].offset);
    bin::write32bit (out, 0u);
    for (auto it = content.begin(); it != content.end(); ++it)
    {
        bin::write32bit (out, it->id);
        bin::write32bit (out, it->offset);
        bin::write32bit (out, it->unpacked_size);
        bin::write32bit (out, it->packed_size);
    }
}

void
write_ami_entry (const xami::file_info& file, xami::entry& entry, std::ostream& out)
{
    switch (file.type)
    {
    case xami::file_png:
        entry.unpacked_size = xami::convert_png (file.name, out, entry.packed_size);
        break;
    case xami::file_grp:
        entry.unpacked_size = xami::deflate_file (file.name, out, entry.packed_size);
        break;
    case xami::file_zgrp:
        entry.unpacked_size = xami::copy_zgrp (file.name, out);
        entry.packed_size = file.size - xami::ZGRP_HEADER_SIZE;
        break;
    case xami::file_mlt:
        entry.unpacked_size = convert_script<mlt_compiler> (file.name, out);
        entry.packed_size = 0;
        break;
    case xami::file_txt:
        entry.unpacked_size = convert_script<scr_compiler> (file.name, out);
        entry.packed_size = 0;
        break;
    default:
        entry.unpacked_size = xami::copy_file (file.name, out);
        entry.packed_size = 0;
    }
}

bool
create_from_scratch (const tstring& output, const map_type& input_map, progress_dialog& progress)
{
    const size_t count = input_map.size();
    assert (count && "No input files for archive");
    progress.set_max_range (count);

    file_reader::content_type content (count);
    std::ofstream out (output, std::ios::out|std::ios::binary|std::ios::trunc);
    if (!out)
        throw sys::file_error (output);

    uint32_t data_offset = count * 16 + 16;
    out.seekp (data_offset, std::ios::end);
    unsigned index = 0;
    for (auto it = input_map.begin(); it != input_map.end(); ++it)
    {
        progress.set_current_filename (it->second.name);
        content[index].id = it->first;
        content[index].offset = out.tellp();
        write_ami_entry (it->second, content[index], out);
        ++index;
        progress.step();
        process_dialog_messages (progress.hwnd());
        if (progress.aborted())
            return false;
    }
    out.seekp (0, std::ios::beg);
    write_ami_header (content, out);
    TCLOG << index << _T(" entries written.\n");
    return true;
}

bool
create_from_source (const tstring& input, const tstring& output, const map_type& input_map,
                    progress_dialog& progress)
{
    xami::file_reader ami_file (input.c_str());
    xami::file_reader::content_type content;
    ami_file.read_content (content);
    if (content.empty())
    {
        TCLOG << input << _T(": archive table of contents is empty.\n");
        return false;
    }
    progress.set_max_range (ami_file.count());

    std::ofstream out (output, std::ios::out|std::ios::binary|std::ios::trunc);
    if (!out)
        throw sys::file_error (output);

    uint32_t data_offset = ami_file.count() * 16 + 16;
    out.seekp (data_offset, std::ios::end);
    unsigned index = 0;
    unsigned update_count = 0;
    for (auto it = content.begin(); it != content.end(); ++it)
    {
        it->offset = out.tellp();
        auto replacement = input_map.find (it->id);
        if (replacement != input_map.end())
        {
            progress.set_current_filename (replacement->second.name);
            write_ami_entry (replacement->second, *it, out);
            ++update_count;
        }
        else
            ami_file.copy_to (index, out);
        ++index;
        progress.step();
        process_dialog_messages (progress.hwnd());
        if (progress.aborted())
            return false;
    }
    out.seekp (0, std::ios::beg);
    write_ami_header (content, out);
    TCLOG << index << _T(" entries written, ") << update_count << _T(" updated.\n");
    return true;
}

struct base_find_handle
{
    static bool close_handle (sys::raw_handle h)
    {
        return ::FindClose (h);
    }
};
typedef sys::generic_handle<sys::win_invalid_handle, base_find_handle> find_handle;

xami::file_type
get_file_type_from_ext (const tstring& ext)
{
    // extension is already matched by regexp,
    // so decide file type by the first symbol only.
    switch (ext[0])
    {
    case _T('P'): case _T('p'): return xami::file_png;
    case _T('G'): case _T('g'): return xami::file_grp;
    case _T('Z'): case _T('z'): return xami::file_zgrp;
    case _T('M'): case _T('m'): return xami::file_mlt;
    case _T('T'): case _T('t'): return xami::file_txt;
    default:                    return xami::file_raw;
    }
}

void
build_file_table (map_type& file_table)
{
    WIN32_FIND_DATA find_data;
    find_handle hdir (::FindFirstFile (_T("*"), &find_data));
    if (!hdir)
        return;
    static tregex name_re (_T("^(.+)\\.(png|mlt|scr|txt|grp|zgrp)$"),
                           tregex::ECMAScript|tregex::icase);
    do
    {
        if (find_data.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_DIRECTORY))
            continue;
        ext::tcmatch match;
        if (!regex_match (find_data.cFileName, match, name_re))
            continue;
        xami::file_type ftype = get_file_type_from_ext (match[2]);
        unsigned id = 0;
        if (file_txt == ftype)
            id = scr_compiler::get_id_from_file (find_data.cFileName);
        else
            id = _tcstoul (find_data.cFileName, 0, 16);
        if (!id)
            continue;

        if (find_data.nFileSizeHigh)
        {
            TCLOG << find_data.cFileName << _T(": file too long.\n");
            continue;
        }
        if (!find_data.nFileSizeLow)
        {
            TCLOG << find_data.cFileName << _T(": file is empty.\n");
            continue;
        }
        auto it = file_table.find (id);
        if (it != file_table.end())
        {
            if (1 != ::CompareFileTime (&find_data.ftLastWriteTime, &it->second.time))
                continue;
            it->second.assign (find_data, ftype);
        }
        else
            file_table[id].assign (find_data, ftype);
    }
    while (::FindNextFile (hdir, &find_data) != 0);
}

class temporary_file
{
    TCHAR temp_name[MAX_PATH];

public:
    temporary_file (const TCHAR* path, const TCHAR* prefix, UINT id = 0)
    {
        if (!::GetTempFileName (path, prefix, id, temp_name))
        {
            int err = ::GetLastError();
            TCLOG << _T("Unable to create temporary file. ") << get_error_text (err);
            throw sys::file_error (err, path);
        }
    }
    ~temporary_file () { ::DeleteFile (temp_name); }

    const TCHAR* name () const { return temp_name; }
};

void
create_archive ()
{
    TCHAR src_dir[MAX_PATH];
    if (!::GetDlgItemText (g_hwnd, IDC_SOURCE_DIR, src_dir, MAX_PATH))
    {
        TCLOG << _T("Specify source folder.\n");
        flash_control (IDC_SOURCE_DIR);
        return;
    }
    TCHAR dst_name[MAX_PATH];
    if (!::GetDlgItemText (g_hwnd, IDC_TARGET_AMI, dst_name, MAX_PATH))
    {
        TCLOG << _T("Specify target archive name.\n");
        flash_control (IDC_TARGET_AMI);
        return;
    }
    TCHAR original_name[MAX_PATH];
    const bool copy_from_source = BST_CHECKED == ::IsDlgButtonChecked (g_hwnd, IDC_MISSING_FILES);
    if (copy_from_source && !::GetDlgItemText (g_hwnd, IDC_SOURCE_AMI, original_name, MAX_PATH))
    {
        TCLOG << _T("Specify source archive.\n");
        flash_control (IDC_SOURCE_AMI);
        return;
    }
    if (!::SetCurrentDirectory (src_dir))
    {
        int err = ::GetLastError();
        TCLOG << src_dir << _T(": cannot access source directory.");
        if (ERROR_FILE_NOT_FOUND != err)
             TCLOG << _T(' ') << get_error_text (err);
        TCLOG << std::endl;
        flash_control (IDC_SOURCE_DIR);
        return;
    }
    if (copy_from_source && ext::is_same_file (original_name, dst_name))
    {
        TCLOG << _T("Destination and source archive should be different.\n");
        flash_control (IDC_TARGET_AMI);
        return;
    }
    try
    {
        map_type file_table;
        build_file_table (file_table);
        if (file_table.empty())
        {
            TCLOG << _T("Neither images nor text scripts found in source directory.\n");
            flash_control (IDC_SOURCE_DIR);
            return;
        }
        if (ext::file_exists (dst_name))
        {
            tstring text (tstring (_T("File ")) + dst_name + _T("\nalready exists.\n\nOverwrite?"));
            int rc = ::MessageBox (g_hwnd, text.c_str(), _T("Confirm overwrite"), MB_YESNO);
            if (IDYES != rc)
                return;
        }
        tstring temp_path (dst_name);
        size_t name_pos = temp_path.rfind ('\\');
        if (tstring::npos != name_pos && 0 != name_pos)
            temp_path.erase (name_pos, tstring::npos);
        else
            temp_path = _T('.');
        temporary_file tmp (temp_path.c_str(), _T("xami"));
        progress_dialog progress (g_hwnd, _T("Pack files"));
        progress.set_caption (_T("Archiving files into"));
        progress.set_archive_name (ext::get_filename_part (dst_name));
        progress.show();

        bool success;
        if (copy_from_source)
            success = create_from_source (original_name, tmp.name(), file_table, progress);
        else
            success = create_from_scratch (tmp.name(), file_table, progress);
        if (success && !::MoveFileEx (tmp.name(), dst_name, MOVEFILE_REPLACE_EXISTING))
            throw sys::file_error (dst_name);
    }
    catch (sys::generic_error& X)
    {
        TCLOG << X.get_description<TCHAR>() << std::endl;
        ::MessageBox (g_hwnd, X.get_description<TCHAR>(), _T("xAMI run-time error"), MB_OK|MB_ICONASTERISK);
    }
    catch (std::exception& X)
    {
        TCLOG << X.what() << std::endl;
        ::MessageBoxA (g_hwnd, X.what(), "xAMI run-time error", MB_OK|MB_ICONASTERISK);
    }
}

} // namespace xami
