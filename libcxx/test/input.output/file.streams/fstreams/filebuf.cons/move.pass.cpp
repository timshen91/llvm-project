//===----------------------------------------------------------------------===//
//
// ΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚThe LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <fstream>

// template <class charT, class traits = char_traits<charT> >
// class basic_filebuf

// basic_filebuf(basic_filebuf&& rhs);

#include <fstream>
#include <cassert>

int main()
{
#ifdef _LIBCPP_MOVE
    {
        std::filebuf f;
        assert(f.open("test.dat", std::ios_base::out | std::ios_base::in
                                                     | std::ios_base::trunc) != 0);
        assert(f.is_open());
        assert(f.sputn("123", 3) == 3);
        f.pubseekoff(1, std::ios_base::beg);
        assert(f.sgetc() == '2');
        std::filebuf f2(move(f));
        assert(!f.is_open());
        assert(f2.is_open());
        assert(f2.sgetc() == '2');
    }
    remove("test.dat");
    {
        std::wfilebuf f;
        assert(f.open("test.dat", std::ios_base::out | std::ios_base::in
                                                     | std::ios_base::trunc) != 0);
        assert(f.is_open());
        assert(f.sputn(L"123", 3) == 3);
        f.pubseekoff(1, std::ios_base::beg);
        assert(f.sgetc() == L'2');
        std::wfilebuf f2(move(f));
        assert(!f.is_open());
        assert(f2.is_open());
        assert(f2.sgetc() == L'2');
    }
    remove("test.dat");
#endif
}
