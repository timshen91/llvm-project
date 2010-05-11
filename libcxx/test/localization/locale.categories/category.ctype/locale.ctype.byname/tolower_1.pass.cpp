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

// charT tolower(charT) const;

#include <locale>
#include <cassert>

int main()
{
    {
        std::locale l("en_US");
        {
            typedef std::ctype<char> F;
            const F& f = std::use_facet<F>(l);
    
            assert(f.tolower(' ') == ' ');
            assert(f.tolower('A') == 'a');
            assert(f.tolower('\x07') == '\x07');
            assert(f.tolower('.') == '.');
            assert(f.tolower('a') == 'a');
            assert(f.tolower('1') == '1');
            assert(f.tolower('\xDA') == '\xDA');
            assert(f.tolower('\xFA') == '\xFA');
        }
    }
    {
        std::locale l("C");
        {
            typedef std::ctype<char> F;
            const F& f = std::use_facet<F>(l);
    
            assert(f.tolower(' ') == ' ');
            assert(f.tolower('A') == 'a');
            assert(f.tolower('\x07') == '\x07');
            assert(f.tolower('.') == '.');
            assert(f.tolower('a') == 'a');
            assert(f.tolower('1') == '1');
            assert(f.tolower('\xDA') == '\xDA');
            assert(f.tolower('\xFA') == '\xFA');
        }
    }
    {
        std::locale l("en_US");
        {
            typedef std::ctype<wchar_t> F;
            const F& f = std::use_facet<F>(l);
    
            assert(f.tolower(L' ') == L' ');
            assert(f.tolower(L'A') == L'a');
            assert(f.tolower(L'\x07') == L'\x07');
            assert(f.tolower(L'.') == L'.');
            assert(f.tolower(L'a') == L'a');
            assert(f.tolower(L'1') == L'1');
            assert(f.tolower(L'\xDA') == L'\xFA');
            assert(f.tolower(L'\xFA') == L'\xFA');
        }
    }
    {
        std::locale l("C");
        {
            typedef std::ctype<wchar_t> F;
            const F& f = std::use_facet<F>(l);
    
            assert(f.tolower(L' ') == L' ');
            assert(f.tolower(L'A') == L'a');
            assert(f.tolower(L'\x07') == L'\x07');
            assert(f.tolower(L'.') == L'.');
            assert(f.tolower(L'a') == L'a');
            assert(f.tolower(L'1') == L'1');
            assert(f.tolower(L'\xDA') == L'\xDA');
            assert(f.tolower(L'\xFA') == L'\xFA');
        }
    }
}
