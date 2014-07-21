// -*- C++ -*-
//! \file       xami-extract.cc
//! \date       Tue Feb 18 18:10:13 2014
//! \brief      xAMI extract methods implementation.
//

#include "xami.hpp"
#include "xami-progress.hpp"
#include "xami-popup.hpp"
#include "ami-archive.hpp"
#include "fileutil.hpp"
#include "fstream.hpp"

namespace xami {

static const file_type g_default_script_format = file_mlt;

class gui_converter : public converter
{
public:
    gui_converter (progress_dialog* dlg)
        : m_progress (dlg), m_script_count (0), m_images_count (0)
        , m_encoding (get_encoding()), m_script_format (g_default_script_format)
        , m_image_format (file_png)
        , m_create_mode (sys::io::create_new), m_dont_ask_overwrite (false)
    {
        m_extract_texts = BST_CHECKED == ::IsDlgButtonChecked (g_hwnd, IDC_EXTRACT_TEXTS);
        m_extract_images = BST_CHECKED == ::IsDlgButtonChecked (g_hwnd, IDC_EXTRACT_IMAGES);
        int rc = ::SendDlgItemMessage (g_hwnd, IDC_IMAGE_FORMAT, CB_GETCURSEL, 0, 0);
        m_image_format = 1 == rc ? file_grp : file_png;
    }

    bool write_raw (uint32_t id, const char* buffer, size_t size);
    bool write_script (uint32_t id, const char* scr_data, size_t size);
    bool write_image (uint32_t id, const char* grp_data, size_t size);

    unsigned scripts () const { return m_script_count; }
    unsigned images () const { return m_images_count; }

    unsigned count () const { return m_script_count + m_images_count; }

    enum action
    {
        action_ok,
        action_skip,
        action_abort,
        action_failed,
    };

private:
    bool update_progress (const tstring& filename);

    template <class Writer>
    action write_file (uint32_t id, const TCHAR* ext, Writer writer, bool text_mode = false);

    action open_stream (sys::ofstream& out, const tstring& filename, bool text_mode = false);

private:
    progress_dialog*    m_progress;
    unsigned            m_script_count;
    unsigned            m_images_count;
    encoding_id         m_encoding;
    file_type           m_script_format;
    file_type           m_image_format;
    bool                m_extract_texts;
    bool                m_extract_images;
    sys::io::win_createmode m_create_mode;
    bool                m_dont_ask_overwrite;
};

bool gui_converter::
update_progress (const tstring& filename)
{
    m_progress->set_current_filename (filename);
    m_progress->step();
    process_dialog_messages (m_progress->hwnd());
    return !m_progress->aborted();
}

gui_converter::action gui_converter::
open_stream (sys::ofstream& out, const tstring& filename, bool text_mode)
{
    std::ios::openmode ios_mode = std::ios::out|(text_mode ? 0 : std::ios::binary);
    out.open (filename, ios_mode, m_create_mode);
    if (!out && !m_dont_ask_overwrite)
    {
        confirm_dialog confirm (m_progress->hwnd(), filename);
        int rc = confirm.run();
        if (IDCANCEL == rc)
            return action_abort;
        sys::io::win_createmode mode = IDYES == rc ? sys::io::create_always : sys::io::create_new;
        if (confirm.get_option())
        {
            m_create_mode = mode;
            m_dont_ask_overwrite = true;
        }
        if (IDNO == rc)
            return action_skip;
        out.open (filename, ios_mode, mode);
    }
    if (!out)
    {
        int err = ::GetLastError();
        TCLOG << filename << _T(": ") << get_error_text (err);
        return action_skip;
    }
    return action_ok;
}

template <class Writer>
gui_converter::action gui_converter::
write_file (uint32_t id, const TCHAR* ext, Writer writer, bool text_mode)
{
    tstring filename = format_filename (id, ext);
    if (!update_progress (filename))
        return action_abort;
    sys::ofstream out;
    action rc = open_stream (out, filename, text_mode);
    if (action_ok != rc)
        return rc;
    return writer (out) ? action_ok : action_failed;
}

bool gui_converter::
write_raw (uint32_t id, const char* buffer, size_t size)
{
    if (action_abort == write_file (id, _T("dat"), [=] (std::ostream& out) -> bool {
            return bool (out.write (buffer, size)); }))
        return false;
    return true;
}

bool gui_converter::
write_script (uint32_t id, const char* scr_data, size_t size)
{
    if (m_extract_texts)
    {
        action rc;
        switch (m_script_format)
        {
        default:
        case file_mlt:  rc = write_file (id, _T("mlt"), [=] (std::ostream& out) {
                            return xami::write_script_mlt (out, id, scr_data, size, m_encoding);
                        }, true);
                        break;
        case file_txt:  rc = write_file (id, _T("txt"), [=] (std::ostream& out) {
                            return xami::write_script_txt (out, id, scr_data, size, m_encoding);
                        }, true);
                        break;
        case file_xml:  rc = write_file (id, _T("xml"), [=] (std::ostream& out) {
                            return xami::write_script_xml (out, id, scr_data, size, m_encoding);
                        }, true);
                        break;
        }
        if (action_abort == rc)
            return false;
        if (action_ok == rc)
            ++m_script_count;
    }
    else
        m_progress->step();
    return true;
}

bool gui_converter::
write_image (uint32_t id, const char* grp_data, size_t size)
{
    if (m_extract_images)
    {
        const TCHAR* ext = file_png == m_image_format ? _T("png") : _T("grp");
        tstring filename = format_filename (id, ext);
        if (!update_progress (filename))
            return false;
        if (ext::file_exists (filename.c_str()))
        {
            if (m_dont_ask_overwrite)
            {
                if (sys::io::create_new == m_create_mode)
                {
                    TCLOG << filename << _T(": ") << get_error_text (ERROR_FILE_EXISTS);
                    return true;
                }
            }
            else
            {
                confirm_dialog confirm (m_progress->hwnd(), filename);
                int rc = confirm.run();
                if (IDCANCEL == rc)
                    return false;
                if (confirm.get_option())
                {
                    m_create_mode = IDYES == rc ? sys::io::create_always : sys::io::create_new;
                    m_dont_ask_overwrite = true;
                }
                if (IDNO == rc)
                    return true;
            }
        }
        if (file_png == m_image_format)
            xami::write_png (filename, grp_data, size);
        else
            xami::write_raw (filename, grp_data, size);
        ++m_images_count;
    }
    else
        m_progress->step();
    return true;
}

void
extraction_report (int total, const gui_converter& writer)
{
    int count = writer.count();
    if (1 == count)
    {
        TCLOG << _T("1 file extracted");
        if (writer.scripts())
            TCLOG << _T(" (script)");
        else if (writer.images())
            TCLOG << _T(" (image)");
    }
    else if (count > 1)
    {
        if (count != total)
            TCLOG << count << _T(" of ") << total << _T(" files extracted");
        else
        {
            TCLOG << count << _T(" files extracted");
            int items = 0;
            if (writer.scripts())
            {
                TCLOG << _T(" (") << writer.scripts() << _T(" scripts");
                ++items;
            }
            if (writer.images())
            {
                if (items) TCLOG << _T(", ");
                else       TCLOG << _T(" (");
                TCLOG << writer.images() << _T(" images)");
            }
            else if (items) TCLOG << _T(')');
        }
    }
    else
        TCLOG << _T("No files extracted");
    TCLOG << _T(".\n");
}

void
extract_files ()
{
    TCHAR src_name[MAX_PATH];
    size_t rc = ::GetDlgItemText (g_hwnd, IDC_SOURCE_AMI, src_name, MAX_PATH);
    if (!rc)
    {
        TCLOG << _T("Specify source archive.\n");
        flash_control (IDC_SOURCE_AMI);
        return;
    }
    TCHAR dst_path[MAX_PATH];
    rc = ::GetDlgItemText (g_hwnd, IDC_TARGET_DIR, dst_path, MAX_PATH);
    if (!rc)
    {
        TCLOG << _T("Specify where data should be extracted.\n");
        flash_control (IDC_TARGET_DIR);
        return;
    }
    if (!::SetCurrentDirectory (dst_path))
    {
        int err = ::GetLastError();
        TCLOG << dst_path << _T(": cannot access destination directory.");
        if (ERROR_FILE_NOT_FOUND != err)
             TCLOG << _T(' ') << get_error_text (err);
        TCLOG << std::endl;
        flash_control (IDC_TARGET_DIR);
        return;
    }
    TCLOG << _T("Extracting files from ") << src_name << _T("\nto ") << dst_path << std::endl;
    try
    {
        progress_dialog progress (g_hwnd, _T("Extract files"));
        xami::extractor<gui_converter> ami_file (src_name, &progress);
        unsigned total = ami_file.count();

        progress.set_max_range (total);
        progress.set_caption (_T("Extracting files from"));
        progress.set_archive_name (ext::get_filename_part (src_name));
        progress.show();

        int count = ami_file.extract();
        extraction_report (count, ami_file.writer());
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
