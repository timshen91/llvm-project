//===- lib/MachineCode/MCContext.cpp - Machine Code Context ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/MCContext.h"

#include "llvm/MC/MCAtom.h"
#include "llvm/MC/MCImm.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCSymbol.h"
using namespace llvm;

MCContext::MCContext()
{
}

MCContext::~MCContext() {
}

MCSection *MCContext::GetSection(const char *Name) {
  MCSection *&Entry = Sections[Name];
  
  if (!Entry)
    Entry = new (this) MCSection(Name);

  return Entry;
}
    
MCAtom *MCContext::CreateAtom(MCSection *Section) {
  return new (this) MCAtom(Section);
}

MCSymbol *MCContext::CreateSymbol(MCAtom *Atom, const char *Name) {
  assert(Name[0] != '\0' && "Normal symbols cannot be unnamed!");

  // Create and bind the symbol, and ensure that names are unique.
  MCSymbol *&Entry = Symbols[Name];
  assert(!Entry && "Duplicate symbol definition!");
  return Entry = new (this) MCSymbol(Atom, Name, false);
}

MCSymbol *MCContext::CreateTemporarySymbol(MCAtom *Atom, const char *Name) {
  // If unnamed, just create a symbol.
  if (Name[0] == '\0')
    new (this) MCSymbol(Atom, "", true);
    
  // Otherwise create as usual.
  MCSymbol *&Entry = Symbols[Name];
  assert(!Entry && "Duplicate symbol definition!");
  return Entry = new (this) MCSymbol(Atom, Name, true);
}

MCSymbol *MCContext::LookupSymbol(const char *Name) const {
  return Symbols.lookup(Name);
}

void MCContext::ClearSymbolValue(MCSymbol *Sym) {
  SymbolValues.erase(Sym);
}

void MCContext::SetSymbolValue(MCSymbol *Sym, const MCImm &Value) {
  SymbolValues[Sym] = Value;
}

const MCImm *MCContext::GetSymbolValue(MCSymbol *Sym) const {
  DenseMap<MCSymbol*, MCImm>::iterator it = SymbolValues.find(Sym);

  if (it == SymbolValues.end())
    return 0;

  return &it->second;
}
