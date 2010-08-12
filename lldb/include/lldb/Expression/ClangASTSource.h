//===-- ClangASTSource.h ----------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_ClangASTSource_h_
#define liblldb_ClangASTSource_h_

#include "clang/Sema/ExternalSemaSource.h"

namespace lldb_private {
    
class ClangExpressionDeclMap;

//----------------------------------------------------------------------
/// @class ClangASTSource ClangASTSource.h "lldb/Expression/ClangASTSource.h"
/// @brief Provider for named objects defined in the debug info for Clang
///
/// As Clang parses an expression, it may encounter names that are not
/// defined inside the expression, including variables, functions, and
/// types.  Clang knows the name it is looking for, but nothing else.
/// The ExternalSemaSource class provides Decls (VarDecl, FunDecl, TypeDecl)
/// to Clang for these names, consulting the ClangExpressionDeclMap to do
/// the actual lookups.
//----------------------------------------------------------------------
class ClangASTSource : public clang::ExternalSemaSource {
	clang::ASTContext &Context;         ///< The parser's AST context, for copying types into
	ClangExpressionDeclMap &DeclMap;    ///< The object that looks up named entities in LLDB
public:
    friend struct NameSearchContext;
    
    //------------------------------------------------------------------
    /// Constructor
    ///
    /// Initializes class variabes.
    ///
    /// @param[in] context
    ///     A reference to the AST context provided by the parser.
    ///
    /// @param[in] declMap
    ///     A reference to the LLDB object that handles entity lookup.
    //------------------------------------------------------------------
	ClangASTSource(clang::ASTContext &context,
                   ClangExpressionDeclMap &declMap) : 
        Context(context),
        DeclMap(declMap) {}
    
    //------------------------------------------------------------------
    /// Destructor
    //------------------------------------------------------------------
	~ClangASTSource();
	
    //------------------------------------------------------------------
    /// Interface stub that returns NULL.
    //------------------------------------------------------------------
    clang::Decl *GetExternalDecl(uint32_t);
    
    //------------------------------------------------------------------
    /// Interface stub that returns NULL.
    //------------------------------------------------------------------
    clang::Stmt *GetExternalDeclStmt(uint64_t);
	
    //------------------------------------------------------------------
    /// Interface stub that returns an undifferentiated Selector.
    //------------------------------------------------------------------
    clang::Selector GetExternalSelector(uint32_t);
    
    //------------------------------------------------------------------
    /// Interface stub that returns 0.
    //------------------------------------------------------------------
	uint32_t GetNumExternalSelectors();
	
    //------------------------------------------------------------------
    /// Look up all Decls that match a particular name.  Only handles
    /// Identifiers.  Passes the request on to DeclMap, and calls
    /// SetExternalVisibleDeclsForName with the result. 
    ///
    /// @param[in] DC
    ///     The DeclContext to register the found Decls in.
    ///
    /// @param[in] Name
    ///     The name to find entries for.
    ///
    /// @return
    ///     Whatever SetExternalVisibleDeclsForName returns.
    //------------------------------------------------------------------
    clang::DeclContext::lookup_result FindExternalVisibleDeclsByName(const clang::DeclContext *DC,
                                                                     clang::DeclarationName Name);
	
    //------------------------------------------------------------------
    /// Interface stub that returns true.
    //------------------------------------------------------------------
	bool FindExternalLexicalDecls(const clang::DeclContext *DC,
                                  llvm::SmallVectorImpl<clang::Decl*> &Decls);
    
    //------------------------------------------------------------------
    /// Called on entering a translation unit.  Tells Clang by calling
    /// setHasExternalVisibleStorage() and setHasExternalLexicalStorage()
    /// that this object has something to say about undefined names.
    ///
    /// @param[in] ASTConsumer
    ///     Unused.
    //------------------------------------------------------------------
    void StartTranslationUnit(clang::ASTConsumer *Consumer);
};

//----------------------------------------------------------------------
/// @class NameSearchContext ClangASTSource.h "lldb/Expression/ClangASTSource.h"
/// @brief Container for all objects relevant to a single name lookup
///     
/// LLDB needs to create Decls for entities it finds.  This class communicates
/// what name is being searched for and provides helper functions to construct
/// Decls given appropriate type information.
//----------------------------------------------------------------------
struct NameSearchContext {
    ClangASTSource &ASTSource;                          ///< The AST source making the request
    llvm::SmallVectorImpl<clang::NamedDecl*> &Decls;    ///< The list of declarations already constructed
    clang::DeclarationName &Name;                       ///< The name being looked for
    const clang::DeclContext *DC;                       ///< The DeclContext to put declarations into
    
    //------------------------------------------------------------------
    /// Constructor
    ///
    /// Initializes class variables.
    ///
    /// @param[in] astSource
    ///     A reference to the AST source making a request.
    ///
    /// @param[in] decls
    ///     A reference to a list into which new Decls will be placed.  This
    ///     list is typically empty when the function is called.
    ///
    /// @param[in] name
    ///     The name being searched for (always an Identifier).
    ///
    /// @param[in] dc
    ///     The DeclContext to register Decls in.
    //------------------------------------------------------------------
    NameSearchContext (ClangASTSource &astSource,
                       llvm::SmallVectorImpl<clang::NamedDecl*> &decls,
                       clang::DeclarationName &name,
                       const clang::DeclContext *dc) :
        ASTSource(astSource),
        Decls(decls),
        Name(name),
        DC(dc) {}
    
    //------------------------------------------------------------------
    /// Return the AST context for the current search.  Useful when copying
    /// types.
    //------------------------------------------------------------------
    clang::ASTContext *GetASTContext();
    
    //------------------------------------------------------------------
    /// Create a VarDecl with the name being searched for and the provided
    /// type and register it in the right places.
    ///
    /// @param[in] type
    ///     The opaque QualType for the VarDecl being registered.
    //------------------------------------------------------------------
    clang::NamedDecl *AddVarDecl(void *type);
    
    //------------------------------------------------------------------
    /// Create a FunDecl with the name being searched for and the provided
    /// type and register it in the right places.
    ///
    /// @param[in] type
    ///     The opaque QualType for the FunDecl being registered.
    //------------------------------------------------------------------
    clang::NamedDecl *AddFunDecl(void *type);
    
    //------------------------------------------------------------------
    /// Create a FunDecl with the name being searched for and generic
    /// type (i.e. intptr_t NAME_GOES_HERE(...)) and register it in the
    /// right places.
    //------------------------------------------------------------------
    clang::NamedDecl *AddGenericFunDecl();
    
    //------------------------------------------------------------------
    /// Create a TypeDecl with the name being searched for and the provided
    /// type and register it in the right places.
    ///
    /// @param[in] type
    ///     The opaque QualType for the TypeDecl being registered.
    //------------------------------------------------------------------
    clang::NamedDecl *AddTypeDecl(void *type);
};

}

#endif