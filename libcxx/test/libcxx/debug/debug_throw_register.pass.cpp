// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// UNSUPPORTED: libcpp-no-exceptions

// Test that defining _LIBCPP_DEBUG_USE_EXCEPTIONS causes _LIBCPP_ASSERT
// to throw on failure.

#define _LIBCPP_DEBUG 1
#define _LIBCPP_DEBUG_USE_EXCEPTIONS

#include <cstdlib>
#include <exception>
#include <type_traits>
#include <__debug>
#include <cassert>

int main()
{
  try {
    _LIBCPP_ASSERT(false, "foo");
    assert(false);
  } catch (...) {}
}
