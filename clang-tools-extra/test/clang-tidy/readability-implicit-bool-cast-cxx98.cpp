// RUN: clang-tidy %s -checks=-*,readability-implicit-bool-cast -- -std=c++98

#include <cstddef> // for NULL

template<typename T>
void functionTaking(T);

struct Struct {
  int member;
};

void useOldNullMacroInReplacements() {
  int* pointer = NULL;
  functionTaking<bool>(pointer);
  // CHECK-MESSAGES: :[[@LINE-1]]:24: warning: implicit cast 'int *' -> bool [readability-implicit-bool-cast]
  // CHECK-FIXES: functionTaking<bool>(pointer != 0);

  int Struct::* memberPointer = NULL;
  functionTaking<bool>(!memberPointer);
  // CHECK-MESSAGES: :[[@LINE-1]]:25: warning: implicit cast 'int struct Struct::*' -> bool
  // CHECK-FIXES: functionTaking<bool>(memberPointer == 0);
}

void fixFalseLiteralConvertingToNullPointer() {
  functionTaking<int*>(false);
  // CHECK-MESSAGES: :[[@LINE-1]]:24: warning: implicit cast bool -> 'int *'
  // CHECK-FIXES: functionTaking<int*>(0);

  int* pointer = NULL;
  if (pointer == false) {}
  // CHECK-MESSAGES: :[[@LINE-1]]:18: warning: implicit cast bool -> 'int *'
  // CHECK-FIXES: if (pointer == 0) {}

  functionTaking<int Struct::*>(false);
  // CHECK-MESSAGES: :[[@LINE-1]]:33: warning: implicit cast bool -> 'int struct Struct::*'
  // CHECK-FIXES: functionTaking<int Struct::*>(0);

  int Struct::* memberPointer = NULL;
  if (memberPointer != false) {}
  // CHECK-MESSAGES: :[[@LINE-1]]:24: warning: implicit cast bool -> 'int struct Struct::*'
  // CHECK-FIXES: if (memberPointer != 0) {}
}
