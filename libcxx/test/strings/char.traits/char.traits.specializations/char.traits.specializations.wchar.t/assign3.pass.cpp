//===----------------------------------------------------------------------===//
//
// ΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚThe LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// template<> struct char_traits<wchar_t>

// static char_type* assign(char_type* s, size_t n, char_type a);

#include <string>
#include <cassert>

int main()
{
    wchar_t s1[] = {1, 2, 3};
    wchar_t s2[3] = {0};
    assert(std::char_traits<wchar_t>::assign(s2, 3, wchar_t(5)) == s2);
    assert(s2[0] == wchar_t(5));
    assert(s2[1] == wchar_t(5));
    assert(s2[2] == wchar_t(5));
}
