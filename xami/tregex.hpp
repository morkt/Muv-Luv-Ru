// -*- C++ -*-
//! \file       tregex.hpp
//! \date       Tue Feb 18 15:55:49 2014
//! \brief      std::regex for TCHAR type.
//

#ifndef EXT_TREGEX_HPP
#define EXT_TREGEX_HPP

#include <regex>
#include "stringutil.hpp"

namespace ext {

typedef std::basic_regex<TCHAR>             tregex;
typedef std::match_results<const TCHAR*>    tcmatch;
typedef std::match_results<tstring::const_iterator> tsmatch;

using std::regex_match;
using std::regex_search;
using std::regex_replace;

} // namespace ext

#endif /* EXT_TREGEX_HPP */
