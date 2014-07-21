// -*- C++ -*-
//! \file       xami-popup.hpp
//! \date       Sat Feb 22 18:24:39 2014
//! \brief      xAMI popup windows interface.
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

#ifndef XAMI_POPUP_HPP
#define XAMI_POPUP_HPP

#include "xami.hpp"
#include "windres.h"

namespace xami {

class confirm_dialog
{
public:
    explicit confirm_dialog (HWND parent, const tstring& filename);
    ~confirm_dialog ()
    {
        ::EnableWindow (m_parent, TRUE);
        ::DestroyWindow (m_hwnd);
        if (m_background)
            ::DeleteObject (m_background);
    }

    // Returns: IDYES    if user has chosen 'Yes'
    //          IDNO     if user has chosen 'No'
    //          IDCANCEL if user has chosen 'Abort' or closed dialog window
    int run ();

    bool get_option () const
        { return BST_CHECKED == ::IsDlgButtonChecked (m_hwnd, IDC_APPLY_TO_ALL); }

private:
    static BOOL CALLBACK ConfirmProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void set_answer (int answer);

private:
    HWND    m_hwnd;
    HWND    m_parent;
    HBRUSH  m_background;
    int     m_answer;
    bool    m_end_dialog;
};

} // namespace xami

#endif /* XAMI_POPUP_HPP */
