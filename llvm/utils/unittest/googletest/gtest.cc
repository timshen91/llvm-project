// Copyright 2005, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: wan@google.com (Zhanyong Wan)
//
// The Google C++ Testing Framework (Google Test)

#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>

#include <algorithm>
#include <ostream>
#include <sstream>
#include <vector>

#if GTEST_OS_LINUX

// TODO(kenton@google.com): Use autoconf to detect availability of
// gettimeofday().
#define GTEST_HAS_GETTIMEOFDAY_ 1

#include <fcntl.h>
#include <limits.h>
#include <sched.h>
// Declares vsnprintf().  This header is not available on Windows.
#include <strings.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#include <string>
#include <vector>

#elif GTEST_OS_SYMBIAN
#define GTEST_HAS_GETTIMEOFDAY_ 1
#include <sys/time.h>  // NOLINT

#elif GTEST_OS_ZOS
#define GTEST_HAS_GETTIMEOFDAY_ 1
#include <sys/time.h>  // NOLINT

// On z/OS we additionally need strings.h for strcasecmp.
#include <strings.h>  // NOLINT

#elif GTEST_OS_WINDOWS_MOBILE  // We are on Windows CE.

#include <windows.h>  // NOLINT

#elif GTEST_OS_WINDOWS  // We are on Windows proper.

#include <io.h>  // NOLINT
#include <sys/timeb.h>  // NOLINT
#include <sys/types.h>  // NOLINT
#include <sys/stat.h>  // NOLINT

#if GTEST_OS_WINDOWS_MINGW
// MinGW has gettimeofday() but not _ftime64().
// TODO(kenton@google.com): Use autoconf to detect availability of
//   gettimeofday().
// TODO(kenton@google.com): There are other ways to get the time on
//   Windows, like GetTickCount() or GetSystemTimeAsFileTime().  MinGW
//   supports these.  consider using them instead.
#define GTEST_HAS_GETTIMEOFDAY_ 1
#include <sys/time.h>  // NOLINT
#endif  // GTEST_OS_WINDOWS_MINGW

// cpplint thinks that the header is already included, so we want to
// silence it.
#include <windows.h>  // NOLINT

#else

// Assume other platforms have gettimeofday().
// TODO(kenton@google.com): Use autoconf to detect availability of
//   gettimeofday().
#define GTEST_HAS_GETTIMEOFDAY_ 1

// cpplint thinks that the header is already included, so we want to
// silence it.
#include <sys/time.h>  // NOLINT
#include <unistd.h>  // NOLINT

#endif  // GTEST_OS_LINUX

#if GTEST_HAS_EXCEPTIONS
#include <stdexcept>
#endif

// Indicates that this translation unit is part of Google Test's
// implementation.  It must come before gtest-internal-inl.h is
// included, or there will be a compiler error.  This trick is to
// prevent a user from accidentally including gtest-internal-inl.h in
// his code.
#define GTEST_IMPLEMENTATION_ 1
#include "gtest/internal/gtest-internal-inl.h"
#undef GTEST_IMPLEMENTATION_

#if GTEST_OS_WINDOWS
#define vsnprintf _vsnprintf
#endif  // GTEST_OS_WINDOWS

namespace testing {

using internal::CountIf;
using internal::ForEach;
using internal::GetElementOr;
using internal::Shuffle;

// Constants.

// A test whose test case name or test name matches this filter is
// disabled and not run.
static const char kDisableTestFilter[] = "DISABLED_*:*/DISABLED_*";

// A test case whose name matches this filter is considered a death
// test case and will be run before test cases whose name doesn't
// match this filter.
static const char kDeathTestCaseFilter[] = "*DeathTest:*DeathTest/*";

// A test filter that matches everything.
static const char kUniversalFilter[] = "*";

// The default output file for XML output.
static const char kDefaultOutputFile[] = "test_detail.xml";

// The environment variable name for the test shard index.
static const char kTestShardIndex[] = "GTEST_SHARD_INDEX";
// The environment variable name for the total number of test shards.
static const char kTestTotalShards[] = "GTEST_TOTAL_SHARDS";
// The environment variable name for the test shard status file.
static const char kTestShardStatusFile[] = "GTEST_SHARD_STATUS_FILE";

namespace internal {

// The text used in failure messages to indicate the start of the
// stack trace.
const char kStackTraceMarker[] = "\nStack trace:\n";

// g_help_flag is true iff the --help flag or an equivalent form is
// specified on the command line.
bool g_help_flag = false;

}  // namespace internal

GTEST_DEFINE_bool_(
    also_run_disabled_tests,
    internal::BoolFromGTestEnv("also_run_disabled_tests", false),
    "Run disabled tests too, in addition to the tests normally being run.");

GTEST_DEFINE_bool_(
    break_on_failure,
    internal::BoolFromGTestEnv("break_on_failure", false),
    "True iff a failed assertion should be a debugger break-point.");

GTEST_DEFINE_bool_(
    catch_exceptions,
    internal::BoolFromGTestEnv("catch_exceptions", false),
    "True iff " GTEST_NAME_
    " should catch exceptions and treat them as test failures.");

GTEST_DEFINE_string_(
    color,
    internal::StringFromGTestEnv("color", "auto"),
    "Whether to use colors in the output.  Valid values: yes, no, "
    "and auto.  'auto' means to use colors if the output is "
    "being sent to a terminal and the TERM environment variable "
    "is set to xterm, xterm-color, xterm-256color, linux or cygwin.");

GTEST_DEFINE_string_(
    filter,
    internal::StringFromGTestEnv("filter", kUniversalFilter),
    "A colon-separated list of glob (not regex) patterns "
    "for filtering the tests to run, optionally followed by a "
    "'-' and a : separated list of negative patterns (tests to "
    "exclude).  A test is run if it matches one of the positive "
    "patterns and does not match any of the negative patterns.");

GTEST_DEFINE_bool_(list_tests, false,
                   "List all tests without running them.");

GTEST_DEFINE_string_(
    output,
    internal::StringFromGTestEnv("output", ""),
    "A format (currently must be \"xml\"), optionally followed "
    "by a colon and an output file name or directory. A directory "
    "is indicated by a trailing pathname separator. "
    "Examples: \"xml:filename.xml\", \"xml::directoryname/\". "
    "If a directory is specified, output files will be created "
    "within that directory, with file-names based on the test "
    "executable's name and, if necessary, made unique by adding "
    "digits.");

GTEST_DEFINE_bool_(
    print_time,
    internal::BoolFromGTestEnv("print_time", true),
    "True iff " GTEST_NAME_
    " should display elapsed time in text output.");

GTEST_DEFINE_int32_(
    random_seed,
    internal::Int32FromGTestEnv("random_seed", 0),
    "Random number seed to use when shuffling test orders.  Must be in range "
    "[1, 99999], or 0 to use a seed based on the current time.");

GTEST_DEFINE_int32_(
    repeat,
    internal::Int32FromGTestEnv("repeat", 1),
    "How many times to repeat each test.  Specify a negative number "
    "for repeating forever.  Useful for shaking out flaky tests.");

GTEST_DEFINE_bool_(
    show_internal_stack_frames, false,
    "True iff " GTEST_NAME_ " should include internal stack frames when "
    "printing test failure stack traces.");

GTEST_DEFINE_bool_(
    shuffle,
    internal::BoolFromGTestEnv("shuffle", false),
    "True iff " GTEST_NAME_
    " should randomize tests' order on every run.");

GTEST_DEFINE_int32_(
    stack_trace_depth,
    internal::Int32FromGTestEnv("stack_trace_depth", kMaxStackTraceDepth),
    "The maximum number of stack frames to print when an "
    "assertion fails.  The valid range is 0 through 100, inclusive.");

GTEST_DEFINE_bool_(
    throw_on_failure,
    internal::BoolFromGTestEnv("throw_on_failure", false),
    "When this flag is specified, a failed assertion will throw an exception "
    "if exceptions are enabled or exit the program with a non-zero code "
    "otherwise.");

namespace internal {

// Generates a random number from [0, range), using a Linear
// Congruential Generator (LCG).  Crashes if 'range' is 0 or greater
// than kMaxRange.
UInt32 Random::Generate(UInt32 range) {
  // These constants are the same as are used in glibc's rand(3).
  state_ = (1103515245U*state_ + 12345U) % kMaxRange;

  GTEST_CHECK_(range > 0)
      << "Cannot generate a number in the range [0, 0).";
  GTEST_CHECK_(range <= kMaxRange)
      << "Generation of a number in [0, " << range << ") was requested, "
      << "but this can only generate numbers in [0, " << kMaxRange << ").";

  // Converting via modulus introduces a bit of downward bias, but
  // it's simple, and a linear congruential generator isn't too good
  // to begin with.
  return state_ % range;
}

// GTestIsInitialized() returns true iff the user has initialized
// Google Test.  Useful for catching the user mistake of not initializing
// Google Test before calling RUN_ALL_TESTS().
//
// A user must call testing::InitGoogleTest() to initialize Google
// Test.  g_init_gtest_count is set to the number of times
// InitGoogleTest() has been called.  We don't protect this variable
// under a mutex as it is only accessed in the main thread.
int g_init_gtest_count = 0;
static bool GTestIsInitialized() { return g_init_gtest_count != 0; }

// Iterates over a vector of TestCases, keeping a running sum of the
// results of calling a given int-returning method on each.
// Returns the sum.
static int SumOverTestCaseList(const std::vector<TestCase*>& case_list,
                               int (TestCase::*method)() const) {
  int sum = 0;
  for (size_t i = 0; i < case_list.size(); i++) {
    sum += (case_list[i]->*method)();
  }
  return sum;
}

// Returns true iff the test case passed.
static bool TestCasePassed(const TestCase* test_case) {
  return test_case->should_run() && test_case->Passed();
}

// Returns true iff the test case failed.
static bool TestCaseFailed(const TestCase* test_case) {
  return test_case->should_run() && test_case->Failed();
}

// Returns true iff test_case contains at least one test that should
// run.
static bool ShouldRunTestCase(const TestCase* test_case) {
  return test_case->should_run();
}

// AssertHelper constructor.
AssertHelper::AssertHelper(TestPartResult::Type type,
                           const char* file,
                           int line,
                           const char* message)
    : data_(new AssertHelperData(type, file, line, message)) {
}

AssertHelper::~AssertHelper() {
  delete data_;
}

// Message assignment, for assertion streaming support.
void AssertHelper::operator=(const Message& message) const {
  UnitTest::GetInstance()->
    AddTestPartResult(data_->type, data_->file, data_->line,
                      AppendUserMessage(data_->message, message),
                      UnitTest::GetInstance()->impl()
                      ->CurrentOsStackTraceExceptTop(1)
                      // Skips the stack frame for this function itself.
                      );  // NOLINT
}

// Mutex for linked pointers.
GTEST_DEFINE_STATIC_MUTEX_(g_linked_ptr_mutex);

// Application pathname gotten in InitGoogleTest.
String g_executable_path;

// Returns the current application's name, removing directory path if that
// is present.
FilePath GetCurrentExecutableName() {
  FilePath result;

#if GTEST_OS_WINDOWS
  result.Set(FilePath(g_executable_path).RemoveExtension("exe"));
#else
  result.Set(FilePath(g_executable_path));
#endif  // GTEST_OS_WINDOWS

  return result.RemoveDirectoryName();
}

// Functions for processing the gtest_output flag.

// Returns the output format, or "" for normal printed output.
String UnitTestOptions::GetOutputFormat() {
  const char* const gtest_output_flag = GTEST_FLAG(output).c_str();
  if (gtest_output_flag == NULL) return String("");

  const char* const colon = strchr(gtest_output_flag, ':');
  return (colon == NULL) ?
      String(gtest_output_flag) :
      String(gtest_output_flag, colon - gtest_output_flag);
}

// Returns the name of the requested output file, or the default if none
// was explicitly specified.
String UnitTestOptions::GetAbsolutePathToOutputFile() {
  const char* const gtest_output_flag = GTEST_FLAG(output).c_str();
  if (gtest_output_flag == NULL)
    return String("");

  const char* const colon = strchr(gtest_output_flag, ':');
  if (colon == NULL)
    return String(internal::FilePath::ConcatPaths(
               internal::FilePath(
                   UnitTest::GetInstance()->original_working_dir()),
               internal::FilePath(kDefaultOutputFile)).ToString() );

  internal::FilePath output_name(colon + 1);
  if (!output_name.IsAbsolutePath())
    // TODO(wan@google.com): on Windows \some\path is not an absolute
    // path (as its meaning depends on the current drive), yet the
    // following logic for turning it into an absolute path is wrong.
    // Fix it.
    output_name = internal::FilePath::ConcatPaths(
        internal::FilePath(UnitTest::GetInstance()->original_working_dir()),
        internal::FilePath(colon + 1));

  if (!output_name.IsDirectory())
    return output_name.ToString();

  internal::FilePath result(internal::FilePath::GenerateUniqueFileName(
      output_name, internal::GetCurrentExecutableName(),
      GetOutputFormat().c_str()));
  return result.ToString();
}

// Returns true iff the wildcard pattern matches the string.  The
// first ':' or '\0' character in pattern marks the end of it.
//
// This recursive algorithm isn't very efficient, but is clear and
// works well enough for matching test names, which are short.
bool UnitTestOptions::PatternMatchesString(const char *pattern,
                                           const char *str) {
  switch (*pattern) {
    case '\0':
    case ':':  // Either ':' or '\0' marks the end of the pattern.
      return *str == '\0';
    case '?':  // Matches any single character.
      return *str != '\0' && PatternMatchesString(pattern + 1, str + 1);
    case '*':  // Matches any string (possibly empty) of characters.
      return (*str != '\0' && PatternMatchesString(pattern, str + 1)) ||
          PatternMatchesString(pattern + 1, str);
    default:  // Non-special character.  Matches itself.
      return *pattern == *str &&
          PatternMatchesString(pattern + 1, str + 1);
  }
}

bool UnitTestOptions::MatchesFilter(const String& name, const char* filter) {
  const char *cur_pattern = filter;
  for (;;) {
    if (PatternMatchesString(cur_pattern, name.c_str())) {
      return true;
    }

    // Finds the next pattern in the filter.
    cur_pattern = strchr(cur_pattern, ':');

    // Returns if no more pattern can be found.
    if (cur_pattern == NULL) {
      return false;
    }

    // Skips the pattern separater (the ':' character).
    cur_pattern++;
  }
}

// TODO(keithray): move String function implementations to gtest-string.cc.

// Returns true iff the user-specified filter matches the test case
// name and the test name.
bool UnitTestOptions::FilterMatchesTest(const String &test_case_name,
                                        const String &test_name) {
  const String& full_name = String::Format("%s.%s",
                                           test_case_name.c_str(),
                                           test_name.c_str());

  // Split --gtest_filter at '-', if there is one, to separate into
  // positive filter and negative filter portions
  const char* const p = GTEST_FLAG(filter).c_str();
  const char* const dash = strchr(p, '-');
  String positive;
  String negative;
  if (dash == NULL) {
    positive = GTEST_FLAG(filter).c_str();  // Whole string is a positive filter
    negative = String("");
  } else {
    positive = String(p, dash - p);  // Everything up to the dash
    negative = String(dash+1);       // Everything after the dash
    if (positive.empty()) {
      // Treat '-test1' as the same as '*-test1'
      positive = kUniversalFilter;
    }
  }

  // A filter is a colon-separated list of patterns.  It matches a
  // test if any pattern in it matches the test.
  return (MatchesFilter(full_name, positive.c_str()) &&
          !MatchesFilter(full_name, negative.c_str()));
}

#if GTEST_OS_WINDOWS
// Returns EXCEPTION_EXECUTE_HANDLER if Google Test should handle the
// given SEH exception, or EXCEPTION_CONTINUE_SEARCH otherwise.
// This function is useful as an __except condition.
int UnitTestOptions::GTestShouldProcessSEH(DWORD exception_code) {
  // Google Test should handle an exception if:
  //   1. the user wants it to, AND
  //   2. this is not a breakpoint exception.
  return (GTEST_FLAG(catch_exceptions) &&
          exception_code != EXCEPTION_BREAKPOINT) ?
      EXCEPTION_EXECUTE_HANDLER :
      EXCEPTION_CONTINUE_SEARCH;
}
#endif  // GTEST_OS_WINDOWS

}  // namespace internal

// The c'tor sets this object as the test part result reporter used by
// Google Test.  The 'result' parameter specifies where to report the
// results. Intercepts only failures from the current thread.
ScopedFakeTestPartResultReporter::ScopedFakeTestPartResultReporter(
    TestPartResultArray* result)
    : intercept_mode_(INTERCEPT_ONLY_CURRENT_THREAD),
      result_(result) {
  Init();
}

// The c'tor sets this object as the test part result reporter used by
// Google Test.  The 'result' parameter specifies where to report the
// results.
ScopedFakeTestPartResultReporter::ScopedFakeTestPartResultReporter(
    InterceptMode intercept_mode, TestPartResultArray* result)
    : intercept_mode_(intercept_mode),
      result_(result) {
  Init();
}

void ScopedFakeTestPartResultReporter::Init() {
  internal::UnitTestImpl* const impl = internal::GetUnitTestImpl();
  if (intercept_mode_ == INTERCEPT_ALL_THREADS) {
    old_reporter_ = impl->GetGlobalTestPartResultReporter();
    impl->SetGlobalTestPartResultReporter(this);
  } else {
    old_reporter_ = impl->GetTestPartResultReporterForCurrentThread();
    impl->SetTestPartResultReporterForCurrentThread(this);
  }
}

// The d'tor restores the test part result reporter used by Google Test
// before.
ScopedFakeTestPartResultReporter::~ScopedFakeTestPartResultReporter() {
  internal::UnitTestImpl* const impl = internal::GetUnitTestImpl();
  if (intercept_mode_ == INTERCEPT_ALL_THREADS) {
    impl->SetGlobalTestPartResultReporter(old_reporter_);
  } else {
    impl->SetTestPartResultReporterForCurrentThread(old_reporter_);
  }
}

// Increments the test part result count and remembers the result.
// This method is from the TestPartResultReporterInterface interface.
void ScopedFakeTestPartResultReporter::ReportTestPartResult(
    const TestPartResult& result) {
  result_->Append(result);
}

namespace internal {

// Returns the type ID of ::testing::Test.  We should always call this
// instead of GetTypeId< ::testing::Test>() to get the type ID of
// testing::Test.  This is to work around a suspected linker bug when
// using Google Test as a framework on Mac OS X.  The bug causes
// GetTypeId< ::testing::Test>() to return different values depending
// on whether the call is from the Google Test framework itself or
// from user test code.  GetTestTypeId() is guaranteed to always
// return the same value, as it always calls GetTypeId<>() from the
// gtest.cc, which is within the Google Test framework.
TypeId GetTestTypeId() {
  return GetTypeId<Test>();
}

// The value of GetTestTypeId() as seen from within the Google Test
// library.  This is solely for testing GetTestTypeId().
const TypeId kTestTypeIdInGoogleTest = GetTestTypeId();

// This predicate-formatter checks that 'results' contains a test part
// failure of the given type and that the failure message contains the
// given substring.
AssertionResult HasOneFailure(const char* /* results_expr */,
                              const char* /* type_expr */,
                              const char* /* substr_expr */,
                              const TestPartResultArray& results,
                              TestPartResult::Type type,
                              const char* substr) {
  const String expected(type == TestPartResult::kFatalFailure ?
                        "1 fatal failure" :
                        "1 non-fatal failure");
  Message msg;
  if (results.size() != 1) {
    msg << "Expected: " << expected << "\n"
        << "  Actual: " << results.size() << " failures";
    for (int i = 0; i < results.size(); i++) {
      msg << "\n" << results.GetTestPartResult(i);
    }
    return AssertionFailure(msg);
  }

  const TestPartResult& r = results.GetTestPartResult(0);
  if (r.type() != type) {
    msg << "Expected: " << expected << "\n"
        << "  Actual:\n"
        << r;
    return AssertionFailure(msg);
  }

  if (strstr(r.message(), substr) == NULL) {
    msg << "Expected: " << expected << " containing \""
        << substr << "\"\n"
        << "  Actual:\n"
        << r;
    return AssertionFailure(msg);
  }

  return AssertionSuccess();
}

// The constructor of SingleFailureChecker remembers where to look up
// test part results, what type of failure we expect, and what
// substring the failure message should contain.
SingleFailureChecker:: SingleFailureChecker(
    const TestPartResultArray* results,
    TestPartResult::Type type,
    const char* substr)
    : results_(results),
      type_(type),
      substr_(substr) {}

// The destructor of SingleFailureChecker verifies that the given
// TestPartResultArray contains exactly one failure that has the given
// type and contains the given substring.  If that's not the case, a
// non-fatal failure will be generated.
SingleFailureChecker::~SingleFailureChecker() {
  EXPECT_PRED_FORMAT3(HasOneFailure, *results_, type_, substr_.c_str());
}

DefaultGlobalTestPartResultReporter::DefaultGlobalTestPartResultReporter(
    UnitTestImpl* unit_test) : unit_test_(unit_test) {}

void DefaultGlobalTestPartResultReporter::ReportTestPartResult(
    const TestPartResult& result) {
  unit_test_->current_test_result()->AddTestPartResult(result);
  unit_test_->listeners()->repeater()->OnTestPartResult(result);
}

DefaultPerThreadTestPartResultReporter::DefaultPerThreadTestPartResultReporter(
    UnitTestImpl* unit_test) : unit_test_(unit_test) {}

void DefaultPerThreadTestPartResultReporter::ReportTestPartResult(
    const TestPartResult& result) {
  unit_test_->GetGlobalTestPartResultReporter()->ReportTestPartResult(result);
}

// Returns the global test part result reporter.
TestPartResultReporterInterface*
UnitTestImpl::GetGlobalTestPartResultReporter() {
  internal::MutexLock lock(&global_test_part_result_reporter_mutex_);
  return global_test_part_result_repoter_;
}

// Sets the global test part result reporter.
void UnitTestImpl::SetGlobalTestPartResultReporter(
    TestPartResultReporterInterface* reporter) {
  internal::MutexLock lock(&global_test_part_result_reporter_mutex_);
  global_test_part_result_repoter_ = reporter;
}

// Returns the test part result reporter for the current thread.
TestPartResultReporterInterface*
UnitTestImpl::GetTestPartResultReporterForCurrentThread() {
  return per_thread_test_part_result_reporter_.get();
}

// Sets the test part result reporter for the current thread.
void UnitTestImpl::SetTestPartResultReporterForCurrentThread(
    TestPartResultReporterInterface* reporter) {
  per_thread_test_part_result_reporter_.set(reporter);
}

// Gets the number of successful test cases.
int UnitTestImpl::successful_test_case_count() const {
  return CountIf(test_cases_, TestCasePassed);
}

// Gets the number of failed test cases.
int UnitTestImpl::failed_test_case_count() const {
  return CountIf(test_cases_, TestCaseFailed);
}

// Gets the number of all test cases.
int UnitTestImpl::total_test_case_count() const {
  return static_cast<int>(test_cases_.size());
}

// Gets the number of all test cases that contain at least one test
// that should run.
int UnitTestImpl::test_case_to_run_count() const {
  return CountIf(test_cases_, ShouldRunTestCase);
}

// Gets the number of successful tests.
int UnitTestImpl::successful_test_count() const {
  return SumOverTestCaseList(test_cases_, &TestCase::successful_test_count);
}

// Gets the number of failed tests.
int UnitTestImpl::failed_test_count() const {
  return SumOverTestCaseList(test_cases_, &TestCase::failed_test_count);
}

// Gets the number of disabled tests.
int UnitTestImpl::disabled_test_count() const {
  return SumOverTestCaseList(test_cases_, &TestCase::disabled_test_count);
}

// Gets the number of all tests.
int UnitTestImpl::total_test_count() const {
  return SumOverTestCaseList(test_cases_, &TestCase::total_test_count);
}

// Gets the number of tests that should run.
int UnitTestImpl::test_to_run_count() const {
  return SumOverTestCaseList(test_cases_, &TestCase::test_to_run_count);
}

// Returns the current OS stack trace as a String.
//
// The maximum number of stack frames to be included is specified by
// the gtest_stack_trace_depth flag.  The skip_count parameter
// specifies the number of top frames to be skipped, which doesn't
// count against the number of frames to be included.
//
// For example, if Foo() calls Bar(), which in turn calls
// CurrentOsStackTraceExceptTop(1), Foo() will be included in the
// trace but Bar() and CurrentOsStackTraceExceptTop() won't.
String UnitTestImpl::CurrentOsStackTraceExceptTop(int skip_count) {
  (void)skip_count;
  return String("");
}

// Returns the current time in milliseconds.
TimeInMillis GetTimeInMillis() {
#if GTEST_OS_WINDOWS_MOBILE || defined(__BORLANDC__)
  // Difference between 1970-01-01 and 1601-01-01 in milliseconds.
  // http://analogous.blogspot.com/2005/04/epoch.html
  const TimeInMillis kJavaEpochToWinFileTimeDelta =
    static_cast<TimeInMillis>(116444736UL) * 100000UL;
  const DWORD kTenthMicrosInMilliSecond = 10000;

  SYSTEMTIME now_systime;
  FILETIME now_filetime;
  ULARGE_INTEGER now_int64;
  // TODO(kenton@google.com): Shouldn't this just use
  //   GetSystemTimeAsFileTime()?
  GetSystemTime(&now_systime);
  if (SystemTimeToFileTime(&now_systime, &now_filetime)) {
    now_int64.LowPart = now_filetime.dwLowDateTime;
    now_int64.HighPart = now_filetime.dwHighDateTime;
    now_int64.QuadPart = (now_int64.QuadPart / kTenthMicrosInMilliSecond) -
      kJavaEpochToWinFileTimeDelta;
    return now_int64.QuadPart;
  }
  return 0;
#elif GTEST_OS_WINDOWS && !GTEST_HAS_GETTIMEOFDAY_
  __timeb64 now;
#ifdef _MSC_VER
  // MSVC 8 deprecates _ftime64(), so we want to suppress warning 4996
  // (deprecated function) there.
  // TODO(kenton@google.com): Use GetTickCount()?  Or use
  //   SystemTimeToFileTime()
#pragma warning(push)          // Saves the current warning state.
#pragma warning(disable:4996)  // Temporarily disables warning 4996.
  _ftime64(&now);
#pragma warning(pop)           // Restores the warning state.
#else
  _ftime64(&now);
#endif  // _MSC_VER
  return static_cast<TimeInMillis>(now.time) * 1000 + now.millitm;
#elif GTEST_HAS_GETTIMEOFDAY_
  struct timeval now;
  gettimeofday(&now, NULL);
  return static_cast<TimeInMillis>(now.tv_sec) * 1000 + now.tv_usec / 1000;
#else
#error "Don't know how to get the current time on your system."
#endif
}

// Utilities

// class String

// Returns the input enclosed in double quotes if it's not NULL;
// otherwise returns "(null)".  For example, "\"Hello\"" is returned
// for input "Hello".
//
// This is useful for printing a C string in the syntax of a literal.
//
// Known issue: escape sequences are not handled yet.
String String::ShowCStringQuoted(const char* c_str) {
  return c_str ? String::Format("\"%s\"", c_str) : String("(null)");
}

// Copies at most length characters from str into a newly-allocated
// piece of memory of size length+1.  The memory is allocated with new[].
// A terminating null byte is written to the memory, and a pointer to it
// is returned.  If str is NULL, NULL is returned.
static char* CloneString(const char* str, size_t length) {
  if (str == NULL) {
    return NULL;
  } else {
    char* const clone = new char[length + 1];
    posix::StrNCpy(clone, str, length);
    clone[length] = '\0';
    return clone;
  }
}

// Clones a 0-terminated C string, allocating memory using new.  The
// caller is responsible for deleting[] the return value.  Returns the
// cloned string, or NULL if the input is NULL.
const char * String::CloneCString(const char* c_str) {
  return (c_str == NULL) ?
                    NULL : CloneString(c_str, strlen(c_str));
}

#if GTEST_OS_WINDOWS_MOBILE
// Creates a UTF-16 wide string from the given ANSI string, allocating
// memory using new. The caller is responsible for deleting the return
// value using delete[]. Returns the wide string, or NULL if the
// input is NULL.
LPCWSTR String::AnsiToUtf16(const char* ansi) {
  if (!ansi) return NULL;
  const int length = strlen(ansi);
  const int unicode_length =
      MultiByteToWideChar(CP_ACP, 0, ansi, length,
                          NULL, 0);
  WCHAR* unicode = new WCHAR[unicode_length + 1];
  MultiByteToWideChar(CP_ACP, 0, ansi, length,
                      unicode, unicode_length);
  unicode[unicode_length] = 0;
  return unicode;
}

// Creates an ANSI string from the given wide string, allocating
// memory using new. The caller is responsible for deleting the return
// value using delete[]. Returns the ANSI string, or NULL if the
// input is NULL.
const char* String::Utf16ToAnsi(LPCWSTR utf16_str)  {
  if (!utf16_str) return NULL;
  const int ansi_length =
      WideCharToMultiByte(CP_ACP, 0, utf16_str, -1,
                          NULL, 0, NULL, NULL);
  char* ansi = new char[ansi_length + 1];
  WideCharToMultiByte(CP_ACP, 0, utf16_str, -1,
                      ansi, ansi_length, NULL, NULL);
  ansi[ansi_length] = 0;
  return ansi;
}

#endif  // GTEST_OS_WINDOWS_MOBILE

// Compares two C strings.  Returns true iff they have the same content.
//
// Unlike strcmp(), this function can handle NULL argument(s).  A NULL
// C string is considered different to any non-NULL C string,
// including the empty string.
bool String::CStringEquals(const char * lhs, const char * rhs) {
  if ( lhs == NULL ) return rhs == NULL;

  if ( rhs == NULL ) return false;

  return strcmp(lhs, rhs) == 0;
}

#if GTEST_HAS_STD_WSTRING || GTEST_HAS_GLOBAL_WSTRING

// Converts an array of wide chars to a narrow string using the UTF-8
// encoding, and streams the result to the given Message object.
static void StreamWideCharsToMessage(const wchar_t* wstr, size_t length,
                                     Message* msg) {
  // TODO(wan): consider allowing a testing::String object to
  // contain '\0'.  This will make it behave more like std::string,
  // and will allow ToUtf8String() to return the correct encoding
  // for '\0' s.t. we can get rid of the conditional here (and in
  // several other places).
  for (size_t i = 0; i != length; ) {  // NOLINT
    if (wstr[i] != L'\0') {
      *msg << WideStringToUtf8(wstr + i, static_cast<int>(length - i));
      while (i != length && wstr[i] != L'\0')
        i++;
    } else {
      *msg << '\0';
      i++;
    }
  }
}

#endif  // GTEST_HAS_STD_WSTRING || GTEST_HAS_GLOBAL_WSTRING

}  // namespace internal

#if GTEST_HAS_STD_WSTRING
// Converts the given wide string to a narrow string using the UTF-8
// encoding, and streams the result to this Message object.
Message& Message::operator <<(const ::std::wstring& wstr) {
  internal::StreamWideCharsToMessage(wstr.c_str(), wstr.length(), this);
  return *this;
}
#endif  // GTEST_HAS_STD_WSTRING

#if GTEST_HAS_GLOBAL_WSTRING
// Converts the given wide string to a narrow string using the UTF-8
// encoding, and streams the result to this Message object.
Message& Message::operator <<(const ::wstring& wstr) {
  internal::StreamWideCharsToMessage(wstr.c_str(), wstr.length(), this);
  return *this;
}
#endif  // GTEST_HAS_GLOBAL_WSTRING

namespace internal {

// Formats a value to be used in a failure message.

// For a char value, we print it as a C++ char literal and as an
// unsigned integer (both in decimal and in hexadecimal).
String FormatForFailureMessage(char ch) {
  const unsigned int ch_as_uint = ch;
  // A String object cannot contain '\0', so we print "\\0" when ch is
  // '\0'.
  return String::Format("'%s' (%u, 0x%X)",
                        ch ? String::Format("%c", ch).c_str() : "\\0",
                        ch_as_uint, ch_as_uint);
}

// For a wchar_t value, we print it as a C++ wchar_t literal and as an
// unsigned integer (both in decimal and in hexidecimal).
String FormatForFailureMessage(wchar_t wchar) {
  // The C++ standard doesn't specify the exact size of the wchar_t
  // type.  It just says that it shall have the same size as another
  // integral type, called its underlying type.
  //
  // Therefore, in order to print a wchar_t value in the numeric form,
  // we first convert it to the largest integral type (UInt64) and
  // then print the converted value.
  //
  // We use streaming to print the value as "%llu" doesn't work
  // correctly with MSVC 7.1.
  const UInt64 wchar_as_uint64 = wchar;
  Message msg;
  // A String object cannot contain '\0', so we print "\\0" when wchar is
  // L'\0'.
  char buffer[32];  // CodePointToUtf8 requires a buffer that big.
  msg << "L'"
      << (wchar ? CodePointToUtf8(static_cast<UInt32>(wchar), buffer) : "\\0")
      << "' (" << wchar_as_uint64 << ", 0x" << ::std::setbase(16)
      << wchar_as_uint64 << ")";
  return msg.GetString();
}

}  // namespace internal

// AssertionResult constructors.
// Used in EXPECT_TRUE/FALSE(assertion_result).
AssertionResult::AssertionResult(const AssertionResult& other)
    : success_(other.success_),
      message_(other.message_.get() != NULL ?
               new internal::String(*other.message_) :
               static_cast<internal::String*>(NULL)) {
}

// Returns the assertion's negation. Used with EXPECT/ASSERT_FALSE.
AssertionResult AssertionResult::operator!() const {
  AssertionResult negation(!success_);
  if (message_.get() != NULL)
    negation << *message_;
  return negation;
}

// Makes a successful assertion result.
AssertionResult AssertionSuccess() {
  return AssertionResult(true);
}

// Makes a failed assertion result.
AssertionResult AssertionFailure() {
  return AssertionResult(false);
}

// Makes a failed assertion result with the given failure message.
// Deprecated; use AssertionFailure() << message.
AssertionResult AssertionFailure(const Message& message) {
  return AssertionFailure() << message;
}

namespace internal {

// Constructs and returns the message for an equality assertion
// (e.g. ASSERT_EQ, EXPECT_STREQ, etc) failure.
//
// The first four parameters are the expressions used in the assertion
// and their values, as strings.  For example, for ASSERT_EQ(foo, bar)
// where foo is 5 and bar is 6, we have:
//
//   expected_expression: "foo"
//   actual_expression:   "bar"
//   expected_value:      "5"
//   actual_value:        "6"
//
// The ignoring_case parameter is true iff the assertion is a
// *_STRCASEEQ*.  When it's true, the string " (ignoring case)" will
// be inserted into the message.
AssertionResult EqFailure(const char* expected_expression,
                          const char* actual_expression,
                          const String& expected_value,
                          const String& actual_value,
                          bool ignoring_case) {
  Message msg;
  msg << "Value of: " << actual_expression;
  if (actual_value != actual_expression) {
    msg << "\n  Actual: " << actual_value;
  }

  msg << "\nExpected: " << expected_expression;
  if (ignoring_case) {
    msg << " (ignoring case)";
  }
  if (expected_value != expected_expression) {
    msg << "\nWhich is: " << expected_value;
  }

  return AssertionFailure(msg);
}

// Constructs a failure message for Boolean assertions such as EXPECT_TRUE.
String GetBoolAssertionFailureMessage(const AssertionResult& assertion_result,
                                      const char* expression_text,
                                      const char* actual_predicate_value,
                                      const char* expected_predicate_value) {
  const char* actual_message = assertion_result.message();
  Message msg;
  msg << "Value of: " << expression_text
      << "\n  Actual: " << actual_predicate_value;
  if (actual_message[0] != '\0')
    msg << " (" << actual_message << ")";
  msg << "\nExpected: " << expected_predicate_value;
  return msg.GetString();
}

// Helper function for implementing ASSERT_NEAR.
AssertionResult DoubleNearPredFormat(const char* expr1,
                                     const char* expr2,
                                     const char* abs_error_expr,
                                     double val1,
                                     double val2,
                                     double abs_error) {
  const double diff = fabs(val1 - val2);
  if (diff <= abs_error) return AssertionSuccess();

  // TODO(wan): do not print the value of an expression if it's
  // already a literal.
  Message msg;
  msg << "The difference between " << expr1 << " and " << expr2
      << " is " << diff << ", which exceeds " << abs_error_expr << ", where\n"
      << expr1 << " evaluates to " << val1 << ",\n"
      << expr2 << " evaluates to " << val2 << ", and\n"
      << abs_error_expr << " evaluates to " << abs_error << ".";
  return AssertionFailure(msg);
}


// Helper template for implementing FloatLE() and DoubleLE().
template <typename RawType>
AssertionResult FloatingPointLE(const char* expr1,
                                const char* expr2,
                                RawType val1,
                                RawType val2) {
  // Returns success if val1 is less than val2,
  if (val1 < val2) {
    return AssertionSuccess();
  }

  // or if val1 is almost equal to val2.
  const FloatingPoint<RawType> lhs(val1), rhs(val2);
  if (lhs.AlmostEquals(rhs)) {
    return AssertionSuccess();
  }

  // Note that the above two checks will both fail if either val1 or
  // val2 is NaN, as the IEEE floating-point standard requires that
  // any predicate involving a NaN must return false.

  StrStream val1_ss;
  val1_ss << std::setprecision(std::numeric_limits<RawType>::digits10 + 2)
          << val1;

  StrStream val2_ss;
  val2_ss << std::setprecision(std::numeric_limits<RawType>::digits10 + 2)
          << val2;

  Message msg;
  msg << "Expected: (" << expr1 << ") <= (" << expr2 << ")\n"
      << "  Actual: " << StrStreamToString(&val1_ss) << " vs "
      << StrStreamToString(&val2_ss);

  return AssertionFailure(msg);
}

}  // namespace internal

// Asserts that val1 is less than, or almost equal to, val2.  Fails
// otherwise.  In particular, it fails if either val1 or val2 is NaN.
AssertionResult FloatLE(const char* expr1, const char* expr2,
                        float val1, float val2) {
  return internal::FloatingPointLE<float>(expr1, expr2, val1, val2);
}

// Asserts that val1 is less than, or almost equal to, val2.  Fails
// otherwise.  In particular, it fails if either val1 or val2 is NaN.
AssertionResult DoubleLE(const char* expr1, const char* expr2,
                         double val1, double val2) {
  return internal::FloatingPointLE<double>(expr1, expr2, val1, val2);
}

namespace internal {

// The helper function for {ASSERT|EXPECT}_EQ with int or enum
// arguments.
AssertionResult CmpHelperEQ(const char* expected_expression,
                            const char* actual_expression,
                            BiggestInt expected,
                            BiggestInt actual) {
  if (expected == actual) {
    return AssertionSuccess();
  }

  return EqFailure(expected_expression,
                   actual_expression,
                   FormatForComparisonFailureMessage(expected, actual),
                   FormatForComparisonFailureMessage(actual, expected),
                   false);
}

// A macro for implementing the helper functions needed to implement
// ASSERT_?? and EXPECT_?? with integer or enum arguments.  It is here
// just to avoid copy-and-paste of similar code.
#define GTEST_IMPL_CMP_HELPER_(op_name, op)\
AssertionResult CmpHelper##op_name(const char* expr1, const char* expr2, \
                                   BiggestInt val1, BiggestInt val2) {\
  if (val1 op val2) {\
    return AssertionSuccess();\
  } else {\
    Message msg;\
    msg << "Expected: (" << expr1 << ") " #op " (" << expr2\
        << "), actual: " << FormatForComparisonFailureMessage(val1, val2)\
        << " vs " << FormatForComparisonFailureMessage(val2, val1);\
    return AssertionFailure(msg);\
  }\
}

// Implements the helper function for {ASSERT|EXPECT}_NE with int or
// enum arguments.
GTEST_IMPL_CMP_HELPER_(NE, !=)
// Implements the helper function for {ASSERT|EXPECT}_LE with int or
// enum arguments.
GTEST_IMPL_CMP_HELPER_(LE, <=)
// Implements the helper function for {ASSERT|EXPECT}_LT with int or
// enum arguments.
GTEST_IMPL_CMP_HELPER_(LT, < )
// Implements the helper function for {ASSERT|EXPECT}_GE with int or
// enum arguments.
GTEST_IMPL_CMP_HELPER_(GE, >=)
// Implements the helper function for {ASSERT|EXPECT}_GT with int or
// enum arguments.
GTEST_IMPL_CMP_HELPER_(GT, > )

#undef GTEST_IMPL_CMP_HELPER_

// The helper function for {ASSERT|EXPECT}_STREQ.
AssertionResult CmpHelperSTREQ(const char* expected_expression,
                               const char* actual_expression,
                               const char* expected,
                               const char* actual) {
  if (String::CStringEquals(expected, actual)) {
    return AssertionSuccess();
  }

  return EqFailure(expected_expression,
                   actual_expression,
                   String::ShowCStringQuoted(expected),
                   String::ShowCStringQuoted(actual),
                   false);
}

// The helper function for {ASSERT|EXPECT}_STRCASEEQ.
AssertionResult CmpHelperSTRCASEEQ(const char* expected_expression,
                                   const char* actual_expression,
                                   const char* expected,
                                   const char* actual) {
  if (String::CaseInsensitiveCStringEquals(expected, actual)) {
    return AssertionSuccess();
  }

  return EqFailure(expected_expression,
                   actual_expression,
                   String::ShowCStringQuoted(expected),
                   String::ShowCStringQuoted(actual),
                   true);
}

// The helper function for {ASSERT|EXPECT}_STRNE.
AssertionResult CmpHelperSTRNE(const char* s1_expression,
                               const char* s2_expression,
                               const char* s1,
                               const char* s2) {
  if (!String::CStringEquals(s1, s2)) {
    return AssertionSuccess();
  } else {
    Message msg;
    msg << "Expected: (" << s1_expression << ") != ("
        << s2_expression << "), actual: \""
        << s1 << "\" vs \"" << s2 << "\"";
    return AssertionFailure(msg);
  }
}

// The helper function for {ASSERT|EXPECT}_STRCASENE.
AssertionResult CmpHelperSTRCASENE(const char* s1_expression,
                                   const char* s2_expression,
                                   const char* s1,
                                   const char* s2) {
  if (!String::CaseInsensitiveCStringEquals(s1, s2)) {
    return AssertionSuccess();
  } else {
    Message msg;
    msg << "Expected: (" << s1_expression << ") != ("
        << s2_expression << ") (ignoring case), actual: \""
        << s1 << "\" vs \"" << s2 << "\"";
    return AssertionFailure(msg);
  }
}

}  // namespace internal

namespace {

// Helper functions for implementing IsSubString() and IsNotSubstring().

// This group of overloaded functions return true iff needle is a
// substring of haystack.  NULL is considered a substring of itself
// only.

bool IsSubstringPred(const char* needle, const char* haystack) {
  if (needle == NULL || haystack == NULL)
    return needle == haystack;

  return strstr(haystack, needle) != NULL;
}

bool IsSubstringPred(const wchar_t* needle, const wchar_t* haystack) {
  if (needle == NULL || haystack == NULL)
    return needle == haystack;

  return wcsstr(haystack, needle) != NULL;
}

// StringType here can be either ::std::string or ::std::wstring.
template <typename StringType>
bool IsSubstringPred(const StringType& needle,
                     const StringType& haystack) {
  return haystack.find(needle) != StringType::npos;
}

// This function implements either IsSubstring() or IsNotSubstring(),
// depending on the value of the expected_to_be_substring parameter.
// StringType here can be const char*, const wchar_t*, ::std::string,
// or ::std::wstring.
template <typename StringType>
AssertionResult IsSubstringImpl(
    bool expected_to_be_substring,
    const char* needle_expr, const char* haystack_expr,
    const StringType& needle, const StringType& haystack) {
  if (IsSubstringPred(needle, haystack) == expected_to_be_substring)
    return AssertionSuccess();

  const bool is_wide_string = sizeof(needle[0]) > 1;
  const char* const begin_string_quote = is_wide_string ? "L\"" : "\"";
  return AssertionFailure(
      Message()
      << "Value of: " << needle_expr << "\n"
      << "  Actual: " << begin_string_quote << needle << "\"\n"
      << "Expected: " << (expected_to_be_substring ? "" : "not ")
      << "a substring of " << haystack_expr << "\n"
      << "Which is: " << begin_string_quote << haystack << "\"");
}

}  // namespace

// IsSubstring() and IsNotSubstring() check whether needle is a
// substring of haystack (NULL is considered a substring of itself
// only), and return an appropriate error message when they fail.

AssertionResult IsSubstring(
    const char* needle_expr, const char* haystack_expr,
    const char* needle, const char* haystack) {
  return IsSubstringImpl(true, needle_expr, haystack_expr, needle, haystack);
}

AssertionResult IsSubstring(
    const char* needle_expr, const char* haystack_expr,
    const wchar_t* needle, const wchar_t* haystack) {
  return IsSubstringImpl(true, needle_expr, haystack_expr, needle, haystack);
}

AssertionResult IsNotSubstring(
    const char* needle_expr, const char* haystack_expr,
    const char* needle, const char* haystack) {
  return IsSubstringImpl(false, needle_expr, haystack_expr, needle, haystack);
}

AssertionResult IsNotSubstring(
    const char* needle_expr, const char* haystack_expr,
    const wchar_t* needle, const wchar_t* haystack) {
  return IsSubstringImpl(false, needle_expr, haystack_expr, needle, haystack);
}

AssertionResult IsSubstring(
    const char* needle_expr, const char* haystack_expr,
    const ::std::string& needle, const ::std::string& haystack) {
  return IsSubstringImpl(true, needle_expr, haystack_expr, needle, haystack);
}

AssertionResult IsNotSubstring(
    const char* needle_expr, const char* haystack_expr,
    const ::std::string& needle, const ::std::string& haystack) {
  return IsSubstringImpl(false, needle_expr, haystack_expr, needle, haystack);
}

#if GTEST_HAS_STD_WSTRING
AssertionResult IsSubstring(
    const char* needle_expr, const char* haystack_expr,
    const ::std::wstring& needle, const ::std::wstring& haystack) {
  return IsSubstringImpl(true, needle_expr, haystack_expr, needle, haystack);
}

AssertionResult IsNotSubstring(
    const char* needle_expr, const char* haystack_expr,
    const ::std::wstring& needle, const ::std::wstring& haystack) {
  return IsSubstringImpl(false, needle_expr, haystack_expr, needle, haystack);
}
#endif  // GTEST_HAS_STD_WSTRING

namespace internal {

#if GTEST_OS_WINDOWS

namespace {

// Helper function for IsHRESULT{SuccessFailure} predicates
AssertionResult HRESULTFailureHelper(const char* expr,
                                     const char* expected,
                                     long hr) {  // NOLINT
#if GTEST_OS_WINDOWS_MOBILE
  // Windows CE doesn't support FormatMessage.
  const char error_text[] = "";
#else
  // Looks up the human-readable system message for the HRESULT code
  // and since we're not passing any params to FormatMessage, we don't
  // want inserts expanded.
  const DWORD kFlags = FORMAT_MESSAGE_FROM_SYSTEM |
                       FORMAT_MESSAGE_IGNORE_INSERTS;
  const DWORD kBufSize = 4096;  // String::Format can't exceed this length.
  // Gets the system's human readable message string for this HRESULT.
  char error_text[kBufSize] = { '\0' };
  DWORD message_length = ::FormatMessageA(kFlags,
                                          0,  // no source, we're asking system
                                          hr,  // the error
                                          0,  // no line width restrictions
                                          error_text,  // output buffer
                                          kBufSize,  // buf size
                                          NULL);  // no arguments for inserts
  // Trims tailing white space (FormatMessage leaves a trailing cr-lf)
  for (; message_length && isspace(error_text[message_length - 1]);
          --message_length) {
    error_text[message_length - 1] = '\0';
  }
#endif  // GTEST_OS_WINDOWS_MOBILE

  const String error_hex(String::Format("0x%08X ", hr));
  Message msg;
  msg << "Expected: " << expr << " " << expected << ".\n"
      << "  Actual: " << error_hex << error_text << "\n";

  return ::testing::AssertionFailure(msg);
}

}  // namespace

AssertionResult IsHRESULTSuccess(const char* expr, long hr) {  // NOLINT
  if (SUCCEEDED(hr)) {
    return AssertionSuccess();
  }
  return HRESULTFailureHelper(expr, "succeeds", hr);
}

AssertionResult IsHRESULTFailure(const char* expr, long hr) {  // NOLINT
  if (FAILED(hr)) {
    return AssertionSuccess();
  }
  return HRESULTFailureHelper(expr, "fails", hr);
}

#endif  // GTEST_OS_WINDOWS

// Utility functions for encoding Unicode text (wide strings) in
// UTF-8.

// A Unicode code-point can have up to 21 bits, and is encoded in UTF-8
// like this:
//
// Code-point length   Encoding
//   0 -  7 bits       0xxxxxxx
//   8 - 11 bits       110xxxxx 10xxxxxx
//  12 - 16 bits       1110xxxx 10xxxxxx 10xxxxxx
//  17 - 21 bits       11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

// The maximum code-point a one-byte UTF-8 sequence can represent.
const UInt32 kMaxCodePoint1 = (static_cast<UInt32>(1) <<  7) - 1;

// The maximum code-point a two-byte UTF-8 sequence can represent.
const UInt32 kMaxCodePoint2 = (static_cast<UInt32>(1) << (5 + 6)) - 1;

// The maximum code-point a three-byte UTF-8 sequence can represent.
const UInt32 kMaxCodePoint3 = (static_cast<UInt32>(1) << (4 + 2*6)) - 1;

// The maximum code-point a four-byte UTF-8 sequence can represent.
const UInt32 kMaxCodePoint4 = (static_cast<UInt32>(1) << (3 + 3*6)) - 1;

// Chops off the n lowest bits from a bit pattern.  Returns the n
// lowest bits.  As a side effect, the original bit pattern will be
// shifted to the right by n bits.
inline UInt32 ChopLowBits(UInt32* bits, int n) {
  const UInt32 low_bits = *bits & ((static_cast<UInt32>(1) << n) - 1);
  *bits >>= n;
  return low_bits;
}

// Converts a Unicode code point to a narrow string in UTF-8 encoding.
// code_point parameter is of type UInt32 because wchar_t may not be
// wide enough to contain a code point.
// The output buffer str must containt at least 32 characters.
// The function returns the address of the output buffer.
// If the code_point is not a valid Unicode code point
// (i.e. outside of Unicode range U+0 to U+10FFFF) it will be output
// as '(Invalid Unicode 0xXXXXXXXX)'.
char* CodePointToUtf8(UInt32 code_point, char* str) {
  if (code_point <= kMaxCodePoint1) {
    str[1] = '\0';
    str[0] = static_cast<char>(code_point);                          // 0xxxxxxx
  } else if (code_point <= kMaxCodePoint2) {
    str[2] = '\0';
    str[1] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6));  // 10xxxxxx
    str[0] = static_cast<char>(0xC0 | code_point);                   // 110xxxxx
  } else if (code_point <= kMaxCodePoint3) {
    str[3] = '\0';
    str[2] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6));  // 10xxxxxx
    str[1] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6));  // 10xxxxxx
    str[0] = static_cast<char>(0xE0 | code_point);                   // 1110xxxx
  } else if (code_point <= kMaxCodePoint4) {
    str[4] = '\0';
    str[3] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6));  // 10xxxxxx
    str[2] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6));  // 10xxxxxx
    str[1] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6));  // 10xxxxxx
    str[0] = static_cast<char>(0xF0 | code_point);                   // 11110xxx
  } else {
    // The longest string String::Format can produce when invoked
    // with these parameters is 28 character long (not including
    // the terminating nul character). We are asking for 32 character
    // buffer just in case. This is also enough for strncpy to
    // null-terminate the destination string.
    posix::StrNCpy(
        str, String::Format("(Invalid Unicode 0x%X)", code_point).c_str(), 32);
    str[31] = '\0';  // Makes sure no change in the format to strncpy leaves
                     // the result unterminated.
  }
  return str;
}

// The following two functions only make sense if the the system
// uses UTF-16 for wide string encoding. All supported systems
// with 16 bit wchar_t (Windows, Cygwin, Symbian OS) do use UTF-16.

// Determines if the arguments constitute UTF-16 surrogate pair
// and thus should be combined into a single Unicode code point
// using CreateCodePointFromUtf16SurrogatePair.
inline bool IsUtf16SurrogatePair(wchar_t first, wchar_t second) {
  return sizeof(wchar_t) == 2 &&
      (first & 0xFC00) == 0xD800 && (second & 0xFC00) == 0xDC00;
}

// Creates a Unicode code point from UTF16 surrogate pair.
inline UInt32 CreateCodePointFromUtf16SurrogatePair(wchar_t first,
                                                    wchar_t second) {
  const UInt32 mask = (1 << 10) - 1;
  return (sizeof(wchar_t) == 2) ?
      (((first & mask) << 10) | (second & mask)) + 0x10000 :
      // This function should not be called when the condition is
      // false, but we provide a sensible default in case it is.
      static_cast<UInt32>(first);
}

// Converts a wide string to a narrow string in UTF-8 encoding.
// The wide string is assumed to have the following encoding:
//   UTF-16 if sizeof(wchar_t) == 2 (on Windows, Cygwin, Symbian OS)
//   UTF-32 if sizeof(wchar_t) == 4 (on Linux)
// Parameter str points to a null-terminated wide string.
// Parameter num_chars may additionally limit the number
// of wchar_t characters processed. -1 is used when the entire string
// should be processed.
// If the string contains code points that are not valid Unicode code points
// (i.e. outside of Unicode range U+0 to U+10FFFF) they will be output
// as '(Invalid Unicode 0xXXXXXXXX)'. If the string is in UTF16 encoding
// and contains invalid UTF-16 surrogate pairs, values in those pairs
// will be encoded as individual Unicode characters from Basic Normal Plane.
String WideStringToUtf8(const wchar_t* str, int num_chars) {
  if (num_chars == -1)
    num_chars = static_cast<int>(wcslen(str));

  StrStream stream;
  for (int i = 0; i < num_chars; ++i) {
    UInt32 unicode_code_point;

    if (str[i] == L'\0') {
      break;
    } else if (i + 1 < num_chars && IsUtf16SurrogatePair(str[i], str[i + 1])) {
      unicode_code_point = CreateCodePointFromUtf16SurrogatePair(str[i],
                                                                 str[i + 1]);
      i++;
    } else {
      unicode_code_point = static_cast<UInt32>(str[i]);
    }

    char buffer[32];  // CodePointToUtf8 requires a buffer this big.
    stream << CodePointToUtf8(unicode_code_point, buffer);
  }
  return StrStreamToString(&stream);
}

// Converts a wide C string to a String using the UTF-8 encoding.
// NULL will be converted to "(null)".
String String::ShowWideCString(const wchar_t * wide_c_str) {
  if (wide_c_str == NULL) return String("(null)");

  return String(internal::WideStringToUtf8(wide_c_str, -1).c_str());
}

// Similar to ShowWideCString(), except that this function encloses
// the converted string in double quotes.
String String::ShowWideCStringQuoted(const wchar_t* wide_c_str) {
  if (wide_c_str == NULL) return String("(null)");

  return String::Format("L\"%s\"",
                        String::ShowWideCString(wide_c_str).c_str());
}

// Compares two wide C strings.  Returns true iff they have the same
// content.
//
// Unlike wcscmp(), this function can handle NULL argument(s).  A NULL
// C string is considered different to any non-NULL C string,
// including the empty string.
bool String::WideCStringEquals(const wchar_t * lhs, const wchar_t * rhs) {
  if (lhs == NULL) return rhs == NULL;

  if (rhs == NULL) return false;

  return wcscmp(lhs, rhs) == 0;
}

// Helper function for *_STREQ on wide strings.
AssertionResult CmpHelperSTREQ(const char* expected_expression,
                               const char* actual_expression,
                               const wchar_t* expected,
                               const wchar_t* actual) {
  if (String::WideCStringEquals(expected, actual)) {
    return AssertionSuccess();
  }

  return EqFailure(expected_expression,
                   actual_expression,
                   String::ShowWideCStringQuoted(expected),
                   String::ShowWideCStringQuoted(actual),
                   false);
}

// Helper function for *_STRNE on wide strings.
AssertionResult CmpHelperSTRNE(const char* s1_expression,
                               const char* s2_expression,
                               const wchar_t* s1,
                               const wchar_t* s2) {
  if (!String::WideCStringEquals(s1, s2)) {
    return AssertionSuccess();
  }

  Message msg;
  msg << "Expected: (" << s1_expression << ") != ("
      << s2_expression << "), actual: "
      << String::ShowWideCStringQuoted(s1)
      << " vs " << String::ShowWideCStringQuoted(s2);
  return AssertionFailure(msg);
}

// Compares two C strings, ignoring case.  Returns true iff they have
// the same content.
//
// Unlike strcasecmp(), this function can handle NULL argument(s).  A
// NULL C string is considered different to any non-NULL C string,
// including the empty string.
bool String::CaseInsensitiveCStringEquals(const char * lhs, const char * rhs) {
  if (lhs == NULL)
    return rhs == NULL;
  if (rhs == NULL)
    return false;
  return posix::StrCaseCmp(lhs, rhs) == 0;
}

  // Compares two wide C strings, ignoring case.  Returns true iff they
  // have the same content.
  //
  // Unlike wcscasecmp(), this function can handle NULL argument(s).
  // A NULL C string is considered different to any non-NULL wide C string,
  // including the empty string.
  // NB: The implementations on different platforms slightly differ.
  // On windows, this method uses _wcsicmp which compares according to LC_CTYPE
  // environment variable. On GNU platform this method uses wcscasecmp
  // which compares according to LC_CTYPE category of the current locale.
  // On MacOS X, it uses towlower, which also uses LC_CTYPE category of the
  // current locale.
bool String::CaseInsensitiveWideCStringEquals(const wchar_t* lhs,
                                              const wchar_t* rhs) {
  if ( lhs == NULL ) return rhs == NULL;

  if ( rhs == NULL ) return false;

#if GTEST_OS_WINDOWS
  return _wcsicmp(lhs, rhs) == 0;
#elif GTEST_OS_LINUX
  return wcscasecmp(lhs, rhs) == 0;
#else
  // Mac OS X and Cygwin don't define wcscasecmp.  Other unknown OSes
  // may not define it either.
  wint_t left, right;
  do {
    left = towlower(*lhs++);
    right = towlower(*rhs++);
  } while (left && left == right);
  return left == right;
#endif  // OS selector
}

// Compares this with another String.
// Returns < 0 if this is less than rhs, 0 if this is equal to rhs, or > 0
// if this is greater than rhs.
int String::Compare(const String & rhs) const {
  const char* const lhs_c_str = c_str();
  const char* const rhs_c_str = rhs.c_str();

  if (lhs_c_str == NULL) {
    return rhs_c_str == NULL ? 0 : -1;  // NULL < anything except NULL
  } else if (rhs_c_str == NULL) {
    return 1;
  }

  const size_t shorter_str_len =
      length() <= rhs.length() ? length() : rhs.length();
  for (size_t i = 0; i != shorter_str_len; i++) {
    if (lhs_c_str[i] < rhs_c_str[i]) {
      return -1;
    } else if (lhs_c_str[i] > rhs_c_str[i]) {
      return 1;
    }
  }
  return (length() < rhs.length()) ? -1 :
      (length() > rhs.length()) ? 1 : 0;
}

// Returns true iff this String ends with the given suffix.  *Any*
// String is considered to end with a NULL or empty suffix.
bool String::EndsWith(const char* suffix) const {
  if (suffix == NULL || CStringEquals(suffix, "")) return true;

  if (c_str() == NULL) return false;

  const size_t this_len = strlen(c_str());
  const size_t suffix_len = strlen(suffix);
  return (this_len >= suffix_len) &&
         CStringEquals(c_str() + this_len - suffix_len, suffix);
}

// Returns true iff this String ends with the given suffix, ignoring case.
// Any String is considered to end with a NULL or empty suffix.
bool String::EndsWithCaseInsensitive(const char* suffix) const {
  if (suffix == NULL || CStringEquals(suffix, "")) return true;

  if (c_str() == NULL) return false;

  const size_t this_len = strlen(c_str());
  const size_t suffix_len = strlen(suffix);
  return (this_len >= suffix_len) &&
         CaseInsensitiveCStringEquals(c_str() + this_len - suffix_len, suffix);
}

// Formats a list of arguments to a String, using the same format
// spec string as for printf.
//
// We do not use the StringPrintf class as it is not universally
// available.
//
// The result is limited to 4096 characters (including the tailing 0).
// If 4096 characters are not enough to format the input, or if
// there's an error, "<formatting error or buffer exceeded>" is
// returned.
String String::Format(const char * format, ...) {
  va_list args;
  va_start(args, format);

  char buffer[4096];
  const int kBufferSize = sizeof(buffer)/sizeof(buffer[0]);

  // MSVC 8 deprecates vsnprintf(), so we want to suppress warning
  // 4996 (deprecated function) there.
#ifdef _MSC_VER  // We are using MSVC.
#pragma warning(push)          // Saves the current warning state.
#pragma warning(disable:4996)  // Temporarily disables warning 4996.
  const int size = vsnprintf(buffer, kBufferSize, format, args);
#pragma warning(pop)           // Restores the warning state.
#else  // We are not using MSVC.
  const int size = vsnprintf(buffer, kBufferSize, format, args);
#endif  // _MSC_VER
  va_end(args);

  // vsnprintf()'s behavior is not portable.  When the buffer is not
  // big enough, it returns a negative value in MSVC, and returns the
  // needed buffer size on Linux.  When there is an output error, it
  // always returns a negative value.  For simplicity, we lump the two
  // error cases together.
  if (size < 0 || size >= kBufferSize) {
    return String("<formatting error or buffer exceeded>");
  } else {
    return String(buffer, size);
  }
}

// Converts the buffer in a StrStream to a String, converting NUL
// bytes to "\\0" along the way.
String StrStreamToString(StrStream* ss) {
  const ::std::string& str = ss->str();
  const char* const start = str.c_str();
  const char* const end = start + str.length();

  // We need to use a helper StrStream to do this transformation
  // because String doesn't support push_back().
  StrStream helper;
  for (const char* ch = start; ch != end; ++ch) {
    if (*ch == '\0') {
      helper << "\\0";  // Replaces NUL with "\\0";
    } else {
      helper.put(*ch);
    }
  }

  return String(helper.str().c_str());
}

// Appends the user-supplied message to the Google-Test-generated message.
String AppendUserMessage(const String& gtest_msg,
                         const Message& user_msg) {
  // Appends the user message if it's non-empty.
  const String user_msg_string = user_msg.GetString();
  if (user_msg_string.empty()) {
    return gtest_msg;
  }

  Message msg;
  msg << gtest_msg << "\n" << user_msg_string;

  return msg.GetString();
}

}  // namespace internal

// class TestResult

// Creates an empty TestResult.
TestResult::TestResult()
    : death_test_count_(0),
      elapsed_time_(0) {
}

// D'tor.
TestResult::~TestResult() {
}

// Returns the i-th test part result among all the results. i can
// range from 0 to total_part_count() - 1. If i is not in that range,
// aborts the program.
const TestPartResult& TestResult::GetTestPartResult(int i) const {
  if (i < 0 || i >= total_part_count())
    internal::posix::Abort();
  return test_part_results_.at(i);
}

// Returns the i-th test property. i can range from 0 to
// test_property_count() - 1. If i is not in that range, aborts the
// program.
const TestProperty& TestResult::GetTestProperty(int i) const {
  if (i < 0 || i >= test_property_count())
    internal::posix::Abort();
  return test_properties_.at(i);
}

// Clears the test part results.
void TestResult::ClearTestPartResults() {
  test_part_results_.clear();
}

// Adds a test part result to the list.
void TestResult::AddTestPartResult(const TestPartResult& test_part_result) {
  test_part_results_.push_back(test_part_result);
}

// Adds a test property to the list. If a property with the same key as the
// supplied property is already represented, the value of this test_property
// replaces the old value for that key.
void TestResult::RecordProperty(const TestProperty& test_property) {
  if (!ValidateTestProperty(test_property)) {
    return;
  }
  internal::MutexLock lock(&test_properites_mutex_);
  const std::vector<TestProperty>::iterator property_with_matching_key =
      std::find_if(test_properties_.begin(), test_properties_.end(),
                   internal::TestPropertyKeyIs(test_property.key()));
  if (property_with_matching_key == test_properties_.end()) {
    test_properties_.push_back(test_property);
    return;
  }
  property_with_matching_key->SetValue(test_property.value());
}

// Adds a failure if the key is a reserved attribute of Google Test
// testcase tags.  Returns true if the property is valid.
bool TestResult::ValidateTestProperty(const TestProperty& test_property) {
  internal::String key(test_property.key());
  if (key == "name" || key == "status" || key == "time" || key == "classname") {
    ADD_FAILURE()
        << "Reserved key used in RecordProperty(): "
        << key
        << " ('name', 'status', 'time', and 'classname' are reserved by "
        << GTEST_NAME_ << ")";
    return false;
  }
  return true;
}

// Clears the object.
void TestResult::Clear() {
  test_part_results_.clear();
  test_properties_.clear();
  death_test_count_ = 0;
  elapsed_time_ = 0;
}

// Returns true iff the test failed.
bool TestResult::Failed() const {
  for (int i = 0; i < total_part_count(); ++i) {
    if (GetTestPartResult(i).failed())
      return true;
  }
  return false;
}

// Returns true iff the test part fatally failed.
static bool TestPartFatallyFailed(const TestPartResult& result) {
  return result.fatally_failed();
}

// Returns true iff the test fatally failed.
bool TestResult::HasFatalFailure() const {
  return CountIf(test_part_results_, TestPartFatallyFailed) > 0;
}

// Returns true iff the test part non-fatally failed.
static bool TestPartNonfatallyFailed(const TestPartResult& result) {
  return result.nonfatally_failed();
}

// Returns true iff the test has a non-fatal failure.
bool TestResult::HasNonfatalFailure() const {
  return CountIf(test_part_results_, TestPartNonfatallyFailed) > 0;
}

// Gets the number of all test parts.  This is the sum of the number
// of successful test parts and the number of failed test parts.
int TestResult::total_part_count() const {
  return static_cast<int>(test_part_results_.size());
}

// Returns the number of the test properties.
int TestResult::test_property_count() const {
  return static_cast<int>(test_properties_.size());
}

// class Test

// Creates a Test object.

// The c'tor saves the values of all Google Test flags.
Test::Test()
    : gtest_flag_saver_(new internal::GTestFlagSaver) {
}

// The d'tor restores the values of all Google Test flags.
Test::~Test() {
  delete gtest_flag_saver_;
}

// Sets up the test fixture.
//
// A sub-class may override this.
void Test::SetUp() {
}

// Tears down the test fixture.
//
// A sub-class may override this.
void Test::TearDown() {
}

// Allows user supplied key value pairs to be recorded for later output.
void Test::RecordProperty(const char* key, const char* value) {
  UnitTest::GetInstance()->RecordPropertyForCurrentTest(key, value);
}

// Allows user supplied key value pairs to be recorded for later output.
void Test::RecordProperty(const char* key, int value) {
  Message value_message;
  value_message << value;
  RecordProperty(key, value_message.GetString().c_str());
}

namespace internal {

void ReportFailureInUnknownLocation(TestPartResult::Type result_type,
                                    const String& message) {
  // This function is a friend of UnitTest and as such has access to
  // AddTestPartResult.
  UnitTest::GetInstance()->AddTestPartResult(
      result_type,
      NULL,  // No info about the source file where the exception occurred.
      -1,    // We have no info on which line caused the exception.
      message,
      String());  // No stack trace, either.
}

}  // namespace internal

#if GTEST_HAS_SEH
// We are on Windows with SEH.

// Adds an "exception thrown" fatal failure to the current test.
static void AddExceptionThrownFailure(DWORD exception_code,
                                      const char* location) {
  Message message;
  message << "Exception thrown with code 0x" << std::setbase(16) <<
    exception_code << std::setbase(10) << " in " << location << ".";

  internal::ReportFailureInUnknownLocation(TestPartResult::kFatalFailure,
                                           message.GetString());
}

#endif  // GTEST_HAS_SEH

// Google Test requires all tests in the same test case to use the same test
// fixture class.  This function checks if the current test has the
// same fixture class as the first test in the current test case.  If
// yes, it returns true; otherwise it generates a Google Test failure and
// returns false.
bool Test::HasSameFixtureClass() {
  internal::UnitTestImpl* const impl = internal::GetUnitTestImpl();
  const TestCase* const test_case = impl->current_test_case();

  // Info about the first test in the current test case.
  const internal::TestInfoImpl* const first_test_info =
      test_case->test_info_list()[0]->impl();
  const internal::TypeId first_fixture_id = first_test_info->fixture_class_id();
  const char* const first_test_name = first_test_info->name();

  // Info about the current test.
  const internal::TestInfoImpl* const this_test_info =
      impl->current_test_info()->impl();
  const internal::TypeId this_fixture_id = this_test_info->fixture_class_id();
  const char* const this_test_name = this_test_info->name();

  if (this_fixture_id != first_fixture_id) {
    // Is the first test defined using TEST?
    const bool first_is_TEST = first_fixture_id == internal::GetTestTypeId();
    // Is this test defined using TEST?
    const bool this_is_TEST = this_fixture_id == internal::GetTestTypeId();

    if (first_is_TEST || this_is_TEST) {
      // The user mixed TEST and TEST_F in this test case - we'll tell
      // him/her how to fix it.

      // Gets the name of the TEST and the name of the TEST_F.  Note
      // that first_is_TEST and this_is_TEST cannot both be true, as
      // the fixture IDs are different for the two tests.
      const char* const TEST_name =
          first_is_TEST ? first_test_name : this_test_name;
      const char* const TEST_F_name =
          first_is_TEST ? this_test_name : first_test_name;

      ADD_FAILURE()
          << "All tests in the same test case must use the same test fixture\n"
          << "class, so mixing TEST_F and TEST in the same test case is\n"
          << "illegal.  In test case " << this_test_info->test_case_name()
          << ",\n"
          << "test " << TEST_F_name << " is defined using TEST_F but\n"
          << "test " << TEST_name << " is defined using TEST.  You probably\n"
          << "want to change the TEST to TEST_F or move it to another test\n"
          << "case.";
    } else {
      // The user defined two fixture classes with the same name in
      // two namespaces - we'll tell him/her how to fix it.
      ADD_FAILURE()
          << "All tests in the same test case must use the same test fixture\n"
          << "class.  However, in test case "
          << this_test_info->test_case_name() << ",\n"
          << "you defined test " << first_test_name
          << " and test " << this_test_name << "\n"
          << "using two different test fixture classes.  This can happen if\n"
          << "the two classes are from different namespaces or translation\n"
          << "units and have the same name.  You should probably rename one\n"
          << "of the classes to put the tests into different test cases.";
    }
    return false;
  }

  return true;
}

// Runs the test and updates the test result.
void Test::Run() {
  if (!HasSameFixtureClass()) return;

  internal::UnitTestImpl* const impl = internal::GetUnitTestImpl();
#if GTEST_HAS_SEH
  // Catch SEH-style exceptions.
  impl->os_stack_trace_getter()->UponLeavingGTest();
  __try {
    SetUp();
  } __except(internal::UnitTestOptions::GTestShouldProcessSEH(
      GetExceptionCode())) {
    AddExceptionThrownFailure(GetExceptionCode(), "SetUp()");
  }

  // We will run the test only if SetUp() had no fatal failure.
  if (!HasFatalFailure()) {
    impl->os_stack_trace_getter()->UponLeavingGTest();
    __try {
      TestBody();
    } __except(internal::UnitTestOptions::GTestShouldProcessSEH(
        GetExceptionCode())) {
      AddExceptionThrownFailure(GetExceptionCode(), "the test body");
    }
  }

  // However, we want to clean up as much as possible.  Hence we will
  // always call TearDown(), even if SetUp() or the test body has
  // failed.
  impl->os_stack_trace_getter()->UponLeavingGTest();
  __try {
    TearDown();
  } __except(internal::UnitTestOptions::GTestShouldProcessSEH(
      GetExceptionCode())) {
    AddExceptionThrownFailure(GetExceptionCode(), "TearDown()");
  }

#else  // We are on a compiler or platform that doesn't support SEH.
  impl->os_stack_trace_getter()->UponLeavingGTest();
  SetUp();

  // We will run the test only if SetUp() was successful.
  if (!HasFatalFailure()) {
    impl->os_stack_trace_getter()->UponLeavingGTest();
    TestBody();
  }

  // However, we want to clean up as much as possible.  Hence we will
  // always call TearDown(), even if SetUp() or the test body has
  // failed.
  impl->os_stack_trace_getter()->UponLeavingGTest();
  TearDown();
#endif  // GTEST_HAS_SEH
}


// Returns true iff the current test has a fatal failure.
bool Test::HasFatalFailure() {
  return internal::GetUnitTestImpl()->current_test_result()->HasFatalFailure();
}

// Returns true iff the current test has a non-fatal failure.
bool Test::HasNonfatalFailure() {
  return internal::GetUnitTestImpl()->current_test_result()->
      HasNonfatalFailure();
}

// class TestInfo

// Constructs a TestInfo object. It assumes ownership of the test factory
// object via impl_.
TestInfo::TestInfo(const char* a_test_case_name,
                   const char* a_name,
                   const char* a_test_case_comment,
                   const char* a_comment,
                   internal::TypeId fixture_class_id,
                   internal::TestFactoryBase* factory) {
  impl_ = new internal::TestInfoImpl(this, a_test_case_name, a_name,
                                     a_test_case_comment, a_comment,
                                     fixture_class_id, factory);
}

// Destructs a TestInfo object.
TestInfo::~TestInfo() {
  delete impl_;
}

namespace internal {

// Creates a new TestInfo object and registers it with Google Test;
// returns the created object.
//
// Arguments:
//
//   test_case_name:   name of the test case
//   name:             name of the test
//   test_case_comment: a comment on the test case that will be included in
//                      the test output
//   comment:          a comment on the test that will be included in the
//                     test output
//   fixture_class_id: ID of the test fixture class
//   set_up_tc:        pointer to the function that sets up the test case
//   tear_down_tc:     pointer to the function that tears down the test case
//   factory:          pointer to the factory that creates a test object.
//                     The newly created TestInfo instance will assume
//                     ownership of the factory object.
TestInfo* MakeAndRegisterTestInfo(
    const char* test_case_name, const char* name,
    const char* test_case_comment, const char* comment,
    TypeId fixture_class_id,
    SetUpTestCaseFunc set_up_tc,
    TearDownTestCaseFunc tear_down_tc,
    TestFactoryBase* factory) {
  TestInfo* const test_info =
      new TestInfo(test_case_name, name, test_case_comment, comment,
                   fixture_class_id, factory);
  GetUnitTestImpl()->AddTestInfo(set_up_tc, tear_down_tc, test_info);
  return test_info;
}

#if GTEST_HAS_PARAM_TEST
void ReportInvalidTestCaseType(const char* test_case_name,
                               const char* file, int line) {
  Message errors;
  errors
      << "Attempted redefinition of test case " << test_case_name << ".\n"
      << "All tests in the same test case must use the same test fixture\n"
      << "class.  However, in test case " << test_case_name << ", you tried\n"
      << "to define a test using a fixture class different from the one\n"
      << "used earlier. This can happen if the two fixture classes are\n"
      << "from different namespaces and have the same name. You should\n"
      << "probably rename one of the classes to put the tests into different\n"
      << "test cases.";

  fprintf(stderr, "%s %s", FormatFileLocation(file, line).c_str(),
          errors.GetString().c_str());
}
#endif  // GTEST_HAS_PARAM_TEST

}  // namespace internal

// Returns the test case name.
const char* TestInfo::test_case_name() const {
  return impl_->test_case_name();
}

// Returns the test name.
const char* TestInfo::name() const {
  return impl_->name();
}

// Returns the test case comment.
const char* TestInfo::test_case_comment() const {
  return impl_->test_case_comment();
}

// Returns the test comment.
const char* TestInfo::comment() const {
  return impl_->comment();
}

// Returns true if this test should run.
bool TestInfo::should_run() const { return impl_->should_run(); }

// Returns true if this test matches the user-specified filter.
bool TestInfo::matches_filter() const { return impl_->matches_filter(); }

// Returns the result of the test.
const TestResult* TestInfo::result() const { return impl_->result(); }

// Increments the number of death tests encountered in this test so
// far.
int TestInfo::increment_death_test_count() {
  return impl_->result()->increment_death_test_count();
}

namespace internal {

// This method expands all parameterized tests registered with macros TEST_P
// and INSTANTIATE_TEST_CASE_P into regular tests and registers those.
// This will be done just once during the program runtime.
void UnitTestImpl::RegisterParameterizedTests() {
#if GTEST_HAS_PARAM_TEST
  if (!parameterized_tests_registered_) {
    parameterized_test_registry_.RegisterTests();
    parameterized_tests_registered_ = true;
  }
#endif
}

// Creates the test object, runs it, records its result, and then
// deletes it.
void TestInfoImpl::Run() {
  if (!should_run_) return;

  // Tells UnitTest where to store test result.
  UnitTestImpl* const impl = internal::GetUnitTestImpl();
  impl->set_current_test_info(parent_);

  TestEventListener* repeater = UnitTest::GetInstance()->listeners().repeater();

  // Notifies the unit test event listeners that a test is about to start.
  repeater->OnTestStart(*parent_);

  const TimeInMillis start = GetTimeInMillis();

  impl->os_stack_trace_getter()->UponLeavingGTest();
#if GTEST_HAS_SEH
  // Catch SEH-style exceptions.
  Test* test = NULL;

  __try {
    // Creates the test object.
    test = factory_->CreateTest();
  } __except(internal::UnitTestOptions::GTestShouldProcessSEH(
      GetExceptionCode())) {
    AddExceptionThrownFailure(GetExceptionCode(),
                              "the test fixture's constructor");
    return;
  }
#else  // We are on a compiler or platform that doesn't support SEH.

  // TODO(wan): If test->Run() throws, test won't be deleted.  This is
  // not a problem now as we don't use exceptions.  If we were to
  // enable exceptions, we should revise the following to be
  // exception-safe.

  // Creates the test object.
  Test* test = factory_->CreateTest();
#endif  // GTEST_HAS_SEH

  // Runs the test only if the constructor of the test fixture didn't
  // generate a fatal failure.
  if (!Test::HasFatalFailure()) {
    test->Run();
  }

  // Deletes the test object.
  impl->os_stack_trace_getter()->UponLeavingGTest();
  delete test;
  test = NULL;

  result_.set_elapsed_time(GetTimeInMillis() - start);

  // Notifies the unit test event listener that a test has just finished.
  repeater->OnTestEnd(*parent_);

  // Tells UnitTest to stop associating assertion results to this
  // test.
  impl->set_current_test_info(NULL);
}

}  // namespace internal

// class TestCase

// Gets the number of successful tests in this test case.
int TestCase::successful_test_count() const {
  return CountIf(test_info_list_, TestPassed);
}

// Gets the number of failed tests in this test case.
int TestCase::failed_test_count() const {
  return CountIf(test_info_list_, TestFailed);
}

int TestCase::disabled_test_count() const {
  return CountIf(test_info_list_, TestDisabled);
}

// Get the number of tests in this test case that should run.
int TestCase::test_to_run_count() const {
  return CountIf(test_info_list_, ShouldRunTest);
}

// Gets the number of all tests.
int TestCase::total_test_count() const {
  return static_cast<int>(test_info_list_.size());
}

// Creates a TestCase with the given name.
//
// Arguments:
//
//   name:         name of the test case
//   set_up_tc:    pointer to the function that sets up the test case
//   tear_down_tc: pointer to the function that tears down the test case
TestCase::TestCase(const char* a_name, const char* a_comment,
                   Test::SetUpTestCaseFunc set_up_tc,
                   Test::TearDownTestCaseFunc tear_down_tc)
    : name_(a_name),
      comment_(a_comment),
      set_up_tc_(set_up_tc),
      tear_down_tc_(tear_down_tc),
      should_run_(false),
      elapsed_time_(0) {
}

// Destructor of TestCase.
TestCase::~TestCase() {
  // Deletes every Test in the collection.
  ForEach(test_info_list_, internal::Delete<TestInfo>);
}

// Returns the i-th test among all the tests. i can range from 0 to
// total_test_count() - 1. If i is not in that range, returns NULL.
const TestInfo* TestCase::GetTestInfo(int i) const {
  const int index = GetElementOr(test_indices_, i, -1);
  return index < 0 ? NULL : test_info_list_[index];
}

// Returns the i-th test among all the tests. i can range from 0 to
// total_test_count() - 1. If i is not in that range, returns NULL.
TestInfo* TestCase::GetMutableTestInfo(int i) {
  const int index = GetElementOr(test_indices_, i, -1);
  return index < 0 ? NULL : test_info_list_[index];
}

// Adds a test to this test case.  Will delete the test upon
// destruction of the TestCase object.
void TestCase::AddTestInfo(TestInfo * test_info) {
  test_info_list_.push_back(test_info);
  test_indices_.push_back(static_cast<int>(test_indices_.size()));
}

// Runs every test in this TestCase.
void TestCase::Run() {
  if (!should_run_) return;

  internal::UnitTestImpl* const impl = internal::GetUnitTestImpl();
  impl->set_current_test_case(this);

  TestEventListener* repeater = UnitTest::GetInstance()->listeners().repeater();

  repeater->OnTestCaseStart(*this);
  impl->os_stack_trace_getter()->UponLeavingGTest();
  set_up_tc_();

  const internal::TimeInMillis start = internal::GetTimeInMillis();
  for (int i = 0; i < total_test_count(); i++) {
    GetMutableTestInfo(i)->impl()->Run();
  }
  elapsed_time_ = internal::GetTimeInMillis() - start;

  impl->os_stack_trace_getter()->UponLeavingGTest();
  tear_down_tc_();
  repeater->OnTestCaseEnd(*this);
  impl->set_current_test_case(NULL);
}

// Clears the results of all tests in this test case.
void TestCase::ClearResult() {
  ForEach(test_info_list_, internal::TestInfoImpl::ClearTestResult);
}

// Returns true iff test passed.
bool TestCase::TestPassed(const TestInfo * test_info) {
  const internal::TestInfoImpl* const impl = test_info->impl();
  return impl->should_run() && impl->result()->Passed();
}

// Returns true iff test failed.
bool TestCase::TestFailed(const TestInfo * test_info) {
  const internal::TestInfoImpl* const impl = test_info->impl();
  return impl->should_run() && impl->result()->Failed();
}

// Returns true iff test is disabled.
bool TestCase::TestDisabled(const TestInfo * test_info) {
  return test_info->impl()->is_disabled();
}

// Returns true if the given test should run.
bool TestCase::ShouldRunTest(const TestInfo *test_info) {
  return test_info->impl()->should_run();
}

// Shuffles the tests in this test case.
void TestCase::ShuffleTests(internal::Random* random) {
  Shuffle(random, &test_indices_);
}

// Restores the test order to before the first shuffle.
void TestCase::UnshuffleTests() {
  for (size_t i = 0; i < test_indices_.size(); i++) {
    test_indices_[i] = static_cast<int>(i);
  }
}

// Formats a countable noun.  Depending on its quantity, either the
// singular form or the plural form is used. e.g.
//
// FormatCountableNoun(1, "formula", "formuli") returns "1 formula".
// FormatCountableNoun(5, "book", "books") returns "5 books".
static internal::String FormatCountableNoun(int count,
                                            const char * singular_form,
                                            const char * plural_form) {
  return internal::String::Format("%d %s", count,
                                  count == 1 ? singular_form : plural_form);
}

// Formats the count of tests.
static internal::String FormatTestCount(int test_count) {
  return FormatCountableNoun(test_count, "test", "tests");
}

// Formats the count of test cases.
static internal::String FormatTestCaseCount(int test_case_count) {
  return FormatCountableNoun(test_case_count, "test case", "test cases");
}

// Converts a TestPartResult::Type enum to human-friendly string
// representation.  Both kNonFatalFailure and kFatalFailure are translated
// to "Failure", as the user usually doesn't care about the difference
// between the two when viewing the test result.
static const char * TestPartResultTypeToString(TestPartResult::Type type) {
  switch (type) {
    case TestPartResult::kSuccess:
      return "Success";

    case TestPartResult::kNonFatalFailure:
    case TestPartResult::kFatalFailure:
#ifdef _MSC_VER
      return "error: ";
#else
      return "Failure\n";
#endif
  }

  return "Unknown result type";
}

// Prints a TestPartResult to a String.
static internal::String PrintTestPartResultToString(
    const TestPartResult& test_part_result) {
  return (Message()
          << internal::FormatFileLocation(test_part_result.file_name(),
                                          test_part_result.line_number())
          << " " << TestPartResultTypeToString(test_part_result.type())
          << test_part_result.message()).GetString();
}

// Prints a TestPartResult.
static void PrintTestPartResult(const TestPartResult& test_part_result) {
  const internal::String& result =
      PrintTestPartResultToString(test_part_result);
  printf("%s\n", result.c_str());
  fflush(stdout);
  // If the test program runs in Visual Studio or a debugger, the
  // following statements add the test part result message to the Output
  // window such that the user can double-click on it to jump to the
  // corresponding source code location; otherwise they do nothing.
#if GTEST_OS_WINDOWS && !GTEST_OS_WINDOWS_MOBILE
  // We don't call OutputDebugString*() on Windows Mobile, as printing
  // to stdout is done by OutputDebugString() there already - we don't
  // want the same message printed twice.
  ::OutputDebugStringA(result.c_str());
  ::OutputDebugStringA("\n");
#endif
}

// class PrettyUnitTestResultPrinter

namespace internal {

enum GTestColor {
  COLOR_DEFAULT,
  COLOR_RED,
  COLOR_GREEN,
  COLOR_YELLOW
};

#if GTEST_OS_WINDOWS && !GTEST_OS_WINDOWS_MOBILE

// Returns the character attribute for the given color.
WORD GetColorAttribute(GTestColor color) {
  switch (color) {
    case COLOR_RED:    return FOREGROUND_RED;
    case COLOR_GREEN:  return FOREGROUND_GREEN;
    case COLOR_YELLOW: return FOREGROUND_RED | FOREGROUND_GREEN;
    default:           return 0;
  }
}

#else

// Returns the ANSI color code for the given color.  COLOR_DEFAULT is
// an invalid input.
const char* GetAnsiColorCode(GTestColor color) {
  switch (color) {
    case COLOR_RED:     return "1";
    case COLOR_GREEN:   return "2";
    case COLOR_YELLOW:  return "3";
    default:            return NULL;
  };
}

#endif  // GTEST_OS_WINDOWS && !GTEST_OS_WINDOWS_MOBILE

// Returns true iff Google Test should use colors in the output.
bool ShouldUseColor(bool stdout_is_tty) {
  const char* const gtest_color = GTEST_FLAG(color).c_str();

  if (String::CaseInsensitiveCStringEquals(gtest_color, "auto")) {
#if GTEST_OS_WINDOWS
    // On Windows the TERM variable is usually not set, but the
    // console there does support colors.
    return stdout_is_tty;
#else
    // On non-Windows platforms, we rely on the TERM variable.
    const char* const term = posix::GetEnv("TERM");
    const bool term_supports_color =
        String::CStringEquals(term, "xterm") ||
        String::CStringEquals(term, "xterm-color") ||
        String::CStringEquals(term, "xterm-256color") ||
        String::CStringEquals(term, "linux") ||
        String::CStringEquals(term, "cygwin");
    return stdout_is_tty && term_supports_color;
#endif  // GTEST_OS_WINDOWS
  }

  return String::CaseInsensitiveCStringEquals(gtest_color, "yes") ||
      String::CaseInsensitiveCStringEquals(gtest_color, "true") ||
      String::CaseInsensitiveCStringEquals(gtest_color, "t") ||
      String::CStringEquals(gtest_color, "1");
  // We take "yes", "true", "t", and "1" as meaning "yes".  If the
  // value is neither one of these nor "auto", we treat it as "no" to
  // be conservative.
}

// Helpers for printing colored strings to stdout. Note that on Windows, we
// cannot simply emit special characters and have the terminal change colors.
// This routine must actually emit the characters rather than return a string
// that would be colored when printed, as can be done on Linux.
void ColoredPrintf(GTestColor color, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

#if GTEST_OS_WINDOWS_MOBILE || GTEST_OS_SYMBIAN || GTEST_OS_ZOS
  const bool use_color = false;
#else
  static const bool in_color_mode =
      ShouldUseColor(posix::IsATTY(posix::FileNo(stdout)) != 0);
  const bool use_color = in_color_mode && (color != COLOR_DEFAULT);
#endif  // GTEST_OS_WINDOWS_MOBILE || GTEST_OS_SYMBIAN || GTEST_OS_ZOS
  // The '!= 0' comparison is necessary to satisfy MSVC 7.1.

  if (!use_color) {
    vprintf(fmt, args);
    va_end(args);
    return;
  }

#if GTEST_OS_WINDOWS && !GTEST_OS_WINDOWS_MOBILE
  const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

  // Gets the current text color.
  CONSOLE_SCREEN_BUFFER_INFO buffer_info;
  GetConsoleScreenBufferInfo(stdout_handle, &buffer_info);
  const WORD old_color_attrs = buffer_info.wAttributes;

  // We need to flush the stream buffers into the console before each
  // SetConsoleTextAttribute call lest it affect the text that is already
  // printed but has not yet reached the console.
  fflush(stdout);
  SetConsoleTextAttribute(stdout_handle,
                          GetColorAttribute(color) | FOREGROUND_INTENSITY);
  vprintf(fmt, args);

  fflush(stdout);
  // Restores the text color.
  SetConsoleTextAttribute(stdout_handle, old_color_attrs);
#else
  printf("\033[0;3%sm", GetAnsiColorCode(color));
  vprintf(fmt, args);
  printf("\033[m");  // Resets the terminal to default.
#endif  // GTEST_OS_WINDOWS && !GTEST_OS_WINDOWS_MOBILE
  va_end(args);
}

// This class implements the TestEventListener interface.
//
// Class PrettyUnitTestResultPrinter is copyable.
class PrettyUnitTestResultPrinter : public TestEventListener {
 public:
  PrettyUnitTestResultPrinter() {}
  static void PrintTestName(const char * test_case, const char * test) {
    printf("%s.%s", test_case, test);
  }

  // The following methods override what's in the TestEventListener class.
  virtual void OnTestProgramStart(const UnitTest& /*unit_test*/) {}
  virtual void OnTestIterationStart(const UnitTest& unit_test, int iteration);
  virtual void OnEnvironmentsSetUpStart(const UnitTest& unit_test);
  virtual void OnEnvironmentsSetUpEnd(const UnitTest& /*unit_test*/) {}
  virtual void OnTestCaseStart(const TestCase& test_case);
  virtual void OnTestStart(const TestInfo& test_info);
  virtual void OnTestPartResult(const TestPartResult& result);
  virtual void OnTestEnd(const TestInfo& test_info);
  virtual void OnTestCaseEnd(const TestCase& test_case);
  virtual void OnEnvironmentsTearDownStart(const UnitTest& unit_test);
  virtual void OnEnvironmentsTearDownEnd(const UnitTest& /*unit_test*/) {}
  virtual void OnTestIterationEnd(const UnitTest& unit_test, int iteration);
  virtual void OnTestProgramEnd(const UnitTest& /*unit_test*/) {}

 private:
  static void PrintFailedTests(const UnitTest& unit_test);

  internal::String test_case_name_;
};

  // Fired before each iteration of tests starts.
void PrettyUnitTestResultPrinter::OnTestIterationStart(
    const UnitTest& unit_test, int iteration) {
  if (GTEST_FLAG(repeat) != 1)
    printf("\nRepeating all tests (iteration %d) . . .\n\n", iteration + 1);

  const char* const filter = GTEST_FLAG(filter).c_str();

  // Prints the filter if it's not *.  This reminds the user that some
  // tests may be skipped.
  if (!internal::String::CStringEquals(filter, kUniversalFilter)) {
    ColoredPrintf(COLOR_YELLOW,
                  "Note: %s filter = %s\n", GTEST_NAME_, filter);
  }

  if (internal::ShouldShard(kTestTotalShards, kTestShardIndex, false)) {
    ColoredPrintf(COLOR_YELLOW,
                  "Note: This is test shard %s of %s.\n",
                  internal::posix::GetEnv(kTestShardIndex),
                  internal::posix::GetEnv(kTestTotalShards));
  }

  if (GTEST_FLAG(shuffle)) {
    ColoredPrintf(COLOR_YELLOW,
                  "Note: Randomizing tests' orders with a seed of %d .\n",
                  unit_test.random_seed());
  }

  ColoredPrintf(COLOR_GREEN,  "[==========] ");
  printf("Running %s from %s.\n",
         FormatTestCount(unit_test.test_to_run_count()).c_str(),
         FormatTestCaseCount(unit_test.test_case_to_run_count()).c_str());
  fflush(stdout);
}

void PrettyUnitTestResultPrinter::OnEnvironmentsSetUpStart(
    const UnitTest& /*unit_test*/) {
  ColoredPrintf(COLOR_GREEN,  "[----------] ");
  printf("Global test environment set-up.\n");
  fflush(stdout);
}

void PrettyUnitTestResultPrinter::OnTestCaseStart(const TestCase& test_case) {
  test_case_name_ = test_case.name();
  const internal::String counts =
      FormatCountableNoun(test_case.test_to_run_count(), "test", "tests");
  ColoredPrintf(COLOR_GREEN, "[----------] ");
  printf("%s from %s", counts.c_str(), test_case_name_.c_str());
  if (test_case.comment()[0] == '\0') {
    printf("\n");
  } else {
    printf(", where %s\n", test_case.comment());
  }
  fflush(stdout);
}

void PrettyUnitTestResultPrinter::OnTestStart(const TestInfo& test_info) {
  ColoredPrintf(COLOR_GREEN,  "[ RUN      ] ");
  PrintTestName(test_case_name_.c_str(), test_info.name());
  if (test_info.comment()[0] == '\0') {
    printf("\n");
  } else {
    printf(", where %s\n", test_info.comment());
  }
  fflush(stdout);
}

// Called after an assertion failure.
void PrettyUnitTestResultPrinter::OnTestPartResult(
    const TestPartResult& result) {
  // If the test part succeeded, we don't need to do anything.
  if (result.type() == TestPartResult::kSuccess)
    return;

  // Print failure message from the assertion (e.g. expected this and got that).
  PrintTestPartResult(result);
  fflush(stdout);
}

void PrettyUnitTestResultPrinter::OnTestEnd(const TestInfo& test_info) {
  if (test_info.result()->Passed()) {
    ColoredPrintf(COLOR_GREEN, "[       OK ] ");
  } else {
    ColoredPrintf(COLOR_RED, "[  FAILED  ] ");
  }
  PrintTestName(test_case_name_.c_str(), test_info.name());
  if (GTEST_FLAG(print_time)) {
    printf(" (%s ms)\n", internal::StreamableToString(
           test_info.result()->elapsed_time()).c_str());
  } else {
    printf("\n");
  }
  fflush(stdout);
}

void PrettyUnitTestResultPrinter::OnTestCaseEnd(const TestCase& test_case) {
  if (!GTEST_FLAG(print_time)) return;

  test_case_name_ = test_case.name();
  const internal::String counts =
      FormatCountableNoun(test_case.test_to_run_count(), "test", "tests");
  ColoredPrintf(COLOR_GREEN, "[----------] ");
  printf("%s from %s (%s ms total)\n\n",
         counts.c_str(), test_case_name_.c_str(),
         internal::StreamableToString(test_case.elapsed_time()).c_str());
  fflush(stdout);
}

void PrettyUnitTestResultPrinter::OnEnvironmentsTearDownStart(
    const UnitTest& /*unit_test*/) {
  ColoredPrintf(COLOR_GREEN,  "[----------] ");
  printf("Global test environment tear-down\n");
  fflush(stdout);
}

// Internal helper for printing the list of failed tests.
void PrettyUnitTestResultPrinter::PrintFailedTests(const UnitTest& unit_test) {
  const int failed_test_count = unit_test.failed_test_count();
  if (failed_test_count == 0) {
    return;
  }

  for (int i = 0; i < unit_test.total_test_case_count(); ++i) {
    const TestCase& test_case = *unit_test.GetTestCase(i);
    if (!test_case.should_run() || (test_case.failed_test_count() == 0)) {
      continue;
    }
    for (int j = 0; j < test_case.total_test_count(); ++j) {
      const TestInfo& test_info = *test_case.GetTestInfo(j);
      if (!test_info.should_run() || test_info.result()->Passed()) {
        continue;
      }
      ColoredPrintf(COLOR_RED, "[  FAILED  ] ");
      printf("%s.%s", test_case.name(), test_info.name());
      if (test_case.comment()[0] != '\0' ||
          test_info.comment()[0] != '\0') {
        printf(", where %s", test_case.comment());
        if (test_case.comment()[0] != '\0' &&
            test_info.comment()[0] != '\0') {
          printf(" and ");
        }
      }
      printf("%s\n", test_info.comment());
    }
  }
}

 void PrettyUnitTestResultPrinter::OnTestIterationEnd(const UnitTest& unit_test,
                                                      int /*iteration*/) {
  ColoredPrintf(COLOR_GREEN,  "[==========] ");
  printf("%s from %s ran.",
         FormatTestCount(unit_test.test_to_run_count()).c_str(),
         FormatTestCaseCount(unit_test.test_case_to_run_count()).c_str());
  if (GTEST_FLAG(print_time)) {
    printf(" (%s ms total)",
           internal::StreamableToString(unit_test.elapsed_time()).c_str());
  }
  printf("\n");
  ColoredPrintf(COLOR_GREEN,  "[  PASSED  ] ");
  printf("%s.\n", FormatTestCount(unit_test.successful_test_count()).c_str());

  int num_failures = unit_test.failed_test_count();
  if (!unit_test.Passed()) {
    const int failed_test_count = unit_test.failed_test_count();
    ColoredPrintf(COLOR_RED,  "[  FAILED  ] ");
    printf("%s, listed below:\n", FormatTestCount(failed_test_count).c_str());
    PrintFailedTests(unit_test);
    printf("\n%2d FAILED %s\n", num_failures,
                        num_failures == 1 ? "TEST" : "TESTS");
  }

  int num_disabled = unit_test.disabled_test_count();
  if (num_disabled && !GTEST_FLAG(also_run_disabled_tests)) {
    if (!num_failures) {
      printf("\n");  // Add a spacer if no FAILURE banner is displayed.
    }
    ColoredPrintf(COLOR_YELLOW,
                  "  YOU HAVE %d DISABLED %s\n\n",
                  num_disabled,
                  num_disabled == 1 ? "TEST" : "TESTS");
  }
  // Ensure that Google Test output is printed before, e.g., heapchecker output.
  fflush(stdout);
}

// End PrettyUnitTestResultPrinter

// class TestEventRepeater
//
// This class forwards events to other event listeners.
class TestEventRepeater : public TestEventListener {
 public:
  TestEventRepeater() : forwarding_enabled_(true) {}
  virtual ~TestEventRepeater();
  void Append(TestEventListener *listener);
  TestEventListener* Release(TestEventListener* listener);

  // Controls whether events will be forwarded to listeners_. Set to false
  // in death test child processes.
  bool forwarding_enabled() const { return forwarding_enabled_; }
  void set_forwarding_enabled(bool enable) { forwarding_enabled_ = enable; }

  virtual void OnTestProgramStart(const UnitTest& unit_test);
  virtual void OnTestIterationStart(const UnitTest& unit_test, int iteration);
  virtual void OnEnvironmentsSetUpStart(const UnitTest& unit_test);
  virtual void OnEnvironmentsSetUpEnd(const UnitTest& unit_test);
  virtual void OnTestCaseStart(const TestCase& test_case);
  virtual void OnTestStart(const TestInfo& test_info);
  virtual void OnTestPartResult(const TestPartResult& result);
  virtual void OnTestEnd(const TestInfo& test_info);
  virtual void OnTestCaseEnd(const TestCase& test_case);
  virtual void OnEnvironmentsTearDownStart(const UnitTest& unit_test);
  virtual void OnEnvironmentsTearDownEnd(const UnitTest& unit_test);
  virtual void OnTestIterationEnd(const UnitTest& unit_test, int iteration);
  virtual void OnTestProgramEnd(const UnitTest& unit_test);

 private:
  // Controls whether events will be forwarded to listeners_. Set to false
  // in death test child processes.
  bool forwarding_enabled_;
  // The list of listeners that receive events.
  std::vector<TestEventListener*> listeners_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(TestEventRepeater);
};

TestEventRepeater::~TestEventRepeater() {
  ForEach(listeners_, Delete<TestEventListener>);
}

void TestEventRepeater::Append(TestEventListener *listener) {
  listeners_.push_back(listener);
}

// TODO(vladl@google.com): Factor the search functionality into Vector::Find.
TestEventListener* TestEventRepeater::Release(TestEventListener *listener) {
  for (size_t i = 0; i < listeners_.size(); ++i) {
    if (listeners_[i] == listener) {
      listeners_.erase(listeners_.begin() + i);
      return listener;
    }
  }

  return NULL;
}

// Since most methods are very similar, use macros to reduce boilerplate.
// This defines a member that forwards the call to all listeners.
#define GTEST_REPEATER_METHOD_(Name, Type) \
void TestEventRepeater::Name(const Type& parameter) { \
  if (forwarding_enabled_) { \
    for (size_t i = 0; i < listeners_.size(); i++) { \
      listeners_[i]->Name(parameter); \
    } \
  } \
}
// This defines a member that forwards the call to all listeners in reverse
// order.
#define GTEST_REVERSE_REPEATER_METHOD_(Name, Type) \
void TestEventRepeater::Name(const Type& parameter) { \
  if (forwarding_enabled_) { \
    for (int i = static_cast<int>(listeners_.size()) - 1; i >= 0; i--) { \
      listeners_[i]->Name(parameter); \
    } \
  } \
}

GTEST_REPEATER_METHOD_(OnTestProgramStart, UnitTest)
GTEST_REPEATER_METHOD_(OnEnvironmentsSetUpStart, UnitTest)
GTEST_REPEATER_METHOD_(OnTestCaseStart, TestCase)
GTEST_REPEATER_METHOD_(OnTestStart, TestInfo)
GTEST_REPEATER_METHOD_(OnTestPartResult, TestPartResult)
GTEST_REPEATER_METHOD_(OnEnvironmentsTearDownStart, UnitTest)
GTEST_REVERSE_REPEATER_METHOD_(OnEnvironmentsSetUpEnd, UnitTest)
GTEST_REVERSE_REPEATER_METHOD_(OnEnvironmentsTearDownEnd, UnitTest)
GTEST_REVERSE_REPEATER_METHOD_(OnTestEnd, TestInfo)
GTEST_REVERSE_REPEATER_METHOD_(OnTestCaseEnd, TestCase)
GTEST_REVERSE_REPEATER_METHOD_(OnTestProgramEnd, UnitTest)

#undef GTEST_REPEATER_METHOD_
#undef GTEST_REVERSE_REPEATER_METHOD_

void TestEventRepeater::OnTestIterationStart(const UnitTest& unit_test,
                                             int iteration) {
  if (forwarding_enabled_) {
    for (size_t i = 0; i < listeners_.size(); i++) {
      listeners_[i]->OnTestIterationStart(unit_test, iteration);
    }
  }
}

void TestEventRepeater::OnTestIterationEnd(const UnitTest& unit_test,
                                           int iteration) {
  if (forwarding_enabled_) {
    for (int i = static_cast<int>(listeners_.size()) - 1; i >= 0; i--) {
      listeners_[i]->OnTestIterationEnd(unit_test, iteration);
    }
  }
}

// End TestEventRepeater

// This class generates an XML output file.
class XmlUnitTestResultPrinter : public EmptyTestEventListener {
 public:
  explicit XmlUnitTestResultPrinter(const char* output_file);

  virtual void OnTestIterationEnd(const UnitTest& unit_test, int iteration);

 private:
  // Is c a whitespace character that is normalized to a space character
  // when it appears in an XML attribute value?
  static bool IsNormalizableWhitespace(char c) {
    return c == 0x9 || c == 0xA || c == 0xD;
  }

  // May c appear in a well-formed XML document?
  static bool IsValidXmlCharacter(char c) {
    return IsNormalizableWhitespace(c) || c >= 0x20;
  }

  // Returns an XML-escaped copy of the input string str.  If
  // is_attribute is true, the text is meant to appear as an attribute
  // value, and normalizable whitespace is preserved by replacing it
  // with character references.
  static String EscapeXml(const char* str, bool is_attribute);

  // Returns the given string with all characters invalid in XML removed.
  static String RemoveInvalidXmlCharacters(const char* str);

  // Convenience wrapper around EscapeXml when str is an attribute value.
  static String EscapeXmlAttribute(const char* str) {
    return EscapeXml(str, true);
  }

  // Convenience wrapper around EscapeXml when str is not an attribute value.
  static String EscapeXmlText(const char* str) { return EscapeXml(str, false); }

  // Streams an XML CDATA section, escaping invalid CDATA sequences as needed.
  static void OutputXmlCDataSection(::std::ostream* stream, const char* data);

  // Streams an XML representation of a TestInfo object.
  static void OutputXmlTestInfo(::std::ostream* stream,
                                const char* test_case_name,
                                const TestInfo& test_info);

  // Prints an XML representation of a TestCase object
  static void PrintXmlTestCase(FILE* out, const TestCase& test_case);

  // Prints an XML summary of unit_test to output stream out.
  static void PrintXmlUnitTest(FILE* out, const UnitTest& unit_test);

  // Produces a string representing the test properties in a result as space
  // delimited XML attributes based on the property key="value" pairs.
  // When the String is not empty, it includes a space at the beginning,
  // to delimit this attribute from prior attributes.
  static String TestPropertiesAsXmlAttributes(const TestResult& result);

  // The output file.
  const String output_file_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(XmlUnitTestResultPrinter);
};

// Creates a new XmlUnitTestResultPrinter.
XmlUnitTestResultPrinter::XmlUnitTestResultPrinter(const char* output_file)
    : output_file_(output_file) {
  if (output_file_.c_str() == NULL || output_file_.empty()) {
    fprintf(stderr, "XML output file may not be null\n");
    fflush(stderr);
    exit(EXIT_FAILURE);
  }
}

// Called after the unit test ends.
void XmlUnitTestResultPrinter::OnTestIterationEnd(const UnitTest& unit_test,
                                                  int /*iteration*/) {
  FILE* xmlout = NULL;
  FilePath output_file(output_file_);
  FilePath output_dir(output_file.RemoveFileName());

  if (output_dir.CreateDirectoriesRecursively()) {
    xmlout = posix::FOpen(output_file_.c_str(), "w");
  }
  if (xmlout == NULL) {
    // TODO(wan): report the reason of the failure.
    //
    // We don't do it for now as:
    //
    //   1. There is no urgent need for it.
    //   2. It's a bit involved to make the errno variable thread-safe on
    //      all three operating systems (Linux, Windows, and Mac OS).
    //   3. To interpret the meaning of errno in a thread-safe way,
    //      we need the strerror_r() function, which is not available on
    //      Windows.
    fprintf(stderr,
            "Unable to open file \"%s\"\n",
            output_file_.c_str());
    fflush(stderr);
    exit(EXIT_FAILURE);
  }
  PrintXmlUnitTest(xmlout, unit_test);
  fclose(xmlout);
}

// Returns an XML-escaped copy of the input string str.  If is_attribute
// is true, the text is meant to appear as an attribute value, and
// normalizable whitespace is preserved by replacing it with character
// references.
//
// Invalid XML characters in str, if any, are stripped from the output.
// It is expected that most, if not all, of the text processed by this
// module will consist of ordinary English text.
// If this module is ever modified to produce version 1.1 XML output,
// most invalid characters can be retained using character references.
// TODO(wan): It might be nice to have a minimally invasive, human-readable
// escaping scheme for invalid characters, rather than dropping them.
String XmlUnitTestResultPrinter::EscapeXml(const char* str, bool is_attribute) {
  Message m;

  if (str != NULL) {
    for (const char* src = str; *src; ++src) {
      switch (*src) {
        case '<':
          m << "&lt;";
          break;
        case '>':
          m << "&gt;";
          break;
        case '&':
          m << "&amp;";
          break;
        case '\'':
          if (is_attribute)
            m << "&apos;";
          else
            m << '\'';
          break;
        case '"':
          if (is_attribute)
            m << "&quot;";
          else
            m << '"';
          break;
        default:
          if (IsValidXmlCharacter(*src)) {
            if (is_attribute && IsNormalizableWhitespace(*src))
              m << String::Format("&#x%02X;", unsigned(*src));
            else
              m << *src;
          }
          break;
      }
    }
  }

  return m.GetString();
}

// Returns the given string with all characters invalid in XML removed.
// Currently invalid characters are dropped from the string. An
// alternative is to replace them with certain characters such as . or ?.
String XmlUnitTestResultPrinter::RemoveInvalidXmlCharacters(const char* str) {
  char* const output = new char[strlen(str) + 1];
  char* appender = output;
  for (char ch = *str; ch != '\0'; ch = *++str)
    if (IsValidXmlCharacter(ch))
      *appender++ = ch;
  *appender = '\0';

  String ret_value(output);
  delete[] output;
  return ret_value;
}

// The following routines generate an XML representation of a UnitTest
// object.
//
// This is how Google Test concepts map to the DTD:
//
// <testsuites name="AllTests">        <-- corresponds to a UnitTest object
//   <testsuite name="testcase-name">  <-- corresponds to a TestCase object
//     <testcase name="test-name">     <-- corresponds to a TestInfo object
//       <failure message="...">...</failure>
//       <failure message="...">...</failure>
//       <failure message="...">...</failure>
//                                     <-- individual assertion failures
//     </testcase>
//   </testsuite>
// </testsuites>

// Formats the given time in milliseconds as seconds.
std::string FormatTimeInMillisAsSeconds(TimeInMillis ms) {
  ::std::stringstream ss;
  ss << ms/1000.0;
  return ss.str();
}

// Streams an XML CDATA section, escaping invalid CDATA sequences as needed.
void XmlUnitTestResultPrinter::OutputXmlCDataSection(::std::ostream* stream,
                                                     const char* data) {
  const char* segment = data;
  *stream << "<![CDATA[";
  for (;;) {
    const char* const next_segment = strstr(segment, "]]>");
    if (next_segment != NULL) {
      stream->write(
          segment, static_cast<std::streamsize>(next_segment - segment));
      *stream << "]]>]]&gt;<![CDATA[";
      segment = next_segment + strlen("]]>");
    } else {
      *stream << segment;
      break;
    }
  }
  *stream << "]]>";
}

// Prints an XML representation of a TestInfo object.
// TODO(wan): There is also value in printing properties with the plain printer.
void XmlUnitTestResultPrinter::OutputXmlTestInfo(::std::ostream* stream,
                                                 const char* test_case_name,
                                                 const TestInfo& test_info) {
  const TestResult& result = *test_info.result();
  *stream << "    <testcase name=\""
          << EscapeXmlAttribute(test_info.name()).c_str()
          << "\" status=\""
          << (test_info.should_run() ? "run" : "notrun")
          << "\" time=\""
          << FormatTimeInMillisAsSeconds(result.elapsed_time())
          << "\" classname=\"" << EscapeXmlAttribute(test_case_name).c_str()
          << "\"" << TestPropertiesAsXmlAttributes(result).c_str();

  int failures = 0;
  for (int i = 0; i < result.total_part_count(); ++i) {
    const TestPartResult& part = result.GetTestPartResult(i);
    if (part.failed()) {
      if (++failures == 1)
        *stream << ">\n";
      *stream << "      <failure message=\""
              << EscapeXmlAttribute(part.summary()).c_str()
              << "\" type=\"\">";
      const String message = RemoveInvalidXmlCharacters(String::Format(
          "%s:%d\n%s",
          part.file_name(), part.line_number(),
          part.message()).c_str());
      OutputXmlCDataSection(stream, message.c_str());
      *stream << "</failure>\n";
    }
  }

  if (failures == 0)
    *stream << " />\n";
  else
    *stream << "    </testcase>\n";
}

// Prints an XML representation of a TestCase object
void XmlUnitTestResultPrinter::PrintXmlTestCase(FILE* out,
                                                const TestCase& test_case) {
  fprintf(out,
          "  <testsuite name=\"%s\" tests=\"%d\" failures=\"%d\" "
          "disabled=\"%d\" ",
          EscapeXmlAttribute(test_case.name()).c_str(),
          test_case.total_test_count(),
          test_case.failed_test_count(),
          test_case.disabled_test_count());
  fprintf(out,
          "errors=\"0\" time=\"%s\">\n",
          FormatTimeInMillisAsSeconds(test_case.elapsed_time()).c_str());
  for (int i = 0; i < test_case.total_test_count(); ++i) {
    StrStream stream;
    OutputXmlTestInfo(&stream, test_case.name(), *test_case.GetTestInfo(i));
    fprintf(out, "%s", StrStreamToString(&stream).c_str());
  }
  fprintf(out, "  </testsuite>\n");
}

// Prints an XML summary of unit_test to output stream out.
void XmlUnitTestResultPrinter::PrintXmlUnitTest(FILE* out,
                                                const UnitTest& unit_test) {
  fprintf(out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf(out,
          "<testsuites tests=\"%d\" failures=\"%d\" disabled=\"%d\" "
          "errors=\"0\" time=\"%s\" ",
          unit_test.total_test_count(),
          unit_test.failed_test_count(),
          unit_test.disabled_test_count(),
          FormatTimeInMillisAsSeconds(unit_test.elapsed_time()).c_str());
  if (GTEST_FLAG(shuffle)) {
    fprintf(out, "random_seed=\"%d\" ", unit_test.random_seed());
  }
  fprintf(out, "name=\"AllTests\">\n");
  for (int i = 0; i < unit_test.total_test_case_count(); ++i)
    PrintXmlTestCase(out, *unit_test.GetTestCase(i));
  fprintf(out, "</testsuites>\n");
}

// Produces a string representing the test properties in a result as space
// delimited XML attributes based on the property key="value" pairs.
String XmlUnitTestResultPrinter::TestPropertiesAsXmlAttributes(
    const TestResult& result) {
  Message attributes;
  for (int i = 0; i < result.test_property_count(); ++i) {
    const TestProperty& property = result.GetTestProperty(i);
    attributes << " " << property.key() << "="
        << "\"" << EscapeXmlAttribute(property.value()) << "\"";
  }
  return attributes.GetString();
}

// End XmlUnitTestResultPrinter

// Class ScopedTrace

// Pushes the given source file location and message onto a per-thread
// trace stack maintained by Google Test.
// L < UnitTest::mutex_
ScopedTrace::ScopedTrace(const char* file, int line, const Message& message) {
  TraceInfo trace;
  trace.file = file;
  trace.line = line;
  trace.message = message.GetString();

  UnitTest::GetInstance()->PushGTestTrace(trace);
}

// Pops the info pushed by the c'tor.
// L < UnitTest::mutex_
ScopedTrace::~ScopedTrace() {
  UnitTest::GetInstance()->PopGTestTrace();
}


// class OsStackTraceGetter

// Returns the current OS stack trace as a String.  Parameters:
//
//   max_depth  - the maximum number of stack frames to be included
//                in the trace.
//   skip_count - the number of top frames to be skipped; doesn't count
//                against max_depth.
//
// L < mutex_
// We use "L < mutex_" to denote that the function may acquire mutex_.
String OsStackTraceGetter::CurrentStackTrace(int, int) {
  return String("");
}

// L < mutex_
void OsStackTraceGetter::UponLeavingGTest() {
}

const char* const
OsStackTraceGetter::kElidedFramesMarker =
    "... " GTEST_NAME_ " internal frames ...";

}  // namespace internal

// class TestEventListeners

TestEventListeners::TestEventListeners()
    : repeater_(new internal::TestEventRepeater()),
      default_result_printer_(NULL),
      default_xml_generator_(NULL) {
}

TestEventListeners::~TestEventListeners() { delete repeater_; }

// Returns the standard listener responsible for the default console
// output.  Can be removed from the listeners list to shut down default
// console output.  Note that removing this object from the listener list
// with Release transfers its ownership to the user.
void TestEventListeners::Append(TestEventListener* listener) {
  repeater_->Append(listener);
}

// Removes the given event listener from the list and returns it.  It then
// becomes the caller's responsibility to delete the listener. Returns
// NULL if the listener is not found in the list.
TestEventListener* TestEventListeners::Release(TestEventListener* listener) {
  if (listener == default_result_printer_)
    default_result_printer_ = NULL;
  else if (listener == default_xml_generator_)
    default_xml_generator_ = NULL;
  return repeater_->Release(listener);
}

// Returns repeater that broadcasts the TestEventListener events to all
// subscribers.
TestEventListener* TestEventListeners::repeater() { return repeater_; }

// Sets the default_result_printer attribute to the provided listener.
// The listener is also added to the listener list and previous
// default_result_printer is removed from it and deleted. The listener can
// also be NULL in which case it will not be added to the list. Does
// nothing if the previous and the current listener objects are the same.
void TestEventListeners::SetDefaultResultPrinter(TestEventListener* listener) {
  if (default_result_printer_ != listener) {
    // It is an error to pass this method a listener that is already in the
    // list.
    delete Release(default_result_printer_);
    default_result_printer_ = listener;
    if (listener != NULL)
      Append(listener);
  }
}

// Sets the default_xml_generator attribute to the provided listener.  The
// listener is also added to the listener list and previous
// default_xml_generator is removed from it and deleted. The listener can
// also be NULL in which case it will not be added to the list. Does
// nothing if the previous and the current listener objects are the same.
void TestEventListeners::SetDefaultXmlGenerator(TestEventListener* listener) {
  if (default_xml_generator_ != listener) {
    // It is an error to pass this method a listener that is already in the
    // list.
    delete Release(default_xml_generator_);
    default_xml_generator_ = listener;
    if (listener != NULL)
      Append(listener);
  }
}

// Controls whether events will be forwarded by the repeater to the
// listeners in the list.
bool TestEventListeners::EventForwardingEnabled() const {
  return repeater_->forwarding_enabled();
}

void TestEventListeners::SuppressEventForwarding() {
  repeater_->set_forwarding_enabled(false);
}

// class UnitTest

// Gets the singleton UnitTest object.  The first time this method is
// called, a UnitTest object is constructed and returned.  Consecutive
// calls will return the same object.
//
// We don't protect this under mutex_ as a user is not supposed to
// call this before main() starts, from which point on the return
// value will never change.
UnitTest * UnitTest::GetInstance() {
  // When compiled with MSVC 7.1 in optimized mode, destroying the
  // UnitTest object upon exiting the program messes up the exit code,
  // causing successful tests to appear failed.  We have to use a
  // different implementation in this case to bypass the compiler bug.
  // This implementation makes the compiler happy, at the cost of
  // leaking the UnitTest object.

  // CodeGear C++Builder insists on a public destructor for the
  // default implementation.  Use this implementation to keep good OO
  // design with private destructor.

#if (_MSC_VER == 1310 && !defined(_DEBUG)) || defined(__BORLANDC__)
  static UnitTest* const instance = new UnitTest;
  return instance;
#else
  static UnitTest instance;
  return &instance;
#endif  // (_MSC_VER == 1310 && !defined(_DEBUG)) || defined(__BORLANDC__)
}

// Gets the number of successful test cases.
int UnitTest::successful_test_case_count() const {
  return impl()->successful_test_case_count();
}

// Gets the number of failed test cases.
int UnitTest::failed_test_case_count() const {
  return impl()->failed_test_case_count();
}

// Gets the number of all test cases.
int UnitTest::total_test_case_count() const {
  return impl()->total_test_case_count();
}

// Gets the number of all test cases that contain at least one test
// that should run.
int UnitTest::test_case_to_run_count() const {
  return impl()->test_case_to_run_count();
}

// Gets the number of successful tests.
int UnitTest::successful_test_count() const {
  return impl()->successful_test_count();
}

// Gets the number of failed tests.
int UnitTest::failed_test_count() const { return impl()->failed_test_count(); }

// Gets the number of disabled tests.
int UnitTest::disabled_test_count() const {
  return impl()->disabled_test_count();
}

// Gets the number of all tests.
int UnitTest::total_test_count() const { return impl()->total_test_count(); }

// Gets the number of tests that should run.
int UnitTest::test_to_run_count() const { return impl()->test_to_run_count(); }

// Gets the elapsed time, in milliseconds.
internal::TimeInMillis UnitTest::elapsed_time() const {
  return impl()->elapsed_time();
}

// Returns true iff the unit test passed (i.e. all test cases passed).
bool UnitTest::Passed() const { return impl()->Passed(); }

// Returns true iff the unit test failed (i.e. some test case failed
// or something outside of all tests failed).
bool UnitTest::Failed() const { return impl()->Failed(); }

// Gets the i-th test case among all the test cases. i can range from 0 to
// total_test_case_count() - 1. If i is not in that range, returns NULL.
const TestCase* UnitTest::GetTestCase(int i) const {
  return impl()->GetTestCase(i);
}

// Gets the i-th test case among all the test cases. i can range from 0 to
// total_test_case_count() - 1. If i is not in that range, returns NULL.
TestCase* UnitTest::GetMutableTestCase(int i) {
  return impl()->GetMutableTestCase(i);
}

// Returns the list of event listeners that can be used to track events
// inside Google Test.
TestEventListeners& UnitTest::listeners() {
  return *impl()->listeners();
}

// Registers and returns a global test environment.  When a test
// program is run, all global test environments will be set-up in the
// order they were registered.  After all tests in the program have
// finished, all global test environments will be torn-down in the
// *reverse* order they were registered.
//
// The UnitTest object takes ownership of the given environment.
//
// We don't protect this under mutex_, as we only support calling it
// from the main thread.
Environment* UnitTest::AddEnvironment(Environment* env) {
  if (env == NULL) {
    return NULL;
  }

  impl_->environments().push_back(env);
  return env;
}

#if GTEST_HAS_EXCEPTIONS
// A failed Google Test assertion will throw an exception of this type
// when exceptions are enabled.  We derive it from std::runtime_error,
// which is for errors presumably detectable only at run time.  Since
// std::runtime_error inherits from std::exception, many testing
// frameworks know how to extract and print the message inside it.
class GoogleTestFailureException : public ::std::runtime_error {
 public:
  explicit GoogleTestFailureException(const TestPartResult& failure)
      : ::std::runtime_error(PrintTestPartResultToString(failure).c_str()) {}
};
#endif

// Adds a TestPartResult to the current TestResult object.  All Google Test
// assertion macros (e.g. ASSERT_TRUE, EXPECT_EQ, etc) eventually call
// this to report their results.  The user code should use the
// assertion macros instead of calling this directly.
// L < mutex_
void UnitTest::AddTestPartResult(TestPartResult::Type result_type,
                                 const char* file_name,
                                 int line_number,
                                 const internal::String& message,
                                 const internal::String& os_stack_trace) {
  Message msg;
  msg << message;

  internal::MutexLock lock(&mutex_);
  if (impl_->gtest_trace_stack().size() > 0) {
    msg << "\n" << GTEST_NAME_ << " trace:";

    for (int i = static_cast<int>(impl_->gtest_trace_stack().size());
         i > 0; --i) {
      const internal::TraceInfo& trace = impl_->gtest_trace_stack()[i - 1];
      msg << "\n" << internal::FormatFileLocation(trace.file, trace.line)
          << " " << trace.message;
    }
  }

  if (os_stack_trace.c_str() != NULL && !os_stack_trace.empty()) {
    msg << internal::kStackTraceMarker << os_stack_trace;
  }

  const TestPartResult result =
    TestPartResult(result_type, file_name, line_number,
                   msg.GetString().c_str());
  impl_->GetTestPartResultReporterForCurrentThread()->
      ReportTestPartResult(result);

  if (result_type != TestPartResult::kSuccess) {
    // gtest_break_on_failure takes precedence over
    // gtest_throw_on_failure.  This allows a user to set the latter
    // in the code (perhaps in order to use Google Test assertions
    // with another testing framework) and specify the former on the
    // command line for debugging.
    if (GTEST_FLAG(break_on_failure)) {
#if GTEST_OS_WINDOWS
      // Using DebugBreak on Windows allows gtest to still break into a debugger
      // when a failure happens and both the --gtest_break_on_failure and
      // the --gtest_catch_exceptions flags are specified.
      DebugBreak();
#else
      abort();
#endif  // GTEST_OS_WINDOWS
    } else if (GTEST_FLAG(throw_on_failure)) {
#if GTEST_HAS_EXCEPTIONS
      throw GoogleTestFailureException(result);
#else
      // We cannot call abort() as it generates a pop-up in debug mode
      // that cannot be suppressed in VC 7.1 or below.
      exit(1);
#endif
    }
  }
}

// Creates and adds a property to the current TestResult. If a property matching
// the supplied value already exists, updates its value instead.
void UnitTest::RecordPropertyForCurrentTest(const char* key,
                                            const char* value) {
  const TestProperty test_property(key, value);
  impl_->current_test_result()->RecordProperty(test_property);
}

// Runs all tests in this UnitTest object and prints the result.
// Returns 0 if successful, or 1 otherwise.
//
// We don't protect this under mutex_, as we only support calling it
// from the main thread.
int UnitTest::Run() {
#if GTEST_HAS_SEH
  // Catch SEH-style exceptions.

  const bool in_death_test_child_process =
      internal::GTEST_FLAG(internal_run_death_test).length() > 0;

  // Either the user wants Google Test to catch exceptions thrown by the
  // tests or this is executing in the context of death test child
  // process. In either case the user does not want to see pop-up dialogs
  // about crashes - they are expected..
  if (GTEST_FLAG(catch_exceptions) || in_death_test_child_process) {
#if !GTEST_OS_WINDOWS_MOBILE
    // SetErrorMode doesn't exist on CE.
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOALIGNMENTFAULTEXCEPT |
                 SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
#endif  // !GTEST_OS_WINDOWS_MOBILE

#if (defined(_MSC_VER) || GTEST_OS_WINDOWS_MINGW) && !GTEST_OS_WINDOWS_MOBILE
    // Death test children can be terminated with _abort().  On Windows,
    // _abort() can show a dialog with a warning message.  This forces the
    // abort message to go to stderr instead.
    _set_error_mode(_OUT_TO_STDERR);
#endif

#if _MSC_VER >= 1400 && !GTEST_OS_WINDOWS_MOBILE
    // In the debug version, Visual Studio pops up a separate dialog
    // offering a choice to debug the aborted program. We need to suppress
    // this dialog or it will pop up for every EXPECT/ASSERT_DEATH statement
    // executed. Google Test will notify the user of any unexpected
    // failure via stderr.
    //
    // VC++ doesn't define _set_abort_behavior() prior to the version 8.0.
    // Users of prior VC versions shall suffer the agony and pain of
    // clicking through the countless debug dialogs.
    // TODO(vladl@google.com): find a way to suppress the abort dialog() in the
    // debug mode when compiled with VC 7.1 or lower.
    if (!GTEST_FLAG(break_on_failure))
      _set_abort_behavior(
          0x0,                                    // Clear the following flags:
          _WRITE_ABORT_MSG | _CALL_REPORTFAULT);  // pop-up window, core dump.
#endif
  }

  __try {
    return impl_->RunAllTests();
  } __except(internal::UnitTestOptions::GTestShouldProcessSEH(
      GetExceptionCode())) {
    printf("Exception thrown with code 0x%x.\nFAIL\n", GetExceptionCode());
    fflush(stdout);
    return 1;
  }

#else  // We are on a compiler or platform that doesn't support SEH.

  return impl_->RunAllTests();
#endif  // GTEST_HAS_SEH
}

// Returns the working directory when the first TEST() or TEST_F() was
// executed.
const char* UnitTest::original_working_dir() const {
  return impl_->original_working_dir_.c_str();
}

// Returns the TestCase object for the test that's currently running,
// or NULL if no test is running.
// L < mutex_
const TestCase* UnitTest::current_test_case() const {
  internal::MutexLock lock(&mutex_);
  return impl_->current_test_case();
}

// Returns the TestInfo object for the test that's currently running,
// or NULL if no test is running.
// L < mutex_
const TestInfo* UnitTest::current_test_info() const {
  internal::MutexLock lock(&mutex_);
  return impl_->current_test_info();
}

// Returns the random seed used at the start of the current test run.
int UnitTest::random_seed() const { return impl_->random_seed(); }

#if GTEST_HAS_PARAM_TEST
// Returns ParameterizedTestCaseRegistry object used to keep track of
// value-parameterized tests and instantiate and register them.
// L < mutex_
internal::ParameterizedTestCaseRegistry&
    UnitTest::parameterized_test_registry() {
  return impl_->parameterized_test_registry();
}
#endif  // GTEST_HAS_PARAM_TEST

// Creates an empty UnitTest.
UnitTest::UnitTest() {
  impl_ = new internal::UnitTestImpl(this);
}

// Destructor of UnitTest.
UnitTest::~UnitTest() {
  delete impl_;
}

// Pushes a trace defined by SCOPED_TRACE() on to the per-thread
// Google Test trace stack.
// L < mutex_
void UnitTest::PushGTestTrace(const internal::TraceInfo& trace) {
  internal::MutexLock lock(&mutex_);
  impl_->gtest_trace_stack().push_back(trace);
}

// Pops a trace from the per-thread Google Test trace stack.
// L < mutex_
void UnitTest::PopGTestTrace() {
  internal::MutexLock lock(&mutex_);
  impl_->gtest_trace_stack().pop_back();
}

namespace internal {

UnitTestImpl::UnitTestImpl(UnitTest* parent)
    : parent_(parent),
#ifdef _MSC_VER
#pragma warning(push)                    // Saves the current warning state.
#pragma warning(disable:4355)            // Temporarily disables warning 4355
                                         // (using this in initializer).
      default_global_test_part_result_reporter_(this),
      default_per_thread_test_part_result_reporter_(this),
#pragma warning(pop)                     // Restores the warning state again.
#else
      default_global_test_part_result_reporter_(this),
      default_per_thread_test_part_result_reporter_(this),
#endif  // _MSC_VER
      global_test_part_result_repoter_(
          &default_global_test_part_result_reporter_),
      per_thread_test_part_result_reporter_(
          &default_per_thread_test_part_result_reporter_),
#if GTEST_HAS_PARAM_TEST
      parameterized_test_registry_(),
      parameterized_tests_registered_(false),
#endif  // GTEST_HAS_PARAM_TEST
      last_death_test_case_(-1),
      current_test_case_(NULL),
      current_test_info_(NULL),
      ad_hoc_test_result_(),
      os_stack_trace_getter_(NULL),
      post_flag_parse_init_performed_(false),
      random_seed_(0),  // Will be overridden by the flag before first use.
      random_(0),  // Will be reseeded before first use.
#if GTEST_HAS_DEATH_TEST
      elapsed_time_(0),
      internal_run_death_test_flag_(NULL),
      death_test_factory_(new DefaultDeathTestFactory) {
#else
      elapsed_time_(0) {
#endif  // GTEST_HAS_DEATH_TEST
  listeners()->SetDefaultResultPrinter(new PrettyUnitTestResultPrinter);
}

UnitTestImpl::~UnitTestImpl() {
  // Deletes every TestCase.
  ForEach(test_cases_, internal::Delete<TestCase>);

  // Deletes every Environment.
  ForEach(environments_, internal::Delete<Environment>);

  delete os_stack_trace_getter_;
}

#if GTEST_HAS_DEATH_TEST
// Disables event forwarding if the control is currently in a death test
// subprocess. Must not be called before InitGoogleTest.
void UnitTestImpl::SuppressTestEventsIfInSubprocess() {
  if (internal_run_death_test_flag_.get() != NULL)
    listeners()->SuppressEventForwarding();
}
#endif  // GTEST_HAS_DEATH_TEST

// Initializes event listeners performing XML output as specified by
// UnitTestOptions. Must not be called before InitGoogleTest.
void UnitTestImpl::ConfigureXmlOutput() {
  const String& output_format = UnitTestOptions::GetOutputFormat();
  if (output_format == "xml") {
    listeners()->SetDefaultXmlGenerator(new XmlUnitTestResultPrinter(
        UnitTestOptions::GetAbsolutePathToOutputFile().c_str()));
  } else if (output_format != "") {
    printf("WARNING: unrecognized output format \"%s\" ignored.\n",
           output_format.c_str());
    fflush(stdout);
  }
}

// Performs initialization dependent upon flag values obtained in
// ParseGoogleTestFlagsOnly.  Is called from InitGoogleTest after the call to
// ParseGoogleTestFlagsOnly.  In case a user neglects to call InitGoogleTest
// this function is also called from RunAllTests.  Since this function can be
// called more than once, it has to be idempotent.
void UnitTestImpl::PostFlagParsingInit() {
  // Ensures that this function does not execute more than once.
  if (!post_flag_parse_init_performed_) {
    post_flag_parse_init_performed_ = true;

#if GTEST_HAS_DEATH_TEST
    InitDeathTestSubprocessControlInfo();
    SuppressTestEventsIfInSubprocess();
#endif  // GTEST_HAS_DEATH_TEST

    // Registers parameterized tests. This makes parameterized tests
    // available to the UnitTest reflection API without running
    // RUN_ALL_TESTS.
    RegisterParameterizedTests();

    // Configures listeners for XML output. This makes it possible for users
    // to shut down the default XML output before invoking RUN_ALL_TESTS.
    ConfigureXmlOutput();
  }
}

// A predicate that checks the name of a TestCase against a known
// value.
//
// This is used for implementation of the UnitTest class only.  We put
// it in the anonymous namespace to prevent polluting the outer
// namespace.
//
// TestCaseNameIs is copyable.
class TestCaseNameIs {
 public:
  // Constructor.
  explicit TestCaseNameIs(const String& name)
      : name_(name) {}

  // Returns true iff the name of test_case matches name_.
  bool operator()(const TestCase* test_case) const {
    return test_case != NULL && strcmp(test_case->name(), name_.c_str()) == 0;
  }

 private:
  String name_;
};

// Finds and returns a TestCase with the given name.  If one doesn't
// exist, creates one and returns it.  It's the CALLER'S
// RESPONSIBILITY to ensure that this function is only called WHEN THE
// TESTS ARE NOT SHUFFLED.
//
// Arguments:
//
//   test_case_name: name of the test case
//   set_up_tc:      pointer to the function that sets up the test case
//   tear_down_tc:   pointer to the function that tears down the test case
TestCase* UnitTestImpl::GetTestCase(const char* test_case_name,
                                    const char* comment,
                                    Test::SetUpTestCaseFunc set_up_tc,
                                    Test::TearDownTestCaseFunc tear_down_tc) {
  // Can we find a TestCase with the given name?
  const std::vector<TestCase*>::const_iterator test_case =
      std::find_if(test_cases_.begin(), test_cases_.end(),
                   TestCaseNameIs(test_case_name));

  if (test_case != test_cases_.end())
    return *test_case;

  // No.  Let's create one.
  TestCase* const new_test_case =
      new TestCase(test_case_name, comment, set_up_tc, tear_down_tc);

  // Is this a death test case?
  if (internal::UnitTestOptions::MatchesFilter(String(test_case_name),
                                               kDeathTestCaseFilter)) {
    // Yes.  Inserts the test case after the last death test case
    // defined so far.  This only works when the test cases haven't
    // been shuffled.  Otherwise we may end up running a death test
    // after a non-death test.
    ++last_death_test_case_;
    test_cases_.insert(test_cases_.begin() + last_death_test_case_,
                       new_test_case);
  } else {
    // No.  Appends to the end of the list.
    test_cases_.push_back(new_test_case);
  }

  test_case_indices_.push_back(static_cast<int>(test_case_indices_.size()));
  return new_test_case;
}

// Helpers for setting up / tearing down the given environment.  They
// are for use in the ForEach() function.
static void SetUpEnvironment(Environment* env) { env->SetUp(); }
static void TearDownEnvironment(Environment* env) { env->TearDown(); }

// Runs all tests in this UnitTest object, prints the result, and
// returns 0 if all tests are successful, or 1 otherwise.  If any
// exception is thrown during a test on Windows, this test is
// considered to be failed, but the rest of the tests will still be
// run.  (We disable exceptions on Linux and Mac OS X, so the issue
// doesn't apply there.)
// When parameterized tests are enabled, it expands and registers
// parameterized tests first in RegisterParameterizedTests().
// All other functions called from RunAllTests() may safely assume that
// parameterized tests are ready to be counted and run.
int UnitTestImpl::RunAllTests() {
  // Makes sure InitGoogleTest() was called.
  if (!GTestIsInitialized()) {
    printf("%s",
           "\nThis test program did NOT call ::testing::InitGoogleTest "
           "before calling RUN_ALL_TESTS().  Please fix it.\n");
    return 1;
  }

  // Do not run any test if the --help flag was specified.
  if (g_help_flag)
    return 0;

  // Repeats the call to the post-flag parsing initialization in case the
  // user didn't call InitGoogleTest.
  PostFlagParsingInit();

  // Even if sharding is not on, test runners may want to use the
  // GTEST_SHARD_STATUS_FILE to query whether the test supports the sharding
  // protocol.
  internal::WriteToShardStatusFileIfNeeded();

  // True iff we are in a subprocess for running a thread-safe-style
  // death test.
  bool in_subprocess_for_death_test = false;

#if GTEST_HAS_DEATH_TEST
  in_subprocess_for_death_test = (internal_run_death_test_flag_.get() != NULL);
#endif  // GTEST_HAS_DEATH_TEST

  const bool should_shard = ShouldShard(kTestTotalShards, kTestShardIndex,
                                        in_subprocess_for_death_test);

  // Compares the full test names with the filter to decide which
  // tests to run.
  const bool has_tests_to_run = FilterTests(should_shard
                                              ? HONOR_SHARDING_PROTOCOL
                                              : IGNORE_SHARDING_PROTOCOL) > 0;

  // Lists the tests and exits if the --gtest_list_tests flag was specified.
  if (GTEST_FLAG(list_tests)) {
    // This must be called *after* FilterTests() has been called.
    ListTestsMatchingFilter();
    return 0;
  }

  random_seed_ = GTEST_FLAG(shuffle) ?
      GetRandomSeedFromFlag(GTEST_FLAG(random_seed)) : 0;

  // True iff at least one test has failed.
  bool failed = false;

  TestEventListener* repeater = listeners()->repeater();

  repeater->OnTestProgramStart(*parent_);

  // How many times to repeat the tests?  We don't want to repeat them
  // when we are inside the subprocess of a death test.
  const int repeat = in_subprocess_for_death_test ? 1 : GTEST_FLAG(repeat);
  // Repeats forever if the repeat count is negative.
  const bool forever = repeat < 0;
  for (int i = 0; forever || i != repeat; i++) {
    ClearResult();

    const TimeInMillis start = GetTimeInMillis();

    // Shuffles test cases and tests if requested.
    if (has_tests_to_run && GTEST_FLAG(shuffle)) {
      random()->Reseed(random_seed_);
      // This should be done before calling OnTestIterationStart(),
      // such that a test event listener can see the actual test order
      // in the event.
      ShuffleTests();
    }

    // Tells the unit test event listeners that the tests are about to start.
    repeater->OnTestIterationStart(*parent_, i);

    // Runs each test case if there is at least one test to run.
    if (has_tests_to_run) {
      // Sets up all environments beforehand.
      repeater->OnEnvironmentsSetUpStart(*parent_);
      ForEach(environments_, SetUpEnvironment);
      repeater->OnEnvironmentsSetUpEnd(*parent_);

      // Runs the tests only if there was no fatal failure during global
      // set-up.
      if (!Test::HasFatalFailure()) {
        for (int test_index = 0; test_index < total_test_case_count();
             test_index++) {
          GetMutableTestCase(test_index)->Run();
        }
      }

      // Tears down all environments in reverse order afterwards.
      repeater->OnEnvironmentsTearDownStart(*parent_);
      std::for_each(environments_.rbegin(), environments_.rend(),
                    TearDownEnvironment);
      repeater->OnEnvironmentsTearDownEnd(*parent_);
    }

    elapsed_time_ = GetTimeInMillis() - start;

    // Tells the unit test event listener that the tests have just finished.
    repeater->OnTestIterationEnd(*parent_, i);

    // Gets the result and clears it.
    if (!Passed()) {
      failed = true;
    }

    // Restores the original test order after the iteration.  This
    // allows the user to quickly repro a failure that happens in the
    // N-th iteration without repeating the first (N - 1) iterations.
    // This is not enclosed in "if (GTEST_FLAG(shuffle)) { ... }", in
    // case the user somehow changes the value of the flag somewhere
    // (it's always safe to unshuffle the tests).
    UnshuffleTests();

    if (GTEST_FLAG(shuffle)) {
      // Picks a new random seed for each iteration.
      random_seed_ = GetNextRandomSeed(random_seed_);
    }
  }

  repeater->OnTestProgramEnd(*parent_);

  // Returns 0 if all tests passed, or 1 other wise.
  return failed ? 1 : 0;
}

// Reads the GTEST_SHARD_STATUS_FILE environment variable, and creates the file
// if the variable is present. If a file already exists at this location, this
// function will write over it. If the variable is present, but the file cannot
// be created, prints an error and exits.
void WriteToShardStatusFileIfNeeded() {
  const char* const test_shard_file = posix::GetEnv(kTestShardStatusFile);
  if (test_shard_file != NULL) {
    FILE* const file = posix::FOpen(test_shard_file, "w");
    if (file == NULL) {
      ColoredPrintf(COLOR_RED,
                    "Could not write to the test shard status file \"%s\" "
                    "specified by the %s environment variable.\n",
                    test_shard_file, kTestShardStatusFile);
      fflush(stdout);
      exit(EXIT_FAILURE);
    }
    fclose(file);
  }
}

// Checks whether sharding is enabled by examining the relevant
// environment variable values. If the variables are present,
// but inconsistent (i.e., shard_index >= total_shards), prints
// an error and exits. If in_subprocess_for_death_test, sharding is
// disabled because it must only be applied to the original test
// process. Otherwise, we could filter out death tests we intended to execute.
bool ShouldShard(const char* total_shards_env,
                 const char* shard_index_env,
                 bool in_subprocess_for_death_test) {
  if (in_subprocess_for_death_test) {
    return false;
  }

  const Int32 total_shards = Int32FromEnvOrDie(total_shards_env, -1);
  const Int32 shard_index = Int32FromEnvOrDie(shard_index_env, -1);

  if (total_shards == -1 && shard_index == -1) {
    return false;
  } else if (total_shards == -1 && shard_index != -1) {
    const Message msg = Message()
      << "Invalid environment variables: you have "
      << kTestShardIndex << " = " << shard_index
      << ", but have left " << kTestTotalShards << " unset.\n";
    ColoredPrintf(COLOR_RED, msg.GetString().c_str());
    fflush(stdout);
    exit(EXIT_FAILURE);
  } else if (total_shards != -1 && shard_index == -1) {
    const Message msg = Message()
      << "Invalid environment variables: you have "
      << kTestTotalShards << " = " << total_shards
      << ", but have left " << kTestShardIndex << " unset.\n";
    ColoredPrintf(COLOR_RED, msg.GetString().c_str());
    fflush(stdout);
    exit(EXIT_FAILURE);
  } else if (shard_index < 0 || shard_index >= total_shards) {
    const Message msg = Message()
      << "Invalid environment variables: we require 0 <= "
      << kTestShardIndex << " < " << kTestTotalShards
      << ", but you have " << kTestShardIndex << "=" << shard_index
      << ", " << kTestTotalShards << "=" << total_shards << ".\n";
    ColoredPrintf(COLOR_RED, msg.GetString().c_str());
    fflush(stdout);
    exit(EXIT_FAILURE);
  }

  return total_shards > 1;
}

// Parses the environment variable var as an Int32. If it is unset,
// returns default_val. If it is not an Int32, prints an error
// and aborts.
Int32 Int32FromEnvOrDie(const char* const var, Int32 default_val) {
  const char* str_val = posix::GetEnv(var);
  if (str_val == NULL) {
    return default_val;
  }

  Int32 result;
  if (!ParseInt32(Message() << "The value of environment variable " << var,
                  str_val, &result)) {
    exit(EXIT_FAILURE);
  }
  return result;
}

// Given the total number of shards, the shard index, and the test id,
// returns true iff the test should be run on this shard. The test id is
// some arbitrary but unique non-negative integer assigned to each test
// method. Assumes that 0 <= shard_index < total_shards.
bool ShouldRunTestOnShard(int total_shards, int shard_index, int test_id) {
  return (test_id % total_shards) == shard_index;
}

// Compares the name of each test with the user-specified filter to
// decide whether the test should be run, then records the result in
// each TestCase and TestInfo object.
// If shard_tests == true, further filters tests based on sharding
// variables in the environment - see
// http://code.google.com/p/googletest/wiki/GoogleTestAdvancedGuide.
// Returns the number of tests that should run.
int UnitTestImpl::FilterTests(ReactionToSharding shard_tests) {
  const Int32 total_shards = shard_tests == HONOR_SHARDING_PROTOCOL ?
      Int32FromEnvOrDie(kTestTotalShards, -1) : -1;
  const Int32 shard_index = shard_tests == HONOR_SHARDING_PROTOCOL ?
      Int32FromEnvOrDie(kTestShardIndex, -1) : -1;

  // num_runnable_tests are the number of tests that will
  // run across all shards (i.e., match filter and are not disabled).
  // num_selected_tests are the number of tests to be run on
  // this shard.
  int num_runnable_tests = 0;
  int num_selected_tests = 0;
  for (size_t i = 0; i < test_cases_.size(); i++) {
    TestCase* const test_case = test_cases_[i];
    const String &test_case_name = test_case->name();
    test_case->set_should_run(false);

    for (size_t j = 0; j < test_case->test_info_list().size(); j++) {
      TestInfo* const test_info = test_case->test_info_list()[j];
      const String test_name(test_info->name());
      // A test is disabled if test case name or test name matches
      // kDisableTestFilter.
      const bool is_disabled =
          internal::UnitTestOptions::MatchesFilter(test_case_name,
                                                   kDisableTestFilter) ||
          internal::UnitTestOptions::MatchesFilter(test_name,
                                                   kDisableTestFilter);
      test_info->impl()->set_is_disabled(is_disabled);

      const bool matches_filter =
          internal::UnitTestOptions::FilterMatchesTest(test_case_name,
                                                       test_name);
      test_info->impl()->set_matches_filter(matches_filter);

      const bool is_runnable =
          (GTEST_FLAG(also_run_disabled_tests) || !is_disabled) &&
          matches_filter;

      const bool is_selected = is_runnable &&
          (shard_tests == IGNORE_SHARDING_PROTOCOL ||
           ShouldRunTestOnShard(total_shards, shard_index,
                                num_runnable_tests));

      num_runnable_tests += is_runnable;
      num_selected_tests += is_selected;

      test_info->impl()->set_should_run(is_selected);
      test_case->set_should_run(test_case->should_run() || is_selected);
    }
  }
  return num_selected_tests;
}

// Prints the names of the tests matching the user-specified filter flag.
void UnitTestImpl::ListTestsMatchingFilter() {
  for (size_t i = 0; i < test_cases_.size(); i++) {
    const TestCase* const test_case = test_cases_[i];
    bool printed_test_case_name = false;

    for (size_t j = 0; j < test_case->test_info_list().size(); j++) {
      const TestInfo* const test_info =
          test_case->test_info_list()[j];
      if (test_info->matches_filter()) {
        if (!printed_test_case_name) {
          printed_test_case_name = true;
          printf("%s.\n", test_case->name());
        }
        printf("  %s\n", test_info->name());
      }
    }
  }
  fflush(stdout);
}

// Sets the OS stack trace getter.
//
// Does nothing if the input and the current OS stack trace getter are
// the same; otherwise, deletes the old getter and makes the input the
// current getter.
void UnitTestImpl::set_os_stack_trace_getter(
    OsStackTraceGetterInterface* getter) {
  if (os_stack_trace_getter_ != getter) {
    delete os_stack_trace_getter_;
    os_stack_trace_getter_ = getter;
  }
}

// Returns the current OS stack trace getter if it is not NULL;
// otherwise, creates an OsStackTraceGetter, makes it the current
// getter, and returns it.
OsStackTraceGetterInterface* UnitTestImpl::os_stack_trace_getter() {
  if (os_stack_trace_getter_ == NULL) {
    os_stack_trace_getter_ = new OsStackTraceGetter;
  }

  return os_stack_trace_getter_;
}

// Returns the TestResult for the test that's currently running, or
// the TestResult for the ad hoc test if no test is running.
TestResult* UnitTestImpl::current_test_result() {
  return current_test_info_ ?
    current_test_info_->impl()->result() : &ad_hoc_test_result_;
}

// Shuffles all test cases, and the tests within each test case,
// making sure that death tests are still run first.
void UnitTestImpl::ShuffleTests() {
  // Shuffles the death test cases.
  ShuffleRange(random(), 0, last_death_test_case_ + 1, &test_case_indices_);

  // Shuffles the non-death test cases.
  ShuffleRange(random(), last_death_test_case_ + 1,
               static_cast<int>(test_cases_.size()), &test_case_indices_);

  // Shuffles the tests inside each test case.
  for (size_t i = 0; i < test_cases_.size(); i++) {
    test_cases_[i]->ShuffleTests(random());
  }
}

// Restores the test cases and tests to their order before the first shuffle.
void UnitTestImpl::UnshuffleTests() {
  for (size_t i = 0; i < test_cases_.size(); i++) {
    // Unshuffles the tests in each test case.
    test_cases_[i]->UnshuffleTests();
    // Resets the index of each test case.
    test_case_indices_[i] = static_cast<int>(i);
  }
}

// TestInfoImpl constructor. The new instance assumes ownership of the test
// factory object.
TestInfoImpl::TestInfoImpl(TestInfo* parent,
                           const char* a_test_case_name,
                           const char* a_name,
                           const char* a_test_case_comment,
                           const char* a_comment,
                           TypeId a_fixture_class_id,
                           internal::TestFactoryBase* factory) :
    parent_(parent),
    test_case_name_(String(a_test_case_name)),
    name_(String(a_name)),
    test_case_comment_(String(a_test_case_comment)),
    comment_(String(a_comment)),
    fixture_class_id_(a_fixture_class_id),
    should_run_(false),
    is_disabled_(false),
    matches_filter_(false),
    factory_(factory) {
}

// TestInfoImpl destructor.
TestInfoImpl::~TestInfoImpl() {
  delete factory_;
}

// Returns the current OS stack trace as a String.
//
// The maximum number of stack frames to be included is specified by
// the gtest_stack_trace_depth flag.  The skip_count parameter
// specifies the number of top frames to be skipped, which doesn't
// count against the number of frames to be included.
//
// For example, if Foo() calls Bar(), which in turn calls
// GetCurrentOsStackTraceExceptTop(..., 1), Foo() will be included in
// the trace but Bar() and GetCurrentOsStackTraceExceptTop() won't.
String GetCurrentOsStackTraceExceptTop(UnitTest* /*unit_test*/,
                                       int skip_count) {
  // We pass skip_count + 1 to skip this wrapper function in addition
  // to what the user really wants to skip.
  return GetUnitTestImpl()->CurrentOsStackTraceExceptTop(skip_count + 1);
}

// Used by the GTEST_HIDE_UNREACHABLE_CODE_ macro to suppress unreachable
// code warnings.
namespace {
class ClassUniqueToAlwaysTrue {};
}

bool IsTrue(bool condition) { return condition; }

bool AlwaysTrue() {
#if GTEST_HAS_EXCEPTIONS
  // This condition is always false so AlwaysTrue() never actually throws,
  // but it makes the compiler think that it may throw.
  if (IsTrue(false))
    throw ClassUniqueToAlwaysTrue();
#endif  // GTEST_HAS_EXCEPTIONS
  return true;
}

// If *pstr starts with the given prefix, modifies *pstr to be right
// past the prefix and returns true; otherwise leaves *pstr unchanged
// and returns false.  None of pstr, *pstr, and prefix can be NULL.
bool SkipPrefix(const char* prefix, const char** pstr) {
  const size_t prefix_len = strlen(prefix);
  if (strncmp(*pstr, prefix, prefix_len) == 0) {
    *pstr += prefix_len;
    return true;
  }
  return false;
}

// Parses a string as a command line flag.  The string should have
// the format "--flag=value".  When def_optional is true, the "=value"
// part can be omitted.
//
// Returns the value of the flag, or NULL if the parsing failed.
const char* ParseFlagValue(const char* str,
                           const char* flag,
                           bool def_optional) {
  // str and flag must not be NULL.
  if (str == NULL || flag == NULL) return NULL;

  // The flag must start with "--" followed by GTEST_FLAG_PREFIX_.
  const String flag_str = String::Format("--%s%s", GTEST_FLAG_PREFIX_, flag);
  const size_t flag_len = flag_str.length();
  if (strncmp(str, flag_str.c_str(), flag_len) != 0) return NULL;

  // Skips the flag name.
  const char* flag_end = str + flag_len;

  // When def_optional is true, it's OK to not have a "=value" part.
  if (def_optional && (flag_end[0] == '\0')) {
    return flag_end;
  }

  // If def_optional is true and there are more characters after the
  // flag name, or if def_optional is false, there must be a '=' after
  // the flag name.
  if (flag_end[0] != '=') return NULL;

  // Returns the string after "=".
  return flag_end + 1;
}

// Parses a string for a bool flag, in the form of either
// "--flag=value" or "--flag".
//
// In the former case, the value is taken as true as long as it does
// not start with '0', 'f', or 'F'.
//
// In the latter case, the value is taken as true.
//
// On success, stores the value of the flag in *value, and returns
// true.  On failure, returns false without changing *value.
bool ParseBoolFlag(const char* str, const char* flag, bool* value) {
  // Gets the value of the flag as a string.
  const char* const value_str = ParseFlagValue(str, flag, true);

  // Aborts if the parsing failed.
  if (value_str == NULL) return false;

  // Converts the string value to a bool.
  *value = !(*value_str == '0' || *value_str == 'f' || *value_str == 'F');
  return true;
}

// Parses a string for an Int32 flag, in the form of
// "--flag=value".
//
// On success, stores the value of the flag in *value, and returns
// true.  On failure, returns false without changing *value.
bool ParseInt32Flag(const char* str, const char* flag, Int32* value) {
  // Gets the value of the flag as a string.
  const char* const value_str = ParseFlagValue(str, flag, false);

  // Aborts if the parsing failed.
  if (value_str == NULL) return false;

  // Sets *value to the value of the flag.
  return ParseInt32(Message() << "The value of flag --" << flag,
                    value_str, value);
}

// Parses a string for a string flag, in the form of
// "--flag=value".
//
// On success, stores the value of the flag in *value, and returns
// true.  On failure, returns false without changing *value.
bool ParseStringFlag(const char* str, const char* flag, String* value) {
  // Gets the value of the flag as a string.
  const char* const value_str = ParseFlagValue(str, flag, false);

  // Aborts if the parsing failed.
  if (value_str == NULL) return false;

  // Sets *value to the value of the flag.
  *value = value_str;
  return true;
}

// Determines whether a string has a prefix that Google Test uses for its
// flags, i.e., starts with GTEST_FLAG_PREFIX_ or GTEST_FLAG_PREFIX_DASH_.
// If Google Test detects that a command line flag has its prefix but is not
// recognized, it will print its help message. Flags starting with
// GTEST_INTERNAL_PREFIX_ followed by "internal_" are considered Google Test
// internal flags and do not trigger the help message.
static bool HasGoogleTestFlagPrefix(const char* str) {
  return (SkipPrefix("--", &str) ||
          SkipPrefix("-", &str) ||
          SkipPrefix("/", &str)) &&
         !SkipPrefix(GTEST_FLAG_PREFIX_ "internal_", &str) &&
         (SkipPrefix(GTEST_FLAG_PREFIX_, &str) ||
          SkipPrefix(GTEST_FLAG_PREFIX_DASH_, &str));
}

// Prints a string containing code-encoded text.  The following escape
// sequences can be used in the string to control the text color:
//
//   @@    prints a single '@' character.
//   @R    changes the color to red.
//   @G    changes the color to green.
//   @Y    changes the color to yellow.
//   @D    changes to the default terminal text color.
//
// TODO(wan@google.com): Write tests for this once we add stdout
// capturing to Google Test.
static void PrintColorEncoded(const char* str) {
  GTestColor color = COLOR_DEFAULT;  // The current color.

  // Conceptually, we split the string into segments divided by escape
  // sequences.  Then we print one segment at a time.  At the end of
  // each iteration, the str pointer advances to the beginning of the
  // next segment.
  for (;;) {
    const char* p = strchr(str, '@');
    if (p == NULL) {
      ColoredPrintf(color, "%s", str);
      return;
    }

    ColoredPrintf(color, "%s", String(str, p - str).c_str());

    const char ch = p[1];
    str = p + 2;
    if (ch == '@') {
      ColoredPrintf(color, "@");
    } else if (ch == 'D') {
      color = COLOR_DEFAULT;
    } else if (ch == 'R') {
      color = COLOR_RED;
    } else if (ch == 'G') {
      color = COLOR_GREEN;
    } else if (ch == 'Y') {
      color = COLOR_YELLOW;
    } else {
      --str;
    }
  }
}

static const char kColorEncodedHelpMessage[] =
"This program contains tests written using " GTEST_NAME_ ". You can use the\n"
"following command line flags to control its behavior:\n"
"\n"
"Test Selection:\n"
"  @G--" GTEST_FLAG_PREFIX_ "list_tests@D\n"
"      List the names of all tests instead of running them. The name of\n"
"      TEST(Foo, Bar) is \"Foo.Bar\".\n"
"  @G--" GTEST_FLAG_PREFIX_ "filter=@YPOSTIVE_PATTERNS"
    "[@G-@YNEGATIVE_PATTERNS]@D\n"
"      Run only the tests whose name matches one of the positive patterns but\n"
"      none of the negative patterns. '?' matches any single character; '*'\n"
"      matches any substring; ':' separates two patterns.\n"
"  @G--" GTEST_FLAG_PREFIX_ "also_run_disabled_tests@D\n"
"      Run all disabled tests too.\n"
"\n"
"Test Execution:\n"
"  @G--" GTEST_FLAG_PREFIX_ "repeat=@Y[COUNT]@D\n"
"      Run the tests repeatedly; use a negative count to repeat forever.\n"
"  @G--" GTEST_FLAG_PREFIX_ "shuffle@D\n"
"      Randomize tests' orders on every iteration.\n"
"  @G--" GTEST_FLAG_PREFIX_ "random_seed=@Y[NUMBER]@D\n"
"      Random number seed to use for shuffling test orders (between 1 and\n"
"      99999, or 0 to use a seed based on the current time).\n"
"\n"
"Test Output:\n"
"  @G--" GTEST_FLAG_PREFIX_ "color=@Y(@Gyes@Y|@Gno@Y|@Gauto@Y)@D\n"
"      Enable/disable colored output. The default is @Gauto@D.\n"
"  -@G-" GTEST_FLAG_PREFIX_ "print_time=0@D\n"
"      Don't print the elapsed time of each test.\n"
"  @G--" GTEST_FLAG_PREFIX_ "output=xml@Y[@G:@YDIRECTORY_PATH@G"
    GTEST_PATH_SEP_ "@Y|@G:@YFILE_PATH]@D\n"
"      Generate an XML report in the given directory or with the given file\n"
"      name. @YFILE_PATH@D defaults to @Gtest_details.xml@D.\n"
"\n"
"Assertion Behavior:\n"
#if GTEST_HAS_DEATH_TEST && !GTEST_OS_WINDOWS
"  @G--" GTEST_FLAG_PREFIX_ "death_test_style=@Y(@Gfast@Y|@Gthreadsafe@Y)@D\n"
"      Set the default death test style.\n"
#endif  // GTEST_HAS_DEATH_TEST && !GTEST_OS_WINDOWS
"  @G--" GTEST_FLAG_PREFIX_ "break_on_failure@D\n"
"      Turn assertion failures into debugger break-points.\n"
"  @G--" GTEST_FLAG_PREFIX_ "throw_on_failure@D\n"
"      Turn assertion failures into C++ exceptions.\n"
#if GTEST_OS_WINDOWS
"  @G--" GTEST_FLAG_PREFIX_ "catch_exceptions@D\n"
"      Suppress pop-ups caused by exceptions.\n"
#endif  // GTEST_OS_WINDOWS
"\n"
"Except for @G--" GTEST_FLAG_PREFIX_ "list_tests@D, you can alternatively set "
    "the corresponding\n"
"environment variable of a flag (all letters in upper-case). For example, to\n"
"disable colored text output, you can either specify @G--" GTEST_FLAG_PREFIX_
    "color=no@D or set\n"
"the @G" GTEST_FLAG_PREFIX_UPPER_ "COLOR@D environment variable to @Gno@D.\n"
"\n"
"For more information, please read the " GTEST_NAME_ " documentation at\n"
"@G" GTEST_PROJECT_URL_ "@D. If you find a bug in " GTEST_NAME_ "\n"
"(not one in your own code or tests), please report it to\n"
"@G<" GTEST_DEV_EMAIL_ ">@D.\n";

// Parses the command line for Google Test flags, without initializing
// other parts of Google Test.  The type parameter CharType can be
// instantiated to either char or wchar_t.
template <typename CharType>
void ParseGoogleTestFlagsOnlyImpl(int* argc, CharType** argv) {
  for (int i = 1; i < *argc; i++) {
    const String arg_string = StreamableToString(argv[i]);
    const char* const arg = arg_string.c_str();

    using internal::ParseBoolFlag;
    using internal::ParseInt32Flag;
    using internal::ParseStringFlag;

    // Do we see a Google Test flag?
    if (ParseBoolFlag(arg, kAlsoRunDisabledTestsFlag,
                      &GTEST_FLAG(also_run_disabled_tests)) ||
        ParseBoolFlag(arg, kBreakOnFailureFlag,
                      &GTEST_FLAG(break_on_failure)) ||
        ParseBoolFlag(arg, kCatchExceptionsFlag,
                      &GTEST_FLAG(catch_exceptions)) ||
        ParseStringFlag(arg, kColorFlag, &GTEST_FLAG(color)) ||
        ParseStringFlag(arg, kDeathTestStyleFlag,
                        &GTEST_FLAG(death_test_style)) ||
        ParseBoolFlag(arg, kDeathTestUseFork,
                      &GTEST_FLAG(death_test_use_fork)) ||
        ParseStringFlag(arg, kFilterFlag, &GTEST_FLAG(filter)) ||
        ParseStringFlag(arg, kInternalRunDeathTestFlag,
                        &GTEST_FLAG(internal_run_death_test)) ||
        ParseBoolFlag(arg, kListTestsFlag, &GTEST_FLAG(list_tests)) ||
        ParseStringFlag(arg, kOutputFlag, &GTEST_FLAG(output)) ||
        ParseBoolFlag(arg, kPrintTimeFlag, &GTEST_FLAG(print_time)) ||
        ParseInt32Flag(arg, kRandomSeedFlag, &GTEST_FLAG(random_seed)) ||
        ParseInt32Flag(arg, kRepeatFlag, &GTEST_FLAG(repeat)) ||
        ParseBoolFlag(arg, kShuffleFlag, &GTEST_FLAG(shuffle)) ||
        ParseInt32Flag(arg, kStackTraceDepthFlag,
                       &GTEST_FLAG(stack_trace_depth)) ||
        ParseBoolFlag(arg, kThrowOnFailureFlag, &GTEST_FLAG(throw_on_failure))
        ) {
      // Yes.  Shift the remainder of the argv list left by one.  Note
      // that argv has (*argc + 1) elements, the last one always being
      // NULL.  The following loop moves the trailing NULL element as
      // well.
      for (int j = i; j != *argc; j++) {
        argv[j] = argv[j + 1];
      }

      // Decrements the argument count.
      (*argc)--;

      // We also need to decrement the iterator as we just removed
      // an element.
      i--;
    } else if (arg_string == "--help" || arg_string == "-h" ||
               arg_string == "-?" || arg_string == "/?" ||
               HasGoogleTestFlagPrefix(arg)) {
      // Both help flag and unrecognized Google Test flags (excluding
      // internal ones) trigger help display.
      g_help_flag = true;
    }
  }

  if (g_help_flag) {
    // We print the help here instead of in RUN_ALL_TESTS(), as the
    // latter may not be called at all if the user is using Google
    // Test with another testing framework.
    PrintColorEncoded(kColorEncodedHelpMessage);
  }
}

// Parses the command line for Google Test flags, without initializing
// other parts of Google Test.
void ParseGoogleTestFlagsOnly(int* argc, char** argv) {
  ParseGoogleTestFlagsOnlyImpl(argc, argv);
}
void ParseGoogleTestFlagsOnly(int* argc, wchar_t** argv) {
  ParseGoogleTestFlagsOnlyImpl(argc, argv);
}

// The internal implementation of InitGoogleTest().
//
// The type parameter CharType can be instantiated to either char or
// wchar_t.
template <typename CharType>
void InitGoogleTestImpl(int* argc, CharType** argv) {
  g_init_gtest_count++;

  // We don't want to run the initialization code twice.
  if (g_init_gtest_count != 1) return;

  if (*argc <= 0) return;

  internal::g_executable_path = internal::StreamableToString(argv[0]);

#if GTEST_HAS_DEATH_TEST
  g_argvs.clear();
  for (int i = 0; i != *argc; i++) {
    g_argvs.push_back(StreamableToString(argv[i]));
  }
#endif  // GTEST_HAS_DEATH_TEST

  ParseGoogleTestFlagsOnly(argc, argv);
  GetUnitTestImpl()->PostFlagParsingInit();
}

}  // namespace internal

// Initializes Google Test.  This must be called before calling
// RUN_ALL_TESTS().  In particular, it parses a command line for the
// flags that Google Test recognizes.  Whenever a Google Test flag is
// seen, it is removed from argv, and *argc is decremented.
//
// No value is returned.  Instead, the Google Test flag variables are
// updated.
//
// Calling the function for the second time has no user-visible effect.
void InitGoogleTest(int* argc, char** argv) {
  internal::InitGoogleTestImpl(argc, argv);
}

// This overloaded version can be used in Windows programs compiled in
// UNICODE mode.
void InitGoogleTest(int* argc, wchar_t** argv) {
  internal::InitGoogleTestImpl(argc, argv);
}

}  // namespace testing
