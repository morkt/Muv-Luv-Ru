// -*- C++ -*-
//! \file       xami-types.hpp
//! \date       Tue Feb 18 23:01:46 2014
//! \brief      xAMI types declarations.
//

#ifndef XAMI_TYPES_HPP
#define XAMI_TYPES_HPP

#include "stringutil.hpp"

namespace xami {

using ext::tstring;
using ext::uint8_t;
using ext::uint16_t;
using ext::uint32_t;

enum encoding_id
{
    enc_shift_jis,
    enc_utf8,
    enc_default = enc_shift_jis,
};

} // namespace xami

#endif /* XAMI_TYPES_HPP */
