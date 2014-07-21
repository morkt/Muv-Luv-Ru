// -*- C++ -*-
//! \file       scrcomp.cc
//! \date       Thu Feb 20 19:53:24 2014
//! \brief      Amaterasu Translations script file compiler.
//

#include "mltcomp.hpp"
#include <fstream>

int main (int argc, char* argv[])
try
{
    if (argc < 3)
    {
        std::cout << "usage: scrcomp [-w] TEXT-FILE OUT-FILE\n";
        return 0;
    }
    int arg = 1;
    std::ifstream in (argv[arg]);
    if (!in)
    {
        std::cerr << argv[arg] << ": can't open input file\n";
        return 1;
    }
    xami::scr_compiler scr;
    scr.set_filename (argv[arg]);
    if (!scr.read_stream (in))
    {
        std::cerr << argv[arg] << ": invalid text script\n";
        return 2;
    }
    ++arg;
    std::ofstream out (argv[arg], std::ios::out|std::ios::binary|std::ios::trunc);
    if (!out)
    {
        std::cerr << argv[arg] << ": can't open output file\n";
        return 3;
    }
    return !scr.compile_data (out);
}
catch (std::exception& X)
{
    std::cerr << "scrcomp: " << X.what() << '\n';
    return -1;
}
