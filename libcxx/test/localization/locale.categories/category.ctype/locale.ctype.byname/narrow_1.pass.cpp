//===----------------------------------------------------------------------===//
//
// ΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚThe LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class charT> class ctype_byname;

// char narrow(charT c, char dfault) const;

#include <locale>
#include <cassert>

int main()
{
    {
        std::locale l(std::string("fr_CA.ISO8859-1"));
        {
            typedef std::ctype<wchar_t> F;
            const F& f = std::use_facet<F>(l);
    
            assert(f.narrow(L' ', '*') == ' ');
            assert(f.narrow(L'A', '*') == 'A');
            assert(f.narrow(L'\x07', '*') == '\x07');
            assert(f.narrow(L'.', '*') == '.');
            assert(f.narrow(L'a', '*') == 'a');
            assert(f.narrow(L'1', '*') == '1');
            assert(f.narrow(L'\xDA', '*') == '\xDA');
        }
    }
    {
        std::locale l("en_US");
        {
            typedef std::ctype<wchar_t> F;
            const F& f = std::use_facet<F>(l);
    
            assert(f.narrow(L' ', '*') == ' ');
            assert(f.narrow(L'A', '*') == 'A');
            assert(f.narrow(L'\x07', '*') == '\x07');
            assert(f.narrow(L'.', '*') == '.');
            assert(f.narrow(L'a', '*') == 'a');
            assert(f.narrow(L'1', '*') == '1');
            assert(f.narrow(L'\xDA', '*') == '*');
        }
    }
}
