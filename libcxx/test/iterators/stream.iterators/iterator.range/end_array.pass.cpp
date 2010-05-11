//===----------------------------------------------------------------------===//
//
// ΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚThe LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// template <class T, size_t N> T* end(T (&array)[N]);

#include <iterator>
#include <cassert>

int main()
{
    int ia[] = {1, 2, 3};
    int* i = std::begin(ia);
    int* e = std::end(ia);
    assert(e == ia + 3);
    assert(e - i == 3);
}
