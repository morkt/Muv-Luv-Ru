// -*- C++ -*-
//! \file       xami-popup.cc
//! \date       Sat Feb 22 18:49:23 2014
//! \brief      overwrite confirmation popup dialog implementation.
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

#include "xami-popup.hpp"
#include <tchar.h>
#include <iostream>

namespace xami {

confirm_dialog::
confirm_dialog (HWND parent, const tstring& filename)
    : m_parent (parent)
    , m_background (0)
{
    m_background = ::CreateSolidBrush (RGB(255,255,255));
//    m_background = ::GetSysColorBrush (::GetSysColor (COLOR_WINDOW));
    m_hwnd = ::CreateDialogParam (g_happ, MAKEINTRESOURCE (IDD_CONFIRM_OVERWRITE), m_parent,
                                  ConfirmProc, (LPARAM)this);
    if (!m_hwnd)
        throw std::runtime_error ("Failed to create popup window.");
    tstring text (_T("File ")+filename+_T(" already exists."));
    ::SetDlgItemText (m_hwnd, IDC_CONFIRM_TEXT, text.c_str());
}

int confirm_dialog::
run ()
{
    ::EnableWindow (m_parent, FALSE);
    ::ShowWindow (m_hwnd, SW_SHOW);
    m_answer = IDCANCEL;
    m_end_dialog = false;
    MSG msg;
    BOOL ret;
    while (ret = ::GetMessage (&msg, NULL, 0, 0))
    {
        if (-1 == ret)
            break;
        if (!::IsDialogMessage (m_hwnd, &msg))
        {
            ::TranslateMessage (&msg);
            ::DispatchMessage (&msg);
        }
        if (m_end_dialog)
            break;
    }
    return m_answer;
}

void confirm_dialog::
set_answer (int answer)
{
    ::EndDialog (m_hwnd, answer);
    m_answer = answer;
    m_end_dialog = true;
}

BOOL CALLBACK confirm_dialog::
ConfirmProc (HWND hWnd, UINT msgId, WPARAM wParam, LPARAM lParam)
{
    confirm_dialog* self = reinterpret_cast<confirm_dialog*> (::GetWindowLong (hWnd, GWL_USERDATA));
    int handled = TRUE;
    switch (msgId)
    {
    case WM_INITDIALOG:
        self = reinterpret_cast<confirm_dialog*> (lParam);
        ::SetWindowLong (hWnd, GWL_USERDATA, (LONG)self);
        set_default_font (hWnd);
        break;

    case WM_CLOSE:
        self->set_answer (IDCANCEL);
	break;

    case WM_COMMAND:
        switch (0xffff & wParam)
        {
        case IDCANCEL:
        case IDYES:
        case IDNO:
            self->set_answer (0xffff & wParam);
            break;
        }
        break;

    case WM_CTLCOLORSTATIC:
        {
            switch (::GetDlgCtrlID ((HWND)lParam))
            {
            case IDC_CONFIRM_TEXT:
            case IDC_QUESTION_TEXT:
            case IDC_APPLY_TO_ALL:
                ::SetBkMode ((HDC)wParam, TRANSPARENT);
                ::SetTextColor ((HDC)wParam, RGB(0,0,0));
//                ::SetBkColor ((HDC)wParam, RGB(255,255,255));
                return (INT_PTR)self->m_background;
            }
        }
        /* FALL-THROUGH */

    default:
        handled = FALSE;
    }
    return handled;
}

} // namespace xami
