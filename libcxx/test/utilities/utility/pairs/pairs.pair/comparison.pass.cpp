//===----------------------------------------------------------------------===//
//
// ΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚThe LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <utility>

// template <class T1, class T2> struct pair

// template <class T1, class T2> bool operator==(const pair<T1,T2>&, const pair<T1,T2>&);
// template <class T1, class T2> bool operator!=(const pair<T1,T2>&, const pair<T1,T2>&);
// template <class T1, class T2> bool operator< (const pair<T1,T2>&, const pair<T1,T2>&);
// template <class T1, class T2> bool operator> (const pair<T1,T2>&, const pair<T1,T2>&);
// template <class T1, class T2> bool operator>=(const pair<T1,T2>&, const pair<T1,T2>&);
// template <class T1, class T2> bool operator<=(const pair<T1,T2>&, const pair<T1,T2>&);

#include <utility>
#include <cassert>

int main()
{
    {
        typedef std::pair<int, short> P;
        P p1(3, 4);
        P p2(3, 4);
        assert( (p1 == p2));
        assert(!(p1 != p2));
        assert(!(p1 <  p2));
        assert( (p1 <= p2));
        assert(!(p1 >  p2));
        assert( (p1 >= p2));
    }
    {
        typedef std::pair<int, short> P;
        P p1(2, 4);
        P p2(3, 4);
        assert(!(p1 == p2));
        assert( (p1 != p2));
        assert( (p1 <  p2));
        assert( (p1 <= p2));
        assert(!(p1 >  p2));
        assert(!(p1 >= p2));
    }
    {
        typedef std::pair<int, short> P;
        P p1(3, 2);
        P p2(3, 4);
        assert(!(p1 == p2));
        assert( (p1 != p2));
        assert( (p1 <  p2));
        assert( (p1 <= p2));
        assert(!(p1 >  p2));
        assert(!(p1 >= p2));
    }
    {
        typedef std::pair<int, short> P;
        P p1(3, 4);
        P p2(2, 4);
        assert(!(p1 == p2));
        assert( (p1 != p2));
        assert(!(p1 <  p2));
        assert(!(p1 <= p2));
        assert( (p1 >  p2));
        assert( (p1 >= p2));
    }
    {
        typedef std::pair<int, short> P;
        P p1(3, 4);
        P p2(3, 2);
        assert(!(p1 == p2));
        assert( (p1 != p2));
        assert(!(p1 <  p2));
        assert(!(p1 <= p2));
        assert( (p1 >  p2));
        assert( (p1 >= p2));
    }
}
