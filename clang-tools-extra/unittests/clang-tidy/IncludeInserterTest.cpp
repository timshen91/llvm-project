//===---- IncludeInserterTest.cpp - clang-tidy ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "../clang-tidy/IncludeInserter.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "ClangTidyTest.h"
#include "gtest/gtest.h"

// FIXME: Canonicalize paths correctly on windows.
// Currently, adding virtual files will canonicalize the paths before
// storing the virtual entries.
// When resolving virtual entries in the FileManager, the paths (for
// example coming from a #include directive) are not canonicalized
// to native paths; thus, the virtual file is not found.
// This needs to be fixed in the FileManager before we can make
// clang-tidy tests work.
#if !defined(_WIN32)

namespace clang {
namespace tidy {
namespace {

class IncludeInserterCheckBase : public ClangTidyCheck {
public:
  IncludeInserterCheckBase(StringRef CheckName, ClangTidyContext *Context)
      : ClangTidyCheck(CheckName, Context) {}
  void registerPPCallbacks(CompilerInstance &Compiler) override {
    Inserter.reset(new IncludeInserter(Compiler.getSourceManager(),
                                       Compiler.getLangOpts(),
                                       IncludeSorter::IS_Google));
    Compiler.getPreprocessor().addPPCallbacks(Inserter->CreatePPCallbacks());
  }

  void registerMatchers(ast_matchers::MatchFinder *Finder) override {
    Finder->addMatcher(ast_matchers::declStmt().bind("stmt"), this);
  }

  void check(const ast_matchers::MatchFinder::MatchResult &Result) override {
    auto Fixit =
        Inserter->CreateIncludeInsertion(Result.SourceManager->getMainFileID(),
                                         HeaderToInclude(), IsAngledInclude());
    if (Fixit) {
      diag(Result.Nodes.getStmtAs<DeclStmt>("stmt")->getLocStart(), "foo, bar")
          << *Fixit;
    }
    // Second include should yield no Fixit.
    Fixit =
        Inserter->CreateIncludeInsertion(Result.SourceManager->getMainFileID(),
                                         HeaderToInclude(), IsAngledInclude());
    EXPECT_FALSE(Fixit);
  }

  virtual StringRef HeaderToInclude() const = 0;
  virtual bool IsAngledInclude() const = 0;

  std::unique_ptr<IncludeInserter> Inserter;
};

class NonSystemHeaderInserterCheck : public IncludeInserterCheckBase {
public:
  NonSystemHeaderInserterCheck(StringRef CheckName, ClangTidyContext *Context)
      : IncludeInserterCheckBase(CheckName, Context) {}
  StringRef HeaderToInclude() const override { return "path/to/header.h"; }
  bool IsAngledInclude() const override { return false; }
};

class CXXSystemIncludeInserterCheck : public IncludeInserterCheckBase {
public:
  CXXSystemIncludeInserterCheck(StringRef CheckName, ClangTidyContext *Context)
      : IncludeInserterCheckBase(CheckName, Context) {}
  StringRef HeaderToInclude() const override { return "set"; }
  bool IsAngledInclude() const override { return true; }
};

template <typename Check>
std::string runCheckOnCode(StringRef Code, StringRef Filename,
                           size_t NumWarningsExpected) {
  std::vector<ClangTidyError> Errors;
  return test::runCheckOnCode<Check>(Code, &Errors, Filename, None,
                                     ClangTidyOptions(),
                                     {// Main file include
                                      {"clang_tidy/tests/"
                                       "insert_includes_test_header.h",
                                       "\n"},
                                      // Non system headers
                                      {"path/to/a/header.h", "\n"},
                                      {"path/to/z/header.h", "\n"},
                                      {"path/to/header.h", "\n"},
                                      // Fake system headers.
                                      {"stdlib.h", "\n"},
                                      {"unistd.h", "\n"},
                                      {"list", "\n"},
                                      {"map", "\n"},
                                      {"set", "\n"},
                                      {"vector", "\n"}});
  EXPECT_EQ(NumWarningsExpected, Errors.size());
}

TEST(IncludeInserterTest, InsertAfterLastNonSystemInclude) {
  const char *PreCode = R"(
#include "clang_tidy/tests/insert_includes_test_header.h"

#include <list>
#include <map>

#include "path/to/a/header.h"

void foo() {
  int a = 0;
})";
  const char *PostCode = R"(
#include "clang_tidy/tests/insert_includes_test_header.h"

#include <list>
#include <map>

#include "path/to/a/header.h"
#include "path/to/header.h"

void foo() {
  int a = 0;
})";

  EXPECT_EQ(PostCode, runCheckOnCode<NonSystemHeaderInserterCheck>(
                          PreCode, "clang_tidy/tests/"
                                   "insert_includes_test_input2.cc",
                          1));
}

TEST(IncludeInserterTest, InsertBeforeFirstNonSystemInclude) {
  const char *PreCode = R"(
#include "clang_tidy/tests/insert_includes_test_header.h"

#include <list>
#include <map>

#include "path/to/z/header.h"

void foo() {
  int a = 0;
})";
  const char *PostCode = R"(
#include "clang_tidy/tests/insert_includes_test_header.h"

#include <list>
#include <map>

#include "path/to/header.h"
#include "path/to/z/header.h"

void foo() {
  int a = 0;
})";

  EXPECT_EQ(PostCode, runCheckOnCode<NonSystemHeaderInserterCheck>(
                          PreCode, "clang_tidy/tests/"
                                   "insert_includes_test_input2.cc",
                          1));
}

TEST(IncludeInserterTest, InsertBetweenNonSystemIncludes) {
  const char *PreCode = R"(
#include "clang_tidy/tests/insert_includes_test_header.h"

#include <list>
#include <map>

#include "path/to/a/header.h"
#include "path/to/z/header.h"

void foo() {
  int a = 0;
})";
  const char *PostCode = R"(
#include "clang_tidy/tests/insert_includes_test_header.h"

#include <list>
#include <map>

#include "path/to/a/header.h"
#include "path/to/header.h"
#include "path/to/z/header.h"

void foo() {
  int a = 0;
})";

  EXPECT_EQ(PostCode, runCheckOnCode<NonSystemHeaderInserterCheck>(
                          PreCode, "clang_tidy/tests/"
                                   "insert_includes_test_input2.cc",
                          1));
}

TEST(IncludeInserterTest, NonSystemIncludeAlreadyIncluded) {
  const char *PreCode = R"(
#include "clang_tidy/tests/insert_includes_test_header.h"

#include <list>
#include <map>

#include "path/to/a/header.h"
#include "path/to/header.h"
#include "path/to/z/header.h"

void foo() {
  int a = 0;
})";
  EXPECT_EQ(PreCode, runCheckOnCode<NonSystemHeaderInserterCheck>(
                         PreCode, "clang_tidy/tests/"
                                  "insert_includes_test_input2.cc",
                         0));
}

TEST(IncludeInserterTest, InsertNonSystemIncludeAfterLastCXXSystemInclude) {
  const char *PreCode = R"(
#include "clang_tidy/tests/insert_includes_test_header.h"

#include <list>
#include <map>

void foo() {
  int a = 0;
})";
  const char *PostCode = R"(
#include "clang_tidy/tests/insert_includes_test_header.h"

#include <list>
#include <map>

#include "path/to/header.h"

void foo() {
  int a = 0;
})";

  EXPECT_EQ(PostCode, runCheckOnCode<NonSystemHeaderInserterCheck>(
                          PreCode, "clang_tidy/tests/"
                                   "insert_includes_test_header.cc",
                          1));
}

TEST(IncludeInserterTest, InsertNonSystemIncludeAfterMainFileInclude) {
  const char *PreCode = R"(
#include "clang_tidy/tests/insert_includes_test_header.h"

void foo() {
  int a = 0;
})";
  const char *PostCode = R"(
#include "clang_tidy/tests/insert_includes_test_header.h"

#include "path/to/header.h"

void foo() {
  int a = 0;
})";

  EXPECT_EQ(PostCode, runCheckOnCode<NonSystemHeaderInserterCheck>(
                          PreCode, "clang_tidy/tests/"
                                   "insert_includes_test_header.cc",
                          1));
}

TEST(IncludeInserterTest, InsertCXXSystemIncludeAfterLastCXXSystemInclude) {
  const char *PreCode = R"(
#include "clang_tidy/tests/insert_includes_test_header.h"

#include <list>
#include <map>

#include "path/to/a/header.h"

void foo() {
  int a = 0;
})";
  const char *PostCode = R"(
#include "clang_tidy/tests/insert_includes_test_header.h"

#include <list>
#include <map>
#include <set>

#include "path/to/a/header.h"

void foo() {
  int a = 0;
})";

  EXPECT_EQ(PostCode, runCheckOnCode<CXXSystemIncludeInserterCheck>(
                          PreCode, "clang_tidy/tests/"
                                   "insert_includes_test_header.cc",
                          1));
}

TEST(IncludeInserterTest, InsertCXXSystemIncludeBeforeFirstCXXSystemInclude) {
  const char *PreCode = R"(
#include "clang_tidy/tests/insert_includes_test_header.h"

#include <vector>

#include "path/to/a/header.h"

void foo() {
  int a = 0;
})";
  const char *PostCode = R"(
#include "clang_tidy/tests/insert_includes_test_header.h"

#include <set>
#include <vector>

#include "path/to/a/header.h"

void foo() {
  int a = 0;
})";

  EXPECT_EQ(PostCode, runCheckOnCode<CXXSystemIncludeInserterCheck>(
                          PreCode, "clang_tidy/tests/"
                                   "insert_includes_test_header.cc",
                          1));
}

TEST(IncludeInserterTest, InsertCXXSystemIncludeBetweenCXXSystemIncludes) {
  const char *PreCode = R"(
#include "clang_tidy/tests/insert_includes_test_header.h"

#include <map>
#include <vector>

#include "path/to/a/header.h"

void foo() {
  int a = 0;
})";
  const char *PostCode = R"(
#include "clang_tidy/tests/insert_includes_test_header.h"

#include <map>
#include <set>
#include <vector>

#include "path/to/a/header.h"

void foo() {
  int a = 0;
})";

  EXPECT_EQ(PostCode, runCheckOnCode<CXXSystemIncludeInserterCheck>(
                          PreCode, "clang_tidy/tests/"
                                   "insert_includes_test_header.cc",
                          1));
}

TEST(IncludeInserterTest, InsertCXXSystemIncludeAfterMainFileInclude) {
  const char *PreCode = R"(
#include "clang_tidy/tests/insert_includes_test_header.h"

#include "path/to/a/header.h"

void foo() {
  int a = 0;
})";
  const char *PostCode = R"(
#include "clang_tidy/tests/insert_includes_test_header.h"

#include <set>

#include "path/to/a/header.h"

void foo() {
  int a = 0;
})";

  EXPECT_EQ(PostCode, runCheckOnCode<CXXSystemIncludeInserterCheck>(
                          PreCode, "clang_tidy/tests/"
                                   "insert_includes_test_header.cc",
                          1));
}

TEST(IncludeInserterTest, InsertCXXSystemIncludeAfterCSystemInclude) {
  const char *PreCode = R"(
#include "clang_tidy/tests/insert_includes_test_header.h"

#include <stdlib.h>

#include "path/to/a/header.h"

void foo() {
  int a = 0;
})";
  const char *PostCode = R"(
#include "clang_tidy/tests/insert_includes_test_header.h"

#include <stdlib.h>

#include <set>

#include "path/to/a/header.h"

void foo() {
  int a = 0;
})";

  EXPECT_EQ(PostCode, runCheckOnCode<CXXSystemIncludeInserterCheck>(
                          PreCode, "clang_tidy/tests/"
                                   "insert_includes_test_header.cc",
                          1));
}

} // namespace
} // namespace tidy
} // namespace clang

#endif