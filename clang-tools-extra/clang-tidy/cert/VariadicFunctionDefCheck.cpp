//===--- VariadicfunctiondefCheck.cpp - clang-tidy-------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "VariadicFunctionDefCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {

void VariadicFunctionDefCheck::registerMatchers(MatchFinder *Finder) {
  if (!getLangOpts().CPlusPlus)
    return;

  // We only care about function *definitions* that are variadic.
  Finder->addMatcher(functionDecl(isDefinition(), isVariadic()).bind("func"),
                     this);
}

void VariadicFunctionDefCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *FD = Result.Nodes.getNodeAs<FunctionDecl>("func");

  diag(FD->getLocation(),
       "do not define a C-style variadic function; consider using a function "
       "parameter pack or currying instead");
}

} // namespace tidy
} // namespace clang

