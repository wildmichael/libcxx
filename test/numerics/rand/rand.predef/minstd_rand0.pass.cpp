//===----------------------------------------------------------------------===//
//
// ΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚThe LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// typedef linear_congruential_engine<uint_fast32_t, 16807, 0, 2147483647>
//                                                                 minstd_rand0;

#include <random>
#include <cassert>

int main()
{
    std::minstd_rand0 e;
    e.discard(9999);
    assert(e() == 1043618065u);
}
