//===-- main.c --------------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct referent {
    const char *p;
};

int main (int argc, char const *argv[])
{
    const char *my_ptr = strdup("hello");
    struct referent *r = malloc(sizeof(struct referent));
    r->p = my_ptr;

    printf("%p\n", r); // break here

    return 0;
}
