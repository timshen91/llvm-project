//===----------------------------------------------------------------------===////// ΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚΚThe LLVM Compiler Infrastructure//// This file is distributed under the University of Illinois Open Source// License. See LICENSE.TXT for details.////===----------------------------------------------------------------------===//// type_traits// add_volatile#include <type_traits>template <class T, class U>void test_add_volatile_imp(){    static_assert((std::is_same<typename std::add_volatile<T>::type, volatile U>::value), "");}template <class T>void test_add_volatile(){    test_add_volatile_imp<T, volatile T>();    test_add_volatile_imp<const T, const volatile T>();    test_add_volatile_imp<volatile T, volatile T>();    test_add_volatile_imp<const volatile T, const volatile T>();}int main(){    test_add_volatile<void>();    test_add_volatile<int>();    test_add_volatile<int[3]>();    test_add_volatile<int&>();    test_add_volatile<const int&>();    test_add_volatile<int*>();    test_add_volatile<const int*>();}