//===----------------------------------------------------------------------===//
//
// ΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚThe LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// class time_base
// {
// public:
//     enum dateorder {no_order, dmy, mdy, ymd, ydm};
// };

#include <locale>
#include <cassert>

int main()
{
    std::time_base::dateorder d = std::time_base::no_order;
    assert(std::time_base::no_order == 0);
    assert(std::time_base::dmy == 1);
    assert(std::time_base::mdy == 2);
    assert(std::time_base::ymd == 3);
    assert(std::time_base::ydm == 4);
}
