//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <ios>

// template <class charT, class traits> class basic_ios

// iostate exceptions() const;

#include <ios>
#include <streambuf>
#include <cassert>

struct testbuf : public std::streambuf {};

int main()
{
    {
        const std::ios ios(0);
        assert(ios.exceptions() == std::ios::goodbit);
    }
    {
        testbuf sb;
        const std::ios ios(&sb);
        assert(ios.exceptions() == std::ios::goodbit);
    }
}
