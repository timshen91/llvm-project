//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_set>

// unordered_multiset(unordered_multiset&&)
//        noexcept(is_nothrow_move_constructible<allocator_type>::value &&
//                 is_nothrow_move_constructible<key_compare>::value);

// This tests a conforming extension

// UNSUPPORTED: c++98, c++03

#include <unordered_set>
#include <cassert>

#include "test_macros.h"
#include "MoveOnly.h"
#include "test_allocator.h"

template <class T>
struct some_comp
{
    typedef T value_type;
    some_comp(const some_comp&);
    bool operator()(const T&, const T&) const { return false; }
};

template <class T>
struct some_hash
{
    typedef T value_type;
    some_hash();
    some_hash(const some_hash&);
};

int main()
{
    {
        typedef std::unordered_multiset<MoveOnly> C;
        LIBCPP_STATIC_ASSERT(std::is_nothrow_move_constructible<C>::value, "");
    }
    {
        typedef std::unordered_multiset<MoveOnly, std::hash<MoveOnly>,
                           std::equal_to<MoveOnly>, test_allocator<MoveOnly>> C;
        LIBCPP_STATIC_ASSERT(std::is_nothrow_move_constructible<C>::value, "");
    }
    {
        typedef std::unordered_multiset<MoveOnly, std::hash<MoveOnly>,
                          std::equal_to<MoveOnly>, other_allocator<MoveOnly>> C;
        LIBCPP_STATIC_ASSERT(std::is_nothrow_move_constructible<C>::value, "");
    }
    {
        typedef std::unordered_multiset<MoveOnly, some_hash<MoveOnly>> C;
        static_assert(!std::is_nothrow_move_constructible<C>::value, "");
    }
    {
        typedef std::unordered_multiset<MoveOnly, std::hash<MoveOnly>,
                                                         some_comp<MoveOnly>> C;
        static_assert(!std::is_nothrow_move_constructible<C>::value, "");
    }
}
