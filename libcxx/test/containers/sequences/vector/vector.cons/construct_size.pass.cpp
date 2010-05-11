//===----------------------------------------------------------------------===//
//
// ΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚThe LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// explicit vector(size_type n);

#include <vector>
#include <cassert>

#include "../../../DefaultOnly.h"

template <class C>
void
test(typename C::size_type n)
{
    C c(n);
    assert(c.__invariants());
    assert(c.size() == n);
    assert(c.get_allocator() == typename C::allocator_type());
#ifdef _LIBCPP_MOVE
    for (typename C::const_iterator i = c.cbegin(), e = c.cend(); i != e; ++i)
        assert(*i == typename C::value_type());
#endif
}

int main()
{
    test<std::vector<int> >(50);
    test<std::vector<DefaultOnly> >(500);
    assert(DefaultOnly::count == 0);
}
