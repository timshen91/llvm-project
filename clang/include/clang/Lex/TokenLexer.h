//===--- TokenLexer.h - Lex from a token buffer -----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the TokenLexer interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOKENLEXER_H
#define LLVM_CLANG_TOKENLEXER_H

#include "clang/Basic/SourceLocation.h"

namespace clang {
  class MacroInfo;
  class Preprocessor;
  class Token;
  class MacroArgs;
  
/// TokenLexer - This implements a lexer that returns token from a macro body
/// or token stream instead of lexing from a character buffer.  This is used for
/// macro expansion and _Pragma handling, for example.
///
class TokenLexer {
  /// Macro - The macro we are expanding from.  This is null if expanding a
  /// token stream.
  ///
  MacroInfo *Macro;

  /// ActualArgs - The actual arguments specified for a function-like macro, or
  /// null.  The TokenLexer owns the pointed-to object.
  MacroArgs *ActualArgs;

  /// PP - The current preprocessor object we are expanding for.
  ///
  Preprocessor &PP;

  /// Tokens - This is the pointer to an array of tokens that the macro is
  /// defined to, with arguments expanded for function-like macros.  If this is
  /// a token stream, these are the tokens we are returning.  This points into
  /// the macro definition we are lexing from, a scratch buffer allocated from
  /// the preprocessor's bump pointer allocator, or some other buffer that we
  /// may or may not own (depending on OwnsTokens).
  const Token *Tokens;
  
  /// NumTokens - This is the length of the Tokens array.
  ///
  unsigned NumTokens;
  
  /// CurToken - This is the next token that Lex will return.
  ///
  unsigned CurToken;
  
  /// InstantiateLocStart/End - The source location range where this macro was
  /// instantiated.
  SourceLocation InstantiateLocStart, InstantiateLocEnd;
  
  /// Lexical information about the expansion point of the macro: the identifier
  /// that the macro expanded from had these properties.
  bool AtStartOfLine : 1;
  bool HasLeadingSpace : 1;
  
  /// OwnsTokens - This is true if this TokenLexer allocated the Tokens
  /// array, and thus needs to free it when destroyed.  For simple object-like
  /// macros (for example) we just point into the token buffer of the macro
  /// definition, we don't make a copy of it.
  bool OwnsTokens : 1;
  
  /// DisableMacroExpansion - This is true when tokens lexed from the TokenLexer
  /// should not be subject to further macro expansion.
  bool DisableMacroExpansion : 1;
  
  TokenLexer(const TokenLexer&);  // DO NOT IMPLEMENT
  void operator=(const TokenLexer&); // DO NOT IMPLEMENT
public:
  /// Create a TokenLexer for the specified macro with the specified actual
  /// arguments.  Note that this ctor takes ownership of the ActualArgs pointer.
  /// ILEnd specifies the location of the ')' for a function-like macro or the
  /// identifier for an object-like macro.
  TokenLexer(Token &Tok, SourceLocation ILEnd, MacroArgs *ActualArgs,
             Preprocessor &pp)
    : Macro(0), ActualArgs(0), PP(pp), OwnsTokens(false) {
    Init(Tok, ILEnd, ActualArgs);
  }
  
  /// Init - Initialize this TokenLexer to expand from the specified macro
  /// with the specified argument information.  Note that this ctor takes
  /// ownership of the ActualArgs pointer.  ILEnd specifies the location of the
  /// ')' for a function-like macro or the identifier for an object-like macro.
  void Init(Token &Tok, SourceLocation ILEnd, MacroArgs *ActualArgs);
  
  /// Create a TokenLexer for the specified token stream.  If 'OwnsTokens' is
  /// specified, this takes ownership of the tokens and delete[]'s them when
  /// the token lexer is empty.
  TokenLexer(const Token *TokArray, unsigned NumToks, bool DisableExpansion,
             bool ownsTokens, Preprocessor &pp)
    : Macro(0), ActualArgs(0), PP(pp), OwnsTokens(false) {
    Init(TokArray, NumToks, DisableExpansion, ownsTokens);
  }
  
  /// Init - Initialize this TokenLexer with the specified token stream.
  /// This does not take ownership of the specified token vector.
  ///
  /// DisableExpansion is true when macro expansion of tokens lexed from this 
  /// stream should be disabled.
  void Init(const Token *TokArray, unsigned NumToks,
            bool DisableMacroExpansion, bool OwnsTokens);
  
  ~TokenLexer() { destroy(); }
  
  /// isNextTokenLParen - If the next token lexed will pop this macro off the
  /// expansion stack, return 2.  If the next unexpanded token is a '(', return
  /// 1, otherwise return 0.
  unsigned isNextTokenLParen() const;
  
  /// Lex - Lex and return a token from this macro stream.
  void Lex(Token &Tok);
  
private:
  void destroy();
  
  /// isAtEnd - Return true if the next lex call will pop this macro off the
  /// include stack.
  bool isAtEnd() const {
    return CurToken == NumTokens;
  }
  
  /// PasteTokens - Tok is the LHS of a ## operator, and CurToken is the ##
  /// operator.  Read the ## and RHS, and paste the LHS/RHS together.  If there
  /// are is another ## after it, chomp it iteratively.  Return the result as
  /// Tok.  If this returns true, the caller should immediately return the
  /// token.
  bool PasteTokens(Token &Tok);
  
  /// Expand the arguments of a function-like macro so that we can quickly
  /// return preexpanded tokens from Tokens.
  void ExpandFunctionArguments();
  
  /// HandleMicrosoftCommentPaste - In microsoft compatibility mode, /##/ pastes
  /// together to form a comment that comments out everything in the current
  /// macro, other active macros, and anything left on the current physical
  /// source line of the instantiated buffer.  Handle this by returning the
  /// first token on the next line.
  void HandleMicrosoftCommentPaste(Token &Tok);
};

}  // end namespace clang

#endif
