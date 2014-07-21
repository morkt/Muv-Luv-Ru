// -*- C++ -*-
//! \file       xami.hpp
//! \date       Tue Feb 18 10:17:44 2014
//! \brief      xAMI global declarations.
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

#ifndef XAMI_HPP
#define XAMI_HPP

#include <windows.h>
#include "xami-types.hpp"
#include "windres.h"

namespace xami {

extern HINSTANCE    g_happ;
extern HWND         g_hwnd;
extern HFONT        g_dlg_font;

tstring get_error_text (int error_code);
void process_dialog_messages (HWND hwnd);
void flash_control (int id);

void extract_files ();
void create_archive ();

inline encoding_id
get_encoding ()
{
    LONG rc = ::SendDlgItemMessage (g_hwnd, IDC_SCRIPT_ENCODING, CB_GETCURSEL, 0, 0);
    return 1 == rc ? enc_utf8 : enc_shift_jis;
}

inline bool
is_control_empty (HWND hwnd, LONG id)
{
    if (HWND ctl = ::GetDlgItem (hwnd, id))
        return 0 == ::GetWindowTextLength (ctl);
    return true;
}

class send_message_recursive
{
    UINT    msg;
    WPARAM  wParam;
    LPARAM  lParam;

public:
    send_message_recursive (UINT m, WPARAM w, LPARAM l) : msg (m), wParam (w), lParam (l) { }
    void send (HWND hWnd) const
    {
        ::SendMessage (hWnd, msg, wParam, lParam);
        ::EnumChildWindows (hWnd, &send_message_recursive::callback, (LPARAM)this);
    }

private:
    static BOOL CALLBACK callback (HWND hwnd, LPARAM lParam)
    {
        reinterpret_cast<const send_message_recursive*> (lParam)->do_send (hwnd);
        return TRUE;
    }

    void do_send (HWND hWnd) const
    {
        if (IDC_LOG_PANE != ::GetWindowLong (hWnd, GWL_ID))
            ::SendMessage (hWnd, msg, wParam, lParam);
    }
};

inline void set_default_font (HWND hwnd)
{
    send_message_recursive set_font (WM_SETFONT, (WPARAM)g_dlg_font, 0);
    set_font.send (hwnd);
}

} // namespace xami

#endif /* XAMI_HPP */
