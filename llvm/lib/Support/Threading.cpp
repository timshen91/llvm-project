//===-- llvm/Support/Threading.cpp- Control multithreading mode --*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements llvm_start_multithreaded() and friends.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Threading.h"
#include "llvm/Config/config.h"
#include "llvm/Support/Atomic.h"
#include "llvm/Support/Mutex.h"
#include <cassert>

using namespace llvm;

static bool multithreaded_mode = false;

static sys::Mutex* global_lock = 0;

bool llvm::llvm_start_multithreaded() {
#if LLVM_ENABLE_THREADS != 0
  assert(!multithreaded_mode && "Already multithreaded!");
  multithreaded_mode = true;
  global_lock = new sys::Mutex(true);

  // We fence here to ensure that all initialization is complete BEFORE we
  // return from llvm_start_multithreaded().
  sys::MemoryFence();
  return true;
#else
  return false;
#endif
}

void llvm::llvm_stop_multithreaded() {
#if LLVM_ENABLE_THREADS != 0
  assert(multithreaded_mode && "Not currently multithreaded!");

  // We fence here to insure that all threaded operations are complete BEFORE we
  // return from llvm_stop_multithreaded().
  sys::MemoryFence();

  multithreaded_mode = false;
  delete global_lock;
#endif
}

bool llvm::llvm_is_multithreaded() {
  return multithreaded_mode;
}

void llvm::llvm_acquire_global_lock() {
  if (multithreaded_mode) global_lock->acquire();
}

void llvm::llvm_release_global_lock() {
  if (multithreaded_mode) global_lock->release();
}

#if LLVM_ENABLE_THREADS != 0 && defined(HAVE_PTHREAD_H)
#include <pthread.h>

struct ThreadInfo {
  void (*UserFn)(void *);
  void *UserData;
};
static void *ExecuteOnThread_Dispatch(void *Arg) {
  ThreadInfo *TI = reinterpret_cast<ThreadInfo*>(Arg);
  TI->UserFn(TI->UserData);
  return 0;
}

void llvm::llvm_execute_on_thread(void (*Fn)(void*), void *UserData,
                                  unsigned RequestedStackSize) {
  ThreadInfo Info = { Fn, UserData };
  pthread_attr_t Attr;
  pthread_t Thread;

  // Construct the attributes object.
  if (::pthread_attr_init(&Attr) != 0)
    return;

  // Set the requested stack size, if given.
  if (RequestedStackSize != 0) {
    if (::pthread_attr_setstacksize(&Attr, RequestedStackSize) != 0)
      goto error;
  }

  // Construct and execute the thread.
  if (::pthread_create(&Thread, &Attr, ExecuteOnThread_Dispatch, &Info) != 0)
    goto error;

  // Wait for the thread and clean up.
  ::pthread_join(Thread, 0);

 error:
  ::pthread_attr_destroy(&Attr);
}
#elif LLVM_ENABLE_THREADS!=0 && defined(LLVM_ON_WIN32)
#include "Windows/WindowsSupport.h"
#include <process.h>

struct ThreadInfo {
  void (*func)(void*);
  void *param;
};

static unsigned __stdcall ThreadCallback(void *param) {
  struct ThreadInfo *info = reinterpret_cast<struct ThreadInfo *>(param);
  info->func(info->param);

  return 0;
}

void llvm::llvm_execute_on_thread(void (*Fn)(void*), void *UserData,
                                  unsigned RequestedStackSize) {
  struct ThreadInfo param = { Fn, UserData };

  HANDLE hThread = (HANDLE)::_beginthreadex(NULL,
                                            RequestedStackSize, ThreadCallback,
                                            &param, 0, NULL);

  if (hThread) {
    // We actually don't care whether the wait succeeds or fails, in
    // the same way we don't care whether the pthread_join call succeeds
    // or fails.  There's not much we could do if this were to fail. But
    // on success, this call will wait until the thread finishes executing
    // before returning.
    (void)::WaitForSingleObject(hThread, INFINITE);
    ::CloseHandle(hThread);
  }
}
#else
// Support for non-Win32, non-pthread implementation.
void llvm::llvm_execute_on_thread(void (*Fn)(void*), void *UserData,
                                  unsigned RequestedStackSize) {
  (void) RequestedStackSize;
  Fn(UserData);
}

#endif
