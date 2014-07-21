// -*- C++ -*-
//! \file       xami-progress.hpp
//! \date       Tue Feb 18 18:11:43 2014
//! \brief      xAMI progress modal dialog class.
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

#ifndef XAMI_PROGRESS_HPP
#define XAMI_PROGRESS_HPP

#include "xami.hpp"
#include "windres.h"
#include <Commctrl.h>

namespace xami {

class progress_dialog
{
public:
    progress_dialog (HWND parent, const TCHAR* title);
    ~progress_dialog ()
    {
        ::EnableWindow (m_parent, TRUE);
        ::DestroyWindow (m_hwnd);
    }

    HWND hwnd () const { return m_hwnd; }
    bool aborted () const { return m_finish; }

    void show () { ::ShowWindow (m_hwnd, SW_SHOW); }

    void set_caption (const TCHAR* text)
        { ::SetDlgItemText (m_hwnd, IDC_PROGRESS_CAPTION, text); }
    void set_archive_name (const TCHAR* name)
        { ::SetDlgItemText (m_hwnd, IDC_PROGRESS_AMI, name); }
    void set_current_filename (const tstring& name)
        { ::SetDlgItemText (m_hwnd, IDC_PROGRESS_CURRENT, name.c_str()); }

    void reset ()
        { ::SendDlgItemMessage (m_hwnd, IDC_PROGRESS, PBM_SETPOS, 0, 0); }
    void set_max_range (unsigned value)
        { ::SendDlgItemMessage (m_hwnd, IDC_PROGRESS, PBM_SETRANGE32, 0, value); }
    void step ()
        { ::SendDlgItemMessage (m_hwnd, IDC_PROGRESS, PBM_STEPIT, 0, 0); }

private:
    static BOOL CALLBACK ProgressProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND    m_hwnd;
    HWND    m_parent;
    bool    m_finish;
};

} // namespace xami

#endif /* XAMI_PROGRESS_HPP */
