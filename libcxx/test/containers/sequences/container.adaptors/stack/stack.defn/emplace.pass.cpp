//===----------------------------------------------------------------------===//
//
// ΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚThe LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <stack>

// template <class... Args> void emplace(Args&&... args);

#include <stack>
#include <cassert>

#include "../../../../Emplaceable.h"

int main()
{
#ifdef _LIBCPP_MOVE
    std::stack<Emplaceable> q;
    q.emplace(1, 2.5);
    q.emplace(2, 3.5);
    q.emplace(3, 4.5);
    assert(q.size() == 3);
    assert(q.top() == Emplaceable(3, 4.5));
#endif
}
