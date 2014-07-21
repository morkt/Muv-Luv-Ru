// -*- C++ -*-
//! \file       xami.cc
//! \date       Mon Feb 17 07:22:40 2014
//! \brief      exctract/convert muv-luv translation files.
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
#include "xami-config.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <Commctrl.h>
#include <Shlobj.h>
#include <Shlwapi.h>
#include <tchar.h>
#include "windres.h"
#include "logcontrol.hpp"
#include "fileutil.hpp"

namespace xami {

HINSTANCE   g_happ;
HWND        g_hwnd;
HFONT       g_dlg_font;
tstreambuf* g_clog_rdbuf;
std::vector<HICON> g_icons;

HICON
load_icon (int id, int width = 0, int height = 0)
{
    HICON icon = (HICON)::LoadImage (g_happ, MAKEINTRESOURCE(id), IMAGE_ICON, width, height, 0);
    if (icon)
        g_icons.push_back (icon);
    return icon;
}

void
destroy_icons ()
{
    std::for_each (g_icons.cbegin(), g_icons.cend(), ::DestroyIcon);
    g_icons.clear();
}

void
init_log_pane (HWND ctl)
{
    if (HDC screen = ::GetDC (0))
    {
        int dpi = ::GetDeviceCaps (screen, LOGPIXELSY);
        int font_height = -MulDiv (9, dpi, 72);
        ::ReleaseDC (0, screen);
        static HFONT s_font = ::CreateFont (font_height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                            CLEARTYPE_QUALITY, FIXED_PITCH|FF_MODERN, _T("Consolas"));
        ::SendMessage (ctl, WM_SETFONT, (WPARAM)s_font, FALSE);
    }
    TCLOG << _T("xAMI v1.00, Muv-Luv translation archive manipulation tool.\n")
             _T("© 2014 mørkt & the MuvLuvRu project, http://MuvLuvRu.wordpress.com\n");
}

void
init_font (HWND hWnd)
{
    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof(ncm);
    ::SystemParametersInfo (SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
    g_dlg_font = ::CreateFontIndirect (&ncm.lfMessageFont);
    set_default_font (hWnd);
}

void
init_dialog (HWND hWnd, const settings& config)
{
    if (HWND log = ::GetDlgItem (hWnd, IDC_LOG_PANE))
    {
        g_clog_rdbuf = TCLOG.rdbuf (new window_buf (log));
        init_log_pane (log);
    }
    init_font (hWnd);

    if (HICON icon = load_icon (IDI_BROWSE, 16, 16))
    {
        static const int button_ids[] = {
            IDC_SOURCE_AMI_BROWSE, IDC_TARGET_DIR_BROWSE,
            IDC_SOURCE_DIR_BROWSE, IDC_TARGET_AMI_BROWSE,
        };
        for (int i = 0; i < 4; ++i)
            ::SendDlgItemMessage (hWnd, button_ids[i], BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)icon);
    }
    if (HICON icon = load_icon (IDI_OPEN, 16, 16))
        ::SendDlgItemMessage (hWnd, IDC_EXTRACT, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)icon);
    if (HICON icon = load_icon (IDI_NEW_ARCHIVE, 16, 16))
        ::SendDlgItemMessage (hWnd, IDC_SAVE, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)icon);

    static const int ids[] = { IDC_SOURCE_AMI, IDC_TARGET_DIR, IDC_SOURCE_DIR, IDC_TARGET_AMI };
    for (int i = 0; i < 4; ++i)
        if (HWND ctl = ::GetDlgItem (hWnd, ids[i]))
            ::SHAutoComplete (ctl, SHACF_DEFAULT);

    ::SendDlgItemMessage (hWnd, IDC_SCRIPT_ENCODING, CB_ADDSTRING, 0, (LPARAM)_T("Shift-JIS"));
    ::SendDlgItemMessage (hWnd, IDC_SCRIPT_ENCODING, CB_ADDSTRING, 0, (LPARAM)_T("UTF-8"));
    ::SendDlgItemMessage (hWnd, IDC_SCRIPT_ENCODING, CB_SETCURSEL, 0, 0);

    ::SendDlgItemMessage (hWnd, IDC_IMAGE_FORMAT, CB_ADDSTRING, 0, (LPARAM)_T("PNG"));
    ::SendDlgItemMessage (hWnd, IDC_IMAGE_FORMAT, CB_ADDSTRING, 0, (LPARAM)_T("Raw GRP"));
    ::SendDlgItemMessage (hWnd, IDC_IMAGE_FORMAT, CB_SETCURSEL, 0, 0);

    import_settings (hWnd, config);
}

void
browse_input_file (int edit_id)
{
    TCHAR filename[MAX_PATH];
    ::GetDlgItemText (g_hwnd, edit_id, filename, MAX_PATH);

    OPENFILENAME ofn = { sizeof(ofn) };
    ofn.hwndOwner = g_hwnd;
    ofn.lpstrFilter = _T("Archives (*.ami,*.amr)\0*.ami;*.amr\0All Files (*.*)\0*.*\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = _T("Open source archive");
    ofn.Flags = OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt = _T("ami");
    if (::GetOpenFileName (&ofn))
    {
        ::SetDlgItemText (g_hwnd, edit_id, filename);
        if (IDC_SOURCE_AMI == edit_id && is_control_empty (g_hwnd, IDC_TARGET_AMI))
        {
            TCHAR* ext = _tcsrchr (ext::get_filename_part (filename), _T('.'));
            if (ext && (MAX_PATH - (ext - filename) >= 5)
                && 0 != icase::strcmp (ext, _T(".amr")))
            {
                _tcscpy (ext, _T(".amr"));
                ::SetDlgItemText (g_hwnd, IDC_TARGET_AMI, filename);
            }
        }
    }
}

void
browse_output_file (int edit_id)
{
    TCHAR filename[MAX_PATH];
    ::GetDlgItemText (g_hwnd, edit_id, filename, MAX_PATH);

    OPENFILENAME ofn = { sizeof(ofn) };
    ofn.hwndOwner = g_hwnd;
    ofn.lpstrFilter = _T("Archives (*.ami,*.amr)\0*.ami;*.amr\0All Files (*.*)\0*.*\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = _T("Choose target archive name");
    // ask about overwrite later, when "Save" action button is pressed
//    ofn.Flags = OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = _T("amr");
    if (::GetSaveFileName (&ofn))
        ::SetDlgItemText (g_hwnd, edit_id, filename);
}

static int CALLBACK
BrowseCallbackProc (HWND hwnd, UINT uMsg, LPARAM, LPARAM lpData)
{
    // If the BFFM_INITIALIZED message is received
    // set the path to the start path.
    if (BFFM_INITIALIZED == uMsg)
        if (lpData)
            ::SendMessage (hwnd, BFFM_SETSELECTION, TRUE, lpData);
    return 0; // The function should always return 0.
}

void
browse_folder (int edit_id)
{
    TCHAR display_path[MAX_PATH];
    TCHAR path[MAX_PATH];
    ::GetDlgItemText (g_hwnd, edit_id, path, MAX_PATH);

    BROWSEINFO bi = { g_hwnd };
    bi.pszDisplayName = display_path;
    if (IDC_TARGET_DIR == edit_id)
        bi.lpszTitle = _T("Choose folder where files should be extracted to:");
    else
        bi.lpszTitle = _T("Choose folder containing data files to be placed into archive:");
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    if (*path)
    {
        bi.lpfn = BrowseCallbackProc;
        bi.lParam = (LPARAM)path;
    }
    if (auto pidl = ::SHBrowseForFolder (&bi))
    {
        if (::SHGetPathFromIDList (pidl, path))
        {
            ::SetDlgItemText (g_hwnd, edit_id, path);
            if (IDC_TARGET_DIR == edit_id && is_control_empty (g_hwnd, IDC_SOURCE_DIR))
                ::SetDlgItemText (g_hwnd, IDC_SOURCE_DIR, path);
        }
        ::CoTaskMemFree (pidl);
    }
}

class control_flasher
{
    control_flasher () : m_ctl (0), m_bk (0) { }

public:
    static const COLORREF flash_color = RGB(255,100,100);

    enum { timer_id = 1000 };

    static control_flasher& instance ()
    {
        static control_flasher s_flasher;
        return s_flasher;
    }

    void activate (HWND ctl)
    {
        ::InvalidateRect (ctl, 0, TRUE);
        // (From MSDN) GetWindowLong
        // If the function succeeds, the return value is the previous value of the
        // specified integer.
        // If the function fails, the return value is zero. To get extended error
        // information, call GetLastError.
        // If the previous value of the specified integer is zero, and the function
        // succeeds, the return value is zero, but the function does not clear the last
        // error information. To deal with this, you should clear the last error
        // information by calling SetLastError with 0 before calling SetWindowLong. Then,
        // function failure will be indicated by a return value of zero and a GetLastError
        // result that is nonzero.
        ::SetLastError (0);
        const bool error = !::SetWindowLong (ctl, GWL_USERDATA, (LONG)this) && ::GetLastError();
        if (!error)
        {
            m_ctl = ctl;
            m_count = 5;
            if (!::SetTimer (ctl, timer_id, 100, TimerProc))
            {
                ::SetWindowLong (ctl, GWL_USERDATA, 0);
                m_ctl = 0;
            }
        }
    }

    bool active (HWND ctl) const { return ctl == m_ctl && ctl; }

    HBRUSH set_background (HDC hdc)
    {
        if (!m_bk)
            m_bk = ::CreateSolidBrush (flash_color);
        if (m_count & 1)
        {
            ::SetBkColor (hdc, flash_color);
            return m_bk;
        }
        return 0;
    }

    static void CALLBACK TimerProc (HWND hwnd, UINT, UINT_PTR idEvent, DWORD)
    {
        control_flasher* self = reinterpret_cast<control_flasher*> (::GetWindowLong (hwnd, GWL_USERDATA));
        if (self)
        {
            ::InvalidateRect (hwnd, 0, TRUE);
            if (self->m_ctl == hwnd && --self->m_count)
                ::SetTimer (hwnd, timer_id, 100, TimerProc);
            else
            {
                ::KillTimer (hwnd, idEvent);
                ::SetWindowLong (hwnd, GWL_USERDATA, 0);
                if (self->m_ctl == hwnd)
                    self->m_ctl = 0;
            }
        }
    }

private:
    HWND        m_ctl;
    unsigned    m_count;
    HBRUSH      m_bk;
};

void
flash_control (int id)
{
    if (HWND ctl = ::GetDlgItem (g_hwnd, id))
    {
        ::MessageBeep (MB_ICONHAND);
        ::SetFocus (ctl);
        control_flasher::instance().activate (ctl);
    }
}

class local_mem
{
    HLOCAL	m_handle;
public:
    explicit local_mem (HLOCAL handle) : m_handle (handle) {}
    ~local_mem () { if (m_handle) ::LocalFree (m_handle); }
};

tstring
get_error_text (int error_code)
{
    if (error_code != NO_ERROR)
    {
	TCHAR *msg_buf;
	if (::FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			     NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			     (LPTSTR) &msg_buf, 0, NULL))
	{
	    local_mem sentry (msg_buf);
            return tstring (msg_buf);
	}
    }
    return tstring (_T("No error"));
}

void
change_extract_button_state ()
{
    bool enable_extract = BST_CHECKED == ::IsDlgButtonChecked (g_hwnd, IDC_EXTRACT_TEXTS)
                       || BST_CHECKED == ::IsDlgButtonChecked (g_hwnd, IDC_EXTRACT_IMAGES);
    if (HWND extract_button = ::GetDlgItem (g_hwnd, IDC_EXTRACT))
        if (enable_extract != bool (::IsWindowEnabled (extract_button)))
            ::EnableWindow (extract_button, enable_extract);
}

void
process_dialog_messages (HWND hwnd)
{
    MSG msg;
    while (::PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (!::IsDialogMessage (hwnd, &msg))
        {
            ::TranslateMessage (&msg);
            ::DispatchMessage (&msg);
        }
    }
}

INT_PTR WINAPI
MainDialogProc (HWND hWnd, UINT msgId, WPARAM wParam, LPARAM lParam)
{
    (void)lParam;
    int handled = TRUE;
    switch (msgId)
    {
    case WM_INITDIALOG:
        g_hwnd = hWnd;
        init_dialog (hWnd, settings::instance());
        break;

    case WM_CLOSE:
        ::EndDialog (hWnd, 0);
	break;

    case WM_DESTROY:
        if (g_clog_rdbuf)
            delete TCLOG.rdbuf (g_clog_rdbuf);
        export_settings (hWnd, settings::instance());
        destroy_icons();
        break;

    case WM_COMMAND:
        switch (0xffff & wParam)
        {
        case IDC_SOURCE_AMI_BROWSE: browse_input_file (IDC_SOURCE_AMI); break;
        case IDC_TARGET_DIR_BROWSE: browse_folder (IDC_TARGET_DIR); break;
        case IDC_SOURCE_DIR_BROWSE: browse_folder (IDC_SOURCE_DIR); break;
        case IDC_TARGET_AMI_BROWSE: browse_output_file (IDC_TARGET_AMI); break;
        case IDC_EXTRACT: extract_files(); break;
        case IDC_SAVE: create_archive(); break;
        case IDC_EXTRACT_TEXTS:
        case IDC_EXTRACT_IMAGES: change_extract_button_state(); break;
        }
        break;

    case WM_CTLCOLOREDIT:
        if (control_flasher::instance().active ((HWND)lParam))
            return (INT_PTR)control_flasher::instance().set_background ((HDC)wParam);
        /* FALL-THROUGH */

    default:
        handled = FALSE;
    }
    return handled;
}

} // namespace xami

struct com_initialize
{
    explicit com_initialize (LPVOID reserved = NULL, DWORD init = COINIT_APARTMENTTHREADED)
        { ::CoInitializeEx (reserved, init); }
    ~com_initialize ()
        { ::CoUninitialize(); }
};

int WINAPI
WinMain (HINSTANCE hinst, HINSTANCE, LPSTR, int)
try
{
    xami::g_happ = hinst;
    com_initialize com_init;
    if (HMODULE comctl32 = ::LoadLibraryA ("comctl32.dll"))
    {
        typedef BOOL (WINAPI *init_controls_type) (const LPINITCOMMONCONTROLSEX lpInitCtrls);
        init_controls_type init_controls;
        init_controls = (init_controls_type)::GetProcAddress (comctl32, "InitCommonControlsEx");
        if (init_controls)
        {
            INITCOMMONCONTROLSEX controls = {
                sizeof(INITCOMMONCONTROLSEX),
                ICC_STANDARD_CLASSES
            };
            init_controls (&controls);
        }
    }
    xami::settings::instance().read();
    ::DialogBox (xami::g_happ, MAKEINTRESOURCE (IDD_MAIN), ::GetDesktopWindow(), xami::MainDialogProc);
    xami::settings::instance().save();
    return 0;
}
catch (std::exception& X)
{
    ::MessageBoxA (0, X.what(), "xAMI Exception", MB_OK);
    return 1;
}
