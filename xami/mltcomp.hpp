// -*- C++ -*-
//! \file       mltcomp.hpp
//! \date       Tue Feb 18 17:29:08 2014
//! \brief      hand-made MLT script compiler declarations.
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

#ifndef XAMI_MLTCOMP_HPP
#define XAMI_MLTCOMP_HPP

#include <iostream>
#include <vector>
#include <map>
#include <boost/logic/tribool.hpp>
#include <tchar.h>
#include "xami-types.hpp"

namespace xami {

static const bool g_ignore_script_errors = true;

struct syntax_error : std::runtime_error
{
    syntax_error () : std::runtime_error ("Syntax error in text script.") { }
};

enum translation_id
{
    tr_ru,
    tr_en,
    tr_jp,
};

struct line_data
{
    unsigned    id;
    int         line_no;
    std::string text[3];

    translation_id get_lang () const
        { return text[tr_ru].empty() ? tr_en : tr_ru; }
    const std::string& get_text () const
        { return text[tr_ru].empty() ? text[tr_en] : text[tr_ru]; }
};

class scr_writer
{
protected:
    unsigned                scr_type;
    encoding_id             encoding;
    std::vector<unsigned>   text_id_data;
    std::map<unsigned, line_data>
                            text_map;
    tstring                 input_name;
    int                     line_no;
    bool                    ignore_errors;

public:
    explicit scr_writer (encoding_id enc = enc_shift_jis)
        : scr_type (0)
        , encoding (enc)
        , input_name (_T("<stdin>"))
        , ignore_errors (g_ignore_script_errors) {}

    void set_filename (tstring name) { input_name = std::move (name); }
    size_t compile_data (std::ostream& out) const;

protected:
    static bool skip_until_eol (std::istream& in)
    {
        while (in && in.get() != '\n')
            ;
        return !in.fail();
    }

    void convert_string (const std::string& input, std::string& out);
    void convert_string_utf8 (const std::string& input, std::string& out);

    void add_line (translation_id lang_id, const line_data& line);

    std::basic_ostream<TCHAR>& error_stream (int line) const
        { return TCLOG << input_name << _T(':') << line << _T(": "); }
    std::basic_ostream<TCHAR>& error_stream () const { return error_stream (line_no); }

    boost::tribool signal_error (std::istream& in) const
    {
        if (!ignore_errors)
            throw syntax_error();
        skip_until_eol (in);
        return boost::indeterminate;
    }
};

class mlt_compiler : public scr_writer
{
public:
    mlt_compiler () { }

    bool read_stream (std::istream& in);

private:
    // false = abort, true = valid line, indeterminate = ignore line
    boost::tribool read_line (std::istream& in, unsigned& line_id, std::string& line,
                              translation_id& lang_id);
};

class scr_compiler : public scr_writer
{
    unsigned                out_id;

public:
    explicit scr_compiler (encoding_id enc = enc_shift_jis)
        : scr_writer (enc), out_id (0)
        { }
    unsigned get_script_id () const { return out_id; }

    bool read_stream (std::istream& in);

    static unsigned get_id_from_file (const TCHAR* filename);

private:
    boost::tribool read_line (std::istream& in, unsigned& id, std::string& line);
    bool read_header (std::istream& in);
};

struct to_hex
{
    unsigned id;
    to_hex (unsigned i) : id (i) { }
};

template <typename CharT>
inline std::basic_ostream<CharT>& operator<< (std::basic_ostream<CharT>& out, const to_hex& pid)
{
    CharT c = out.fill (std::char_traits<CharT>::to_char_type ('0'));
    out.width (6);
    out << std::hex << pid.id << std::dec;
    out.fill (c);
    return out;
}

} // namespace xami

#endif /* XAMI_MLTCOMP_HPP */
