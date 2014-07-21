// -*- C++ -*-
//! \file       mltcomp.cc
//! \date       Tue Feb 18 17:19:47 2014
//! \brief      build SCR binary from supplied muv-luv translation text file.
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

#include "mltcomp.hpp"
#include "binio.h"
#include <fstream>
#include <cctype>

namespace xami {

struct lang
{
    translation_id lang_id;
    lang (translation_id id) : lang_id (id) { }
};

static bool g_warning = true;

ext::tostream& operator<< (ext::tostream& out, const lang& lng)
{
    switch (lng.lang_id)
    {
    case tr_ru: out << _T("ru"); break;
    case tr_en: out << _T("en"); break;
    case tr_jp: out << _T("jp"); break;
    }
    return out;
}

void scr_writer::
add_line (translation_id lang_id, const line_data& line)
{
    if (line.text[lang_id].empty())
    {
        error_stream (line.line_no) << _T("empty line for [")
            << to_hex (line.id) << _T('|') << lang (lang_id) << _T("] ignored.\n");
        return;
    }
    auto it = text_map.find (line.id);
    if (it != text_map.end())
    {
        if (!it->second.text[lang_id].empty())
            error_stream (line.line_no) << _T("duplicate line for [")
                << to_hex (line.id) << _T('|') << lang (lang_id) << _T("] ignored.\n");
        else
            it->second.text[lang_id] = line.text[lang_id];
    }
    else
    {
        text_map.insert (std::make_pair (line.id, line));
        text_id_data.push_back (line.id);
    }
}

void scr_writer::
convert_string (const std::string& input, std::string& out)
{
    out.reserve (input.size());
    for (auto p = input.begin(); p != input.end(); )
    {
        unsigned char c = *p++;
        if (p != input.end())
        {
            if ((c > 0x80 && c < 0xa0) || (c >= 0xe0 && c < 0xfd))
            {
                out.push_back (c);
                c = *p++;
            }
            else if ('\\' == c)
            {
                switch (*p++)
                {
                case 'n': c = '\n'; break;
                case 'e': c = '\001'; break;
                case 'l':
                case 'f': c = '\002'; break;
                case 'p': c = '\003'; break;
                case 'r': c = '\005'; break;
                case 'c': c = '\022'; break;
                case 'd': c = '\023'; break;
                case 'm': c = '\x1e'; break;
                default:
                    --p;
                    error_stream() << _T("invalid escape sequence.\n");
                    break;
                }
            }
            else if ('/' == c && '/' == *p)
                break;
        }
        out.push_back (c);
    }
}

void scr_writer::
convert_string_utf8 (const std::string& input, std::string& out)
{
    std::wstring wstr;
    for (auto p = input.begin(); p != input.end(); )
    {
	// P is updated by u8tou32
        uint32_t c = ext::u8tou32 (p, input.end());
        if (uint32_t(-1) == c || c > 0x10ffff)
        {
            error_stream() << _T("Invalid UTF-8 sequence.") << std::endl;
            throw std::runtime_error ("Invalid UTF-8 sequence.");
        }
        if (c < 0x10000)
        {
            if (p != input.end())
            {
                if ('\\' == c)
                {
                    switch (*p++)
                    {
                    case 'n': c = '\n'; break;
                    case 'e': c = '\001'; break;
                    case 'l':
                    case 'f': c = '\002'; break;
                    case 'p': c = '\003'; break;
                    case 'r': c = '\005'; break;
                    case 'c': c = '\022'; break;
                    case 'd': c = '\023'; break;
                    case 'm': c = '\x1e'; break;
                    default:
                        --p;
                        error_stream() << _T("invalid escape sequence.\n");
                        break;
                    }
                }
                else if ('/' == c && '/' == *p)
                    break;
            }
            wstr.push_back (static_cast<wchar_t> (c));
        }
        else
        {
            c -= 0x10000;
            wstr.push_back (static_cast<wchar_t> (0xd800 + (c >> 10)));
            wstr.push_back (static_cast<wchar_t> (0xdc00 + (c & 0x3ff)));
        }
    }
    ext::wcstombs (wstr, out, 932);
}

size_t scr_writer::
compile_data (std::ostream& out) const
{
    out.write ("SCR", 4);
    bin::write32bit (out, scr_type);
    bin::write32bit (out, text_id_data.size());
    uint32_t current_offset = 12 + 4 * 3 * text_id_data.size();
    for (auto it = text_id_data.begin(); it != text_id_data.end(); ++it)
    {
        auto ptxt = text_map.find (*it);
        if (g_warning)
        {
            auto lang_id = ptxt->second.get_lang();
            if (tr_ru != lang_id)
                error_stream (ptxt->second.line_no)
                    << _T("no russian line for [") << to_hex (*it) << _T("]\n");
        }
        size_t text_size = ptxt->second.get_text().size();
        bin::write32bit (out, current_offset);
        bin::write32bit (out, text_size);
        bin::write32bit (out, *it);
        current_offset += text_size + 1;
    }
    for (auto it = text_id_data.begin(); it != text_id_data.end(); ++it)
    {
        auto ptxt = text_map.find (*it);
        const std::string& text = ptxt->second.get_text();
        out.write (text.c_str(), text.size()+1);
    }
    return current_offset;
}

// ---------------------------------------------------------------------------
// MLT script interpreter

boost::tribool mlt_compiler::
read_line (std::istream& in, unsigned& line_id, std::string& line, translation_id& lang_id)
{
    char c;
    do // skip whitespace
    {
        if (!in.get (c))
            return false;
        if ('\n' == c) // skip blank lines
            return boost::indeterminate;
    }
    while (std::isspace (c));
    if (';' == c) // skip comments
    {
        return skip_until_eol (in) ? boost::indeterminate : boost::tribool(false);
    }
    if ('[' != c)
    {
        error_stream() << _T("syntax error (expected '[', got '") << c << _T("').\n");
        throw syntax_error();
    }

    in >> std::hex >> line_id;
    lang_id = tr_ru;
    if ('|' == in.peek())
    {
        in.get();
        std::string lang;
        while (in.get (c) && ']' != c)
        {
            if ('\n' == c)
            {
                error_stream() << _T("syntax error (unexpected end of line).\n");
                throw syntax_error();
            }
            lang.push_back (c);
        }
        if ("en" == lang)
            lang_id = tr_en;
        else if ("jp" == lang)
            lang_id = tr_jp;
        else if ("ru" != lang)
        {
            error_stream() << _T("unknown language identifier [") << lang.c_str() << _T("]\n");
            throw syntax_error();
        }
    }
    else if (!in.get (c) || ']' != c)
    {
        error_stream() << _T("syntax error (expected ']', got '") << c << _T("').\n");
        throw syntax_error();
    }
    if (' ' == in.peek())
        in.get();

    std::getline (in, line);
    return true;
}

bool mlt_compiler::
read_stream (std::istream& in)
{
    line_no = 1;
    encoding = enc_shift_jis;
    char sig[3];
    in.read (sig, 3);
    if ('\xef' == sig[0] && '\xbb' == sig[1] && '\xbf' == sig[2])
    {
        encoding = enc_utf8;
        in.read (sig, 3);
    }
    if (!in || 0 != std::memcmp (sig, "SCR", 3))
    {
        error_stream() << _T("invalid input file.\n");
        return false;
    }
    in >> scr_type;
    if (in.peek() != '\n')
    {
        std::string enc;
        in >> enc;
        icase::tolower (enc);
        if ("shift-jis" != enc)
        {
            if ("utf-8" == enc || "utf8" == enc)
                encoding = enc_utf8;
            else
            {
                error_stream() << _T("unknown encoding '") << enc.c_str() << _T("'.\n");
                return false;
            }
        }
    }
    skip_until_eol (in);
    ++line_no;
    unsigned total_lines;
    in >> total_lines;
    if (!skip_until_eol (in))
    {
        error_stream() << _T("unexpected end of file.\n");
        return false;
    }
    auto f_convert_string = enc_shift_jis == encoding ? &mlt_compiler::convert_string
                                                      : &mlt_compiler::convert_string_utf8;
    std::string line_text;
    while (in)
    {
        ++line_no;
        unsigned line_id;
        translation_id lang_id;
        auto result = read_line (in, line_id, line_text, lang_id);
        if (result)
        {
            line_data line = { line_id, line_no };
            (this->*f_convert_string) (line_text, line.text[lang_id]);
            add_line (lang_id, line);
        }
        else if (!result)
            break;
    }
//    std::cout << input_name << ": [" << sig << " " << scr_id << "] "
//              << text_id_data.size() << " entries parsed";
//    if (text_id_data.size() != total_lines)
//        std::cout << " (expected " << total_lines << ')';
//    std::cout << std::endl;
    if (text_id_data.size() != total_lines)
        TCLOG << input_name << _T(": expected ") << total_lines
            << _T(" lines, got ") << text_id_data.size() << _T(".\n");
    return true;
}

// ---------------------------------------------------------------------------
// TXT script interpreter

bool scr_compiler::
read_header (std::istream& in)
{
    std::string keyword;
    if (!(in >> keyword))
        return false;
    if ("FILENAME" == keyword)
        in >> std::hex >> out_id;
    else if ("TYPE" == keyword)
        in >> std::dec >> scr_type;
    skip_until_eol (in);
    return true;
}

boost::tribool scr_compiler::
read_line (std::istream& in, unsigned& line_id, std::string& line_text)
{
    char c;
    if (!in.get (c))
        return false;
    if ('\n' == c) // skip blank lines
        return boost::indeterminate;
    if ('#' == c)
    {
        read_header (in);
        return boost::indeterminate;
    }
    while (std::isspace (c)) // skip whitespace
    {
        if (!in.get (c))
            return false;
        if ('\n' == c) // skip blank lines
            return boost::indeterminate;
    }
    if ('/' == c && '/' == in.peek()) // skip comments
    {
        return skip_until_eol (in) ? boost::indeterminate : boost::tribool(false);
    }
    if ('<' != c)
    {
        error_stream() << _T("syntax error (expected '<', got '") << c << _T("').\n");
        return signal_error (in);
    }

    in >> std::hex >> line_id;
    if (!in.get (c) || '>' != c)
    {
        error_stream() << _T("syntax error (expected '>', got '") << c << _T("').\n");
        return signal_error (in);
    }
    if (' ' == in.peek())
        in.get();

    std::getline (in, line_text);
    return true;
}

bool scr_compiler::
read_stream (std::istream& in)
{
    line_no = 1;
    if ('\xef' == in.peek())
    {
        in.get();
        if ('\xbb' == in.get() && '\xbf' == in.get())
            encoding = enc_utf8;
        else
        {
            error_stream() << _T("invalid input file.\n");
            return false;
        }
    }
    if ('#' != in.peek())
    {
        error_stream() << _T("invalid input file.\n");
        return false;
    }
    auto f_convert_string = enc_shift_jis == encoding ? &scr_compiler::convert_string
                                                      : &scr_compiler::convert_string_utf8;
    std::string line_text;
    while (in)
    {
        unsigned line_id;
        auto result = read_line (in, line_id, line_text);
        if (result)
        {
            line_data line = { line_id, line_no };
            (this->*f_convert_string) (line_text, line.text[tr_ru]);
            add_line (tr_ru, line);
        }
        else if (!result)
            break;
        ++line_no;
    }
//    std::cout << input_name << ": [" << sig << " " << scr_id << "] "
//              << text_id_data.size() << " entries parsed\n";
    return true;
}

unsigned scr_compiler::
get_id_from_file (const TCHAR* filename)
{
    std::ifstream in (filename);
    char c;
    if (!in || !in.get (c))
        return 0;
    if ('\xef' == c)
    {
        if ('\xbb' != in.get() || '\xbf' != in.get())
            return 0;
        in.get (c);
    }
    if ('#' != c)
        return 0;
    char keyword[8];
    if (!in.read (keyword, 8) || 0 != std::memcmp (keyword, "FILENAME", 8))
        return 0;
    unsigned id;
    in >> std::hex >> id;
    return id;
}

} // namespace xami
