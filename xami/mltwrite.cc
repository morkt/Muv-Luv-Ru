// -*- C++ -*-
//! \file       mltwrite.cc
//! \date       Wed Feb 19 18:20:36 2014
//! \brief      convert SCR binary script into MLT text file.
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

#include "xami-util.hpp"
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iterator>
#include "mltcomp.hpp"
#include "tinyformat.h"

namespace xami {

const bool g_add_ru_line = true;

struct escape_char_default
{
    template <class CharT, class Copy>
    void operator() (std::string& out, CharT c, Copy copy_fun)
    {
        switch (c)
        {
        case '\n':   out += "\\n"; break; // New line
        case '\001': out += "\\e"; break; // End of line
        case '\002': out += "\\l"; break; // End of page
        case '\003': out += "\\p"; break; // Pause (slight delay)
        case '\022': out += "\\c"; break; // Choice start
        case '\023': out += "\\d"; break; // Choice end
        case '\x1e': out += "\\m"; break; // footnote
        case '\005': out += "\\r"; break; // Engine-controlled text speed
        default:     copy_fun (out, c);
        }
    }
};

struct escape_char_xml
{
    template <class CharT, class Copy>
    void operator() (std::string& out, CharT c, Copy copy_fun)
    {
        switch (c)
        {
//        case '\n':   out += "\\n"; break; // New line
        case '\001': out += "\\e"; break; // End of line
        case '\002': out += "\\l"; break; // End of page
        case '\003': out += "\\p"; break; // Pause (slight delay)
        case '\022': out += "\\c"; break; // Choice start
        case '\023': out += "\\d"; break; // Choice end
        case '\x1e': out += "\\m"; break; // footnote
        case '\005': out += "\\r"; break; // Engine-controlled text speed
        case '&':    out += "&amp;"; break;
        case '"':    out += "&quot;"; break;
//        case '\'':   out += "&apos;"; break;
        case '<':    out += "&lt;"; break;
        case '>':    out += "&gt;"; break;
        default:     copy_fun (out, c);
        }
    }
};

template <class EscapeChar = escape_char_default>
struct convert_string_raw
{
    EscapeChar escape_char;
    std::string operator () (const char* text, size_t size)
    {
        std::string out;
        out.reserve (size + 4);
        for (size_t i = 0; i < size; ++i)
        {
            char c = text[i];
            escape_char (out, c, [] (std::string& s, char c) { s += c; });
        }
        return out;
    }
};

template <class EscapeChar = escape_char_default>
struct convert_string_utf8
{
    EscapeChar escape_char;
    std::string operator () (const char* text, size_t size)
    {
        if (!size)
            return std::string();
        std::wstring wtext;
        if (!ext::mbstowcs (text, size, wtext, 932))
            throw std::runtime_error ("Cannot convert script from japanese Shift-JIS encoding to Unicode.");

        std::string out;
        for (auto it = wtext.cbegin(); it != wtext.cend(); )
        {
            // iterator IT is updated by u16tou32
            uint32_t c = ext::u16tou32 (it, wtext.cend());
            escape_char (out, c, [] (std::string& s, uint32_t c) {
                ext::u32tou8 (c, std::back_inserter (s));
            }); 
        }
        return out;
    }
};

class script_writer
{
public:
    void write_footer () { }

    encoding_id encoding () const { return m_enc; }

protected:
    script_writer (std::ostream& out, encoding_id enc)
        : m_out (out), m_enc (enc) { }

    std::ostream&   m_out;
    encoding_id     m_enc;
};

struct script_writer_mlt : script_writer
{
    script_writer_mlt (std::ostream& out, encoding_id enc) : script_writer (out, enc) { }

    void write_header (uint32_t file_id, uint32_t type_id, size_t count)
    {
        (void)file_id;
        tfm::format (m_out, "SCR %d %s\n%d\n", type_id, encoding_name<char>(m_enc), count);
    }
    void write_line (uint32_t id, const std::string& text)
    {
        m_out << '\n';
        print_line (id, "en", text);
        if (g_add_ru_line)
            print_line (id, "ru", text);
    }
    void print_line (uint32_t id, const char* lang_id, const std::string& text)
    {
        tfm::format (m_out, "[%06x|%s] %s\n", id, lang_id, text);
    }
};

struct script_writer_xml : script_writer
{
    script_writer_xml (std::ostream& out, encoding_id enc) : script_writer (out, enc) { }

    void write_header (uint32_t file_id, uint32_t type_id, size_t count)
    {
        const char* encoding = encoding_name<char> (m_enc);
        m_out << "<?xml version=\"1.0\" encoding=\"" << encoding << "\"?>\n"
                 "<!--Muv-Luv translation file-->\n"
                 "<script id=\"" << to_hex (file_id) << "\" type=\"" << type_id << "\">\n";
    }
    void write_line (uint32_t id, const std::string& text)
    {
        m_out << "<line id=\"" << to_hex (id) << "\">\n"
               "    <text language=\"en\">" << text << "</text>\n";
        if (g_add_ru_line)
            m_out << "    <text language=\"ru\">" << text << "</text>\n";
        m_out << "</line>\n";
    }
    void write_footer ()
    {
        m_out << "</script>\n";
    }
};

struct script_writer_txt : script_writer
{
    script_writer_txt (std::ostream& out, encoding_id enc) : script_writer (out, enc) { }

    void write_header (uint32_t file_id, uint32_t type_id, size_t count)
    {
        if (enc_utf8 == m_enc)
            m_out.write ("\xef\xbb\xbf", 3);
        tfm::format (m_out, "#FILENAME %08x\n#TYPE %d\n\n", file_id, type_id);
    }
    void write_line (uint32_t id, const std::string& text)
    {
        tfm::format (m_out, "//<%08x> %s\n<%08x> %s\n\n", id, text, id, text);
    }
};

template <class Writer, class EscapeString> bool
write_script (uint32_t file_id, const char* scr_data, size_t size,
              Writer writer, EscapeString escape_string)
{
    assert (size > 12 && "Invalid script data");
    const uint32_t* header = reinterpret_cast<const uint32_t*> (scr_data);
    uint32_t type_id = bin::little_dword (header[1]);
    size_t count = bin::little_dword (header[2]);
    writer.write_header (file_id, type_id, count);

    bool result = true;
    header += 3;
    const uint32_t* entry = header;
    for (size_t i = 0; i < count; ++i)
    {
        size_t offset = bin::little_dword (*entry++);
        size_t line_size = bin::little_dword (*entry++);
        uint32_t id = bin::little_dword (*entry++);
        if (offset >= size || line_size > size || line_size + offset > size)
        {
            TCLOG << to_hex (file_id) << _T(": invalid text script data for line [")
                  << to_hex (id) << _T("]\n");
            result = false;
            break;
        }
        std::string str = escape_string (&scr_data[offset], line_size);
        writer.write_line (id, str);
    }
    writer.write_footer();
    return result;
}

template <class Writer, class EscapeChar>
inline bool write_script_enc (std::ostream& out, uint32_t file_id, const char* scr_data,
                              size_t size, encoding_id enc)
{
    if (enc_utf8 != enc)
        return write_script (file_id, scr_data, size, Writer (out, enc), convert_string_raw<EscapeChar>());
    else
        return write_script (file_id, scr_data, size, Writer (out, enc), convert_string_utf8<EscapeChar>());
}

bool
write_script_mlt (std::ostream& out, uint32_t file_id, const char* scr_data, size_t size, encoding_id enc)
{
    return write_script_enc<script_writer_mlt, escape_char_default> (out, file_id, scr_data, size, enc);
}

bool
write_script_txt (std::ostream& out, uint32_t file_id, const char* scr_data, size_t size, encoding_id enc)
{
    return write_script_enc<script_writer_txt, escape_char_default> (out, file_id, scr_data, size, enc);
}

bool
write_script_xml (std::ostream& out, uint32_t file_id, const char* scr_data, size_t size, encoding_id enc)
{
    return write_script_enc<script_writer_xml, escape_char_xml> (out, file_id, scr_data, size, enc);
}

bool
write_script (const tstring& filename, uint32_t file_id, const char* scr_data, size_t size, encoding_id enc)
{
    std::ofstream out (filename, std::ios::out|std::ios::trunc);
    if (!out)
        return false;
    return write_script_mlt (out, file_id, scr_data, size, enc);
}

} // namespace xami
