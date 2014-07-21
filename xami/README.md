xAMI
====
Muv-Luv translation archive manipulation tool
---------------------------------------------

Unpacks "data.ami" file used by Amaterasu Translations in Muv-Luv and Muv-Luv Alternative visual novels.

Resulting files have seemingly random alphanumeric names that should be retained intact for the subsequent packing.

Scripts are converted into own MLT format. Its structure is rather obvious, just don't touch the first two lines and pay attention to escaped characters (\p, \n, \r etc). Advanced text editor with custom syntax highlighting helps a lot here. Commentary lines should start with semicolon ';'. Text could be encoded in either Shift-JIS or UTF-8 encodings. These are the only relevant MBCS encodings I know of that support both japanese and russian character sets simultaneously.

Images are converted into PNG format. There's a complexity concerning "floating" images (menu elements, various gfx popups etc). I'm too lazy to explain it in detail, just pay attention to 'oFFs' PNG chunk or deal with raw GRP format for yourself. Anyway, it doesn't matter for full size images (800x600 and more).

When packing files back into archive, in addition to the above xami recognizes text scripts used by Amaterasu Translations (like the ones accessible via https://www.assembla.com/code/ixrecMLtl/subversion/nodes/775).

That's about it. If you run into any trouble with it, always try to solve it yourself first rather than asking unnecessary questions (see "AS IS" clause below).

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

~ http://MuvLuvRu.wordpress.com
