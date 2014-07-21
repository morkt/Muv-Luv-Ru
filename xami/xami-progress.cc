// -*- C++ -*-
//! \file       xami-progress.cc
//! \date       Tue Feb 18 09:57:31 2014
//! \brief      xAMI progress modal dialog handling.
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

#include "xami-progress.hpp"

namespace xami {

progress_dialog::
progress_dialog (HWND parent, const TCHAR* title)
    : m_parent (parent)
    , m_finish (false)
{
    m_hwnd = ::CreateDialogParam (g_happ, MAKEINTRESOURCE (IDD_PROGRESS), m_parent,
                                  ProgressProc, (LPARAM)this);
    ::SetWindowText (m_hwnd, title);
    ::EnableWindow (m_parent, FALSE);
}

BOOL CALLBACK progress_dialog::
ProgressProc (HWND hWnd, UINT msgId, WPARAM wParam, LPARAM lParam)
{
    progress_dialog* self = reinterpret_cast<progress_dialog*> (::GetWindowLong (hWnd, GWL_USERDATA));
    int handled = TRUE;
    switch (msgId)
    {
    case WM_INITDIALOG:
        {
            self = reinterpret_cast<progress_dialog*> (lParam);
            ::SetWindowLong (hWnd, GWL_USERDATA, (LONG)self);
            ::SendDlgItemMessage (hWnd, IDC_PROGRESS, PBM_SETSTEP, 1, 0);
            set_default_font (hWnd);
            break;
        }

    case WM_CLOSE:
        ::EndDialog (hWnd, -1);
        self->m_finish = true;
	break;

    case WM_COMMAND:
        if (IDCANCEL == (0xffff & wParam))
        {
            ::EndDialog (hWnd, -1);
            self->m_finish = true;
        }
        break;

    default:
        handled = FALSE;
    }
    return handled;
}

} // namespace xami
