//===--- ParseAST.cpp - Provide the clang::ParseAST method ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the clang::ParseAST method.
//
//===----------------------------------------------------------------------===//

#include "clang/Sema/ParseAST.h"
#include "Sema.h"
#include "clang/Sema/CodeCompleteConsumer.h"
#include "clang/Sema/SemaConsumer.h"
#include "clang/Sema/ExternalSemaSource.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ExternalASTSource.h"
#include "clang/AST/Stmt.h"
#include "clang/Parse/Parser.h"
#include <cstdio>

using namespace clang;

static void DumpRecordLayouts(ASTContext &C) {
  for (ASTContext::type_iterator I = C.types_begin(), E = C.types_end();
       I != E; ++I) {
    const RecordType *RT = dyn_cast<RecordType>(*I);
    if (!RT)
      continue;

    const CXXRecordDecl *RD = dyn_cast<CXXRecordDecl>(RT->getDecl());
    if (!RD || RD->isImplicit() || RD->isDependentType() ||
        RD->isInvalidDecl() || !RD->getDefinition())
      continue;

    // FIXME: Do we really need to hard code this?
    if (RD->getQualifiedNameAsString() == "__va_list_tag")
      continue;

    C.DumpRecordLayout(RD, llvm::errs());
  }
}

//===----------------------------------------------------------------------===//
// Public interface to the file
//===----------------------------------------------------------------------===//

/// ParseAST - Parse the entire file specified, notifying the ASTConsumer as
/// the file is parsed.  This inserts the parsed decls into the translation unit
/// held by Ctx.
///
void clang::ParseAST(Preprocessor &PP, ASTConsumer *Consumer,
                     ASTContext &Ctx, bool PrintStats,
                     bool CompleteTranslationUnit,
                     CodeCompleteConsumer *CompletionConsumer) {
  // Collect global stats on Decls/Stmts (until we have a module streamer).
  if (PrintStats) {
    Decl::CollectingStats(true);
    Stmt::CollectingStats(true);
  }

  Sema S(PP, Ctx, *Consumer, CompleteTranslationUnit, CompletionConsumer);
  Parser P(PP, S);
  if (PP.EnterMainSourceFile())
    return;

  // Initialize the parser.
  P.Initialize();

  Consumer->Initialize(Ctx);

  if (SemaConsumer *SC = dyn_cast<SemaConsumer>(Consumer))
    SC->InitializeSema(S);

  if (ExternalASTSource *External = Ctx.getExternalSource()) {
    if (ExternalSemaSource *ExternalSema =
          dyn_cast<ExternalSemaSource>(External))
      ExternalSema->InitializeSema(S);

    External->StartTranslationUnit(Consumer);
  }

  Parser::DeclGroupPtrTy ADecl;

  while (!P.ParseTopLevelDecl(ADecl)) {  // Not end of file.
    // If we got a null return and something *was* parsed, ignore it.  This
    // is due to a top-level semicolon, an action override, or a parse error
    // skipping something.
    if (ADecl)
      Consumer->HandleTopLevelDecl(ADecl.getAsVal<DeclGroupRef>());
  };
  // Check for any pending objective-c implementation decl.
  while ((ADecl = P.RetrievePendingObjCImpDecl()))
    Consumer->HandleTopLevelDecl(ADecl.getAsVal<DeclGroupRef>());

  // Process any TopLevelDecls generated by #pragma weak.
  for (llvm::SmallVector<Decl*,2>::iterator
        I = S.WeakTopLevelDecls().begin(),
        E = S.WeakTopLevelDecls().end(); I != E; ++I)
    Consumer->HandleTopLevelDecl(DeclGroupRef(*I));

  // Dump record layouts, if requested.
  if (PP.getLangOptions().DumpRecordLayouts)
    DumpRecordLayouts(Ctx);

  Consumer->HandleTranslationUnit(Ctx);

  if (ExternalSemaSource *ESS =
        dyn_cast_or_null<ExternalSemaSource>(Ctx.getExternalSource()))
    ESS->ForgetSema();

  if (SemaConsumer *SC = dyn_cast<SemaConsumer>(Consumer))
    SC->ForgetSema();

  if (PrintStats) {
    fprintf(stderr, "\nSTATISTICS:\n");
    P.getActions().PrintStats();
    Ctx.PrintStats();
    Decl::PrintStats();
    Stmt::PrintStats();
    Consumer->PrintStats();
  }
}
