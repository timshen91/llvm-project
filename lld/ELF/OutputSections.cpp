//===- OutputSections.cpp -------------------------------------------------===//
//
//                             The LLVM Linker
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "OutputSections.h"
#include "Config.h"
#include "SymbolTable.h"
#include "Target.h"
#include "llvm/Support/MathExtras.h"

using namespace llvm;
using namespace llvm::object;
using namespace llvm::support::endian;
using namespace llvm::ELF;

using namespace lld;
using namespace lld::elf2;

template <class ELFT>
OutputSectionBase<ELFT>::OutputSectionBase(StringRef Name, uint32_t sh_type,
                                           uintX_t sh_flags)
    : Name(Name) {
  memset(&Header, 0, sizeof(Elf_Shdr));
  Header.sh_type = sh_type;
  Header.sh_flags = sh_flags;
}

template <class ELFT>
GotPltSection<ELFT>::GotPltSection()
    : OutputSectionBase<ELFT>(".got.plt", llvm::ELF::SHT_PROGBITS,
                              llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_WRITE) {
  this->Header.sh_addralign = sizeof(uintX_t);
  // .got.plt has 3 reserved entry
  Entries.resize(3);
}

template <class ELFT> void GotPltSection<ELFT>::addEntry(SymbolBody *Sym) {
  Sym->GotPltIndex = Entries.size();
  Entries.push_back(Sym);
}

template <class ELFT> bool GotPltSection<ELFT>::empty() const {
  return Entries.size() == 3;
}

template <class ELFT>
typename GotPltSection<ELFT>::uintX_t
GotPltSection<ELFT>::getEntryAddr(const SymbolBody &B) const {
  return this->getVA() + B.GotPltIndex * sizeof(uintX_t);
}

template <class ELFT> void GotPltSection<ELFT>::finalize() {
  this->Header.sh_size = Entries.size() * sizeof(uintX_t);
}

template <class ELFT> void GotPltSection<ELFT>::writeTo(uint8_t *Buf) {
  write<uintX_t, ELFT::TargetEndianness, sizeof(uintX_t)>(
      Buf, Out<ELFT>::Dynamic->getVA());
  for (const SymbolBody *B : Entries) {
    if (B)
      Target->writeGotPltEntry(Buf, Out<ELFT>::Plt->getEntryAddr(*B));
    Buf += sizeof(uintX_t);
  }
}

template <class ELFT>
GotSection<ELFT>::GotSection()
    : OutputSectionBase<ELFT>(".got", llvm::ELF::SHT_PROGBITS,
                              llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_WRITE) {
  this->Header.sh_addralign = sizeof(uintX_t);
}

template <class ELFT> void GotSection<ELFT>::addEntry(SymbolBody *Sym) {
  Sym->GotIndex = Entries.size();
  Entries.push_back(Sym);
}

template <class ELFT>
typename GotSection<ELFT>::uintX_t
GotSection<ELFT>::getEntryAddr(const SymbolBody &B) const {
  return this->getVA() + B.GotIndex * sizeof(uintX_t);
}

template <class ELFT> void GotSection<ELFT>::writeTo(uint8_t *Buf) {
  for (const SymbolBody *B : Entries) {
    uint8_t *Entry = Buf;
    Buf += sizeof(uintX_t);
    if (canBePreempted(B, false))
      continue; // The dynamic linker will take care of it.
    uintX_t VA = getSymVA<ELFT>(*B);
    write<uintX_t, ELFT::TargetEndianness, sizeof(uintX_t)>(Entry, VA);
  }
}

template <class ELFT>
PltSection<ELFT>::PltSection()
    : OutputSectionBase<ELFT>(".plt", llvm::ELF::SHT_PROGBITS,
                              llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_EXECINSTR) {
  this->Header.sh_addralign = 16;
}

template <class ELFT> void PltSection<ELFT>::writeTo(uint8_t *Buf) {
  size_t Off = 0;
  bool LazyReloc = Target->supportsLazyRelocations();
  if (LazyReloc) {
    // First write PLT[0] entry which is special.
    Target->writePltZeroEntry(Buf, Out<ELFT>::GotPlt->getVA(), this->getVA());
    Off += Target->getPltZeroEntrySize();
  }
  for (const SymbolBody *E : Entries) {
    uint64_t Got = LazyReloc ? Out<ELFT>::GotPlt->getEntryAddr(*E)
                             : Out<ELFT>::Got->getEntryAddr(*E);
    uint64_t Plt = this->getVA() + Off;
    Target->writePltEntry(Buf + Off, Got, Plt, E->PltIndex);
    Off += Target->getPltEntrySize();
  }
}

template <class ELFT> void PltSection<ELFT>::addEntry(SymbolBody *Sym) {
  Sym->PltIndex = Entries.size();
  Entries.push_back(Sym);
}

template <class ELFT>
typename PltSection<ELFT>::uintX_t
PltSection<ELFT>::getEntryAddr(const SymbolBody &B) const {
  return this->getVA() + Target->getPltZeroEntrySize() +
         B.PltIndex * Target->getPltEntrySize();
}

template <class ELFT> void PltSection<ELFT>::finalize() {
  this->Header.sh_size = Target->getPltZeroEntrySize() +
                         Entries.size() * Target->getPltEntrySize();
}

template <class ELFT>
RelocationSection<ELFT>::RelocationSection(StringRef Name, bool IsRela)
    : OutputSectionBase<ELFT>(Name,
                              IsRela ? llvm::ELF::SHT_RELA : llvm::ELF::SHT_REL,
                              llvm::ELF::SHF_ALLOC),
      IsRela(IsRela) {
  this->Header.sh_entsize = IsRela ? sizeof(Elf_Rela) : sizeof(Elf_Rel);
  this->Header.sh_addralign = ELFT::Is64Bits ? 8 : 4;
}

template <class ELFT> void RelocationSection<ELFT>::writeTo(uint8_t *Buf) {
  const unsigned EntrySize = IsRela ? sizeof(Elf_Rela) : sizeof(Elf_Rel);
  for (const DynamicReloc<ELFT> &Rel : Relocs) {
    auto *P = reinterpret_cast<Elf_Rel *>(Buf);
    Buf += EntrySize;

    const InputSection<ELFT> &C = Rel.C;
    const Elf_Rel &RI = Rel.RI;
    uint32_t SymIndex = RI.getSymbol(Config->Mips64EL);
    const ObjectFile<ELFT> &File = *C.getFile();
    SymbolBody *Body = File.getSymbolBody(SymIndex);
    if (Body)
      Body = Body->repl();

    uint32_t Type = RI.getType(Config->Mips64EL);
    bool NeedsGot = Body && Target->relocNeedsGot(Type, *Body);
    bool CanBePreempted = canBePreempted(Body, NeedsGot);
    bool LazyReloc = Body && Target->supportsLazyRelocations() &&
                     Target->relocNeedsPlt(Type, *Body);

    if (CanBePreempted) {
      if (NeedsGot)
        P->setSymbolAndType(Body->getDynamicSymbolTableIndex(),
                            LazyReloc ? Target->getPltReloc()
                                      : Target->getGotReloc(),
                            Config->Mips64EL);
      else
        P->setSymbolAndType(Body->getDynamicSymbolTableIndex(), Type,
                            Config->Mips64EL);
    } else {
      P->setSymbolAndType(0, Target->getRelativeReloc(), Config->Mips64EL);
    }

    if (NeedsGot) {
      if (LazyReloc)
        P->r_offset = Out<ELFT>::GotPlt->getEntryAddr(*Body);
      else
        P->r_offset = Out<ELFT>::Got->getEntryAddr(*Body);
    } else {
      P->r_offset = RI.r_offset + C.OutSec->getVA() + C.OutSecOff;
    }

    uintX_t OrigAddend = 0;
    if (IsRela && !NeedsGot)
      OrigAddend = static_cast<const Elf_Rela &>(RI).r_addend;

    uintX_t Addend;
    if (CanBePreempted)
      Addend = OrigAddend;
    else if (Body)
      Addend = getSymVA<ELFT>(cast<ELFSymbolBody<ELFT>>(*Body)) + OrigAddend;
    else if (IsRela)
      Addend = getLocalRelTarget(File, static_cast<const Elf_Rela &>(RI));
    else
      Addend = getLocalRelTarget(File, RI);

    if (IsRela)
      static_cast<Elf_Rela *>(P)->r_addend = Addend;
  }
}

template <class ELFT> void RelocationSection<ELFT>::finalize() {
  this->Header.sh_link = Out<ELFT>::DynSymTab->SectionIndex;
  this->Header.sh_size = Relocs.size() * this->Header.sh_entsize;
}

template <class ELFT>
InterpSection<ELFT>::InterpSection()
    : OutputSectionBase<ELFT>(".interp", llvm::ELF::SHT_PROGBITS,
                              llvm::ELF::SHF_ALLOC) {
  this->Header.sh_size = Config->DynamicLinker.size() + 1;
  this->Header.sh_addralign = 1;
}

template <class ELFT>
void OutputSectionBase<ELFT>::writeHeaderTo(Elf_Shdr *SHdr) {
  *SHdr = Header;
}

template <class ELFT> void InterpSection<ELFT>::writeTo(uint8_t *Buf) {
  memcpy(Buf, Config->DynamicLinker.data(), Config->DynamicLinker.size());
}

template <class ELFT>
HashTableSection<ELFT>::HashTableSection()
    : OutputSectionBase<ELFT>(".hash", llvm::ELF::SHT_HASH,
                              llvm::ELF::SHF_ALLOC) {
  this->Header.sh_entsize = sizeof(Elf_Word);
  this->Header.sh_addralign = sizeof(Elf_Word);
}

static uint32_t hashSysv(StringRef Name) {
  uint32_t H = 0;
  for (char C : Name) {
    H = (H << 4) + C;
    uint32_t G = H & 0xf0000000;
    if (G)
      H ^= G >> 24;
    H &= ~G;
  }
  return H;
}

template <class ELFT> void HashTableSection<ELFT>::finalize() {
  this->Header.sh_link = Out<ELFT>::DynSymTab->SectionIndex;

  unsigned NumEntries = 2;                 // nbucket and nchain.
  NumEntries += Out<ELFT>::DynSymTab->getNumSymbols(); // The chain entries.

  // Create as many buckets as there are symbols.
  // FIXME: This is simplistic. We can try to optimize it, but implementing
  // support for SHT_GNU_HASH is probably even more profitable.
  NumEntries += Out<ELFT>::DynSymTab->getNumSymbols();
  this->Header.sh_size = NumEntries * sizeof(Elf_Word);
}

template <class ELFT> void HashTableSection<ELFT>::writeTo(uint8_t *Buf) {
  unsigned NumSymbols = Out<ELFT>::DynSymTab->getNumSymbols();
  auto *P = reinterpret_cast<Elf_Word *>(Buf);
  *P++ = NumSymbols; // nbucket
  *P++ = NumSymbols; // nchain

  Elf_Word *Buckets = P;
  Elf_Word *Chains = P + NumSymbols;

  for (const typename SymbolTableSection<ELFT>::SymbolData &Item :
       Out<ELFT>::DynSymTab->getSymbols()) {
    SymbolBody *Body = Item.Body;
    StringRef Name = Body->getName();
    unsigned I = Body->getDynamicSymbolTableIndex();
    uint32_t Hash = hashSysv(Name) % NumSymbols;
    Chains[I] = Buckets[Hash];
    Buckets[Hash] = I;
  }
}

static uint32_t hashGnu(StringRef Name) {
  uint32_t H = 5381;
  for (uint8_t C : Name)
    H = (H << 5) + H + C;
  return H;
}

template <class ELFT>
GnuHashTableSection<ELFT>::GnuHashTableSection()
    : OutputSectionBase<ELFT>(".gnu.hash", llvm::ELF::SHT_GNU_HASH,
                              llvm::ELF::SHF_ALLOC) {
  this->Header.sh_entsize = ELFT::Is64Bits ? 0 : 4;
  this->Header.sh_addralign = ELFT::Is64Bits ? 8 : 4;
}

template <class ELFT>
unsigned GnuHashTableSection<ELFT>::calcNBuckets(unsigned NumHashed) {
  if (!NumHashed)
    return 0;

  // These values are prime numbers which are not greater than 2^(N-1) + 1.
  // In result, for any particular NumHashed we return a prime number
  // which is not greater than NumHashed.
  static const unsigned Primes[] = {
      1,   1,    3,    3,    7,    13,    31,    61,    127,   251,
      509, 1021, 2039, 4093, 8191, 16381, 32749, 65521, 131071};

  return Primes[std::min<unsigned>(Log2_32_Ceil(NumHashed),
                                   array_lengthof(Primes) - 1)];
}

// Bloom filter estimation: at least 8 bits for each hashed symbol.
// GNU Hash table requirement: it should be a power of 2,
//   the minimum value is 1, even for an empty table.
// Expected results for a 32-bit target:
//   calcMaskWords(0..4)   = 1
//   calcMaskWords(5..8)   = 2
//   calcMaskWords(9..16)  = 4
// For a 64-bit target:
//   calcMaskWords(0..8)   = 1
//   calcMaskWords(9..16)  = 2
//   calcMaskWords(17..32) = 4
template <class ELFT>
unsigned GnuHashTableSection<ELFT>::calcMaskWords(unsigned NumHashed) {
  if (!NumHashed)
    return 1;
  return NextPowerOf2((NumHashed - 1) / sizeof(Elf_Off));
}

template <class ELFT> void GnuHashTableSection<ELFT>::finalize() {
  const unsigned NumHashed = Out<ELFT>::DynSymTab->getNumGnuHashSymbols();
  NBuckets = calcNBuckets(NumHashed);
  MaskWords = calcMaskWords(NumHashed);
  // Second hash shift estimation: just predefined values.
  Shift2 = ELFT::Is64Bits ? 6 : 5;

  this->Header.sh_link = Out<ELFT>::DynSymTab->SectionIndex;
  this->Header.sh_size = sizeof(Elf_Word) * 4            // Header
                         + sizeof(Elf_Off) * MaskWords   // Bloom Filter
                         + sizeof(Elf_Word) * NBuckets   // Hash Buckets
                         + sizeof(Elf_Word) * NumHashed; // Hash Values
}

template <class ELFT> void GnuHashTableSection<ELFT>::writeTo(uint8_t *Buf) {
  writeHeader(Buf);
  if (!NBuckets) // There are no hashed symbols
    return;
  writeBloomFilter(Buf);
  writeHashTable(Buf);
}

template <class ELFT>
void GnuHashTableSection<ELFT>::writeHeader(uint8_t *&Buf) {
  auto *P = reinterpret_cast<Elf_Word *>(Buf);
  *P++ = NBuckets;
  *P++ = Out<ELFT>::DynSymTab->getNumSymbols() -
         Out<ELFT>::DynSymTab->getNumGnuHashSymbols();
  *P++ = MaskWords;
  *P++ = Shift2;
  Buf = reinterpret_cast<uint8_t *>(P);
}

template <class ELFT>
void GnuHashTableSection<ELFT>::writeBloomFilter(uint8_t *&Buf) {
  unsigned C = sizeof(Elf_Off) * 8;

  auto *Masks = reinterpret_cast<Elf_Off *>(Buf);
  for (const typename SymbolTableSection<ELFT>::SymbolData &Item :
       Out<ELFT>::DynSymTab->getGnuHashSymbols()) {
    size_t Pos = (Item.GnuHash / C) & (MaskWords - 1);
    uintX_t V = (uintX_t(1) << (Item.GnuHash % C)) |
                (uintX_t(1) << ((Item.GnuHash >> Shift2) % C));
    Masks[Pos] |= V;
  }
  Buf += sizeof(Elf_Off) * MaskWords;
}

template <class ELFT>
void GnuHashTableSection<ELFT>::writeHashTable(uint8_t *Buf) {
  Elf_Word *Buckets = reinterpret_cast<Elf_Word *>(Buf);
  Elf_Word *Values = Buckets + NBuckets;

  int PrevBucket = -1;
  int I = 0;
  for (const typename SymbolTableSection<ELFT>::SymbolData &Item :
       Out<ELFT>::DynSymTab->getGnuHashSymbols()) {
    int Bucket = Item.GnuHash % NBuckets;
    assert(PrevBucket <= Bucket);
    if (Bucket != PrevBucket) {
      Buckets[Bucket] = Item.Body->getDynamicSymbolTableIndex();
      PrevBucket = Bucket;
      if (I > 0)
        Values[I - 1] |= 1;
    }
    Values[I] = Item.GnuHash & ~1;
    ++I;
  }
  if (I > 0)
    Values[I - 1] |= 1;
}

template <class ELFT>
DynamicSection<ELFT>::DynamicSection(SymbolTable<ELFT> &SymTab)
    : OutputSectionBase<ELFT>(".dynamic", llvm::ELF::SHT_DYNAMIC,
                              llvm::ELF::SHF_ALLOC | llvm::ELF::SHF_WRITE),
      SymTab(SymTab) {
  Elf_Shdr &Header = this->Header;
  Header.sh_addralign = ELFT::Is64Bits ? 8 : 4;
  Header.sh_entsize = ELFT::Is64Bits ? 16 : 8;
}

template <class ELFT> void DynamicSection<ELFT>::finalize() {
  if (this->Header.sh_size)
    return; // Already finalized.

  Elf_Shdr &Header = this->Header;
  Header.sh_link = Out<ELFT>::DynStrTab->SectionIndex;

  unsigned NumEntries = 0;
  if (Out<ELFT>::RelaDyn->hasRelocs()) {
    ++NumEntries; // DT_RELA / DT_REL
    ++NumEntries; // DT_RELASZ / DT_RELSZ
    ++NumEntries; // DT_RELAENT / DT_RELENT
  }
  if (Out<ELFT>::RelaPlt && Out<ELFT>::RelaPlt->hasRelocs()) {
    ++NumEntries; // DT_JMPREL
    ++NumEntries; // DT_PLTRELSZ
    ++NumEntries; // DT_PLTGOT
    ++NumEntries; // DT_PLTREL
  }

  ++NumEntries; // DT_SYMTAB
  ++NumEntries; // DT_SYMENT
  ++NumEntries; // DT_STRTAB
  ++NumEntries; // DT_STRSZ
  if (Out<ELFT>::GnuHashTab)
    ++NumEntries; // DT_GNU_HASH
  if (Out<ELFT>::HashTab)
    ++NumEntries; // DT_HASH

  if (!Config->RPath.empty()) {
    ++NumEntries; // DT_RUNPATH / DT_RPATH
    Out<ELFT>::DynStrTab->add(Config->RPath);
  }

  if (!Config->SoName.empty()) {
    ++NumEntries; // DT_SONAME
    Out<ELFT>::DynStrTab->add(Config->SoName);
  }

  if (PreInitArraySec)
    NumEntries += 2;
  if (InitArraySec)
    NumEntries += 2;
  if (FiniArraySec)
    NumEntries += 2;

  for (const std::unique_ptr<SharedFile<ELFT>> &F : SymTab.getSharedFiles()) {
    if (!F->isNeeded())
      continue;
    Out<ELFT>::DynStrTab->add(F->getSoName());
    ++NumEntries;
  }

  if (Symbol *S = SymTab.getSymbols().lookup(Config->Init))
    InitSym = dyn_cast<ELFSymbolBody<ELFT>>(S->Body);
  if (Symbol *S = SymTab.getSymbols().lookup(Config->Fini))
    FiniSym = dyn_cast<ELFSymbolBody<ELFT>>(S->Body);
  if (InitSym)
    ++NumEntries; // DT_INIT
  if (FiniSym)
    ++NumEntries; // DT_FINI

  if (Config->Bsymbolic)
    DtFlags |= DF_SYMBOLIC;
  if (Config->ZNodelete)
    DtFlags1 |= DF_1_NODELETE;
  if (Config->ZNow) {
    DtFlags |= DF_BIND_NOW;
    DtFlags1 |= DF_1_NOW;
  }
  if (Config->ZOrigin) {
    DtFlags |= DF_ORIGIN;
    DtFlags1 |= DF_1_ORIGIN;
  }

  if (DtFlags)
    ++NumEntries; // DT_FLAGS
  if (DtFlags1)
    ++NumEntries; // DT_FLAGS_1
  ++NumEntries; // DT_NULL

  Header.sh_size = NumEntries * Header.sh_entsize;
}

template <class ELFT> void DynamicSection<ELFT>::writeTo(uint8_t *Buf) {
  auto *P = reinterpret_cast<Elf_Dyn *>(Buf);

  auto WritePtr = [&](int32_t Tag, uint64_t Val) {
    P->d_tag = Tag;
    P->d_un.d_ptr = Val;
    ++P;
  };

  auto WriteVal = [&](int32_t Tag, uint32_t Val) {
    P->d_tag = Tag;
    P->d_un.d_val = Val;
    ++P;
  };

  if (Out<ELFT>::RelaDyn->hasRelocs()) {
    bool IsRela = Out<ELFT>::RelaDyn->isRela();
    WritePtr(IsRela ? DT_RELA : DT_REL, Out<ELFT>::RelaDyn->getVA());
    WriteVal(IsRela ? DT_RELASZ : DT_RELSZ, Out<ELFT>::RelaDyn->getSize());
    WriteVal(IsRela ? DT_RELAENT : DT_RELENT,
             IsRela ? sizeof(Elf_Rela) : sizeof(Elf_Rel));
  }
  if (Out<ELFT>::RelaPlt && Out<ELFT>::RelaPlt->hasRelocs()) {
    WritePtr(DT_JMPREL, Out<ELFT>::RelaPlt->getVA());
    WriteVal(DT_PLTRELSZ, Out<ELFT>::RelaPlt->getSize());
    WritePtr(DT_PLTGOT, Out<ELFT>::GotPlt->getVA());
    WriteVal(DT_PLTREL, Out<ELFT>::RelaPlt->isRela() ? DT_RELA : DT_REL);
  }

  WritePtr(DT_SYMTAB, Out<ELFT>::DynSymTab->getVA());
  WritePtr(DT_SYMENT, sizeof(Elf_Sym));
  WritePtr(DT_STRTAB, Out<ELFT>::DynStrTab->getVA());
  WriteVal(DT_STRSZ, Out<ELFT>::DynStrTab->data().size());
  if (Out<ELFT>::GnuHashTab)
    WritePtr(DT_GNU_HASH, Out<ELFT>::GnuHashTab->getVA());
  if (Out<ELFT>::HashTab)
    WritePtr(DT_HASH, Out<ELFT>::HashTab->getVA());

  if (!Config->RPath.empty())

    // If --enable-new-dtags is set lld emits DT_RUNPATH
    // instead of DT_RPATH. The two tags are functionally
    // equivalent except for the following:
    // - DT_RUNPATH is searched after LD_LIBRARY_PATH, while
    // DT_RPATH is searched before.
    // - DT_RUNPATH is used only to search for direct
    // dependencies of the object it's contained in, while
    // DT_RPATH is used for indirect dependencies as well.
    WriteVal(Config->EnableNewDtags ? DT_RUNPATH : DT_RPATH,
             Out<ELFT>::DynStrTab->getFileOff(Config->RPath));

  if (!Config->SoName.empty())
    WriteVal(DT_SONAME, Out<ELFT>::DynStrTab->getFileOff(Config->SoName));

  auto WriteArray = [&](int32_t T1, int32_t T2,
                        const OutputSectionBase<ELFT> *Sec) {
    if (!Sec)
      return;
    WritePtr(T1, Sec->getVA());
    WriteVal(T2, Sec->getSize());
  };
  WriteArray(DT_PREINIT_ARRAY, DT_PREINIT_ARRAYSZ, PreInitArraySec);
  WriteArray(DT_INIT_ARRAY, DT_INIT_ARRAYSZ, InitArraySec);
  WriteArray(DT_FINI_ARRAY, DT_FINI_ARRAYSZ, FiniArraySec);

  for (const std::unique_ptr<SharedFile<ELFT>> &F : SymTab.getSharedFiles())
    if (F->isNeeded())
      WriteVal(DT_NEEDED, Out<ELFT>::DynStrTab->getFileOff(F->getSoName()));

  if (InitSym)
    WritePtr(DT_INIT, getSymVA<ELFT>(*InitSym));
  if (FiniSym)
    WritePtr(DT_FINI, getSymVA<ELFT>(*FiniSym));
  if (DtFlags)
    WriteVal(DT_FLAGS, DtFlags);
  if (DtFlags1)
    WriteVal(DT_FLAGS_1, DtFlags1);
  WriteVal(DT_NULL, 0);
}

template <class ELFT>
OutputSection<ELFT>::OutputSection(StringRef Name, uint32_t sh_type,
                                   uintX_t sh_flags)
    : OutputSectionBase<ELFT>(Name, sh_type, sh_flags) {}

template <class ELFT>
void OutputSection<ELFT>::addSection(InputSection<ELFT> *C) {
  Sections.push_back(C);
  C->OutSec = this;
  uint32_t Align = C->getAlign();
  if (Align > this->Header.sh_addralign)
    this->Header.sh_addralign = Align;

  uintX_t Off = this->Header.sh_size;
  Off = RoundUpToAlignment(Off, Align);
  C->OutSecOff = Off;
  Off += C->getSize();
  this->Header.sh_size = Off;
}

template <class ELFT>
typename ELFFile<ELFT>::uintX_t lld::elf2::getSymVA(const SymbolBody &S) {
  switch (S.kind()) {
  case SymbolBody::DefinedSyntheticKind: {
    auto &D = cast<DefinedSynthetic<ELFT>>(S);
    return D.Section.getVA() + D.Sym.st_value;
  }
  case SymbolBody::DefinedAbsoluteKind:
    return cast<DefinedAbsolute<ELFT>>(S).Sym.st_value;
  case SymbolBody::DefinedRegularKind: {
    const auto &DR = cast<DefinedRegular<ELFT>>(S);
    const InputSectionBase<ELFT> &SC = DR.Section;
    return SC.OutSec->getVA() + SC.getOffset(DR.Sym);
  }
  case SymbolBody::DefinedCommonKind:
    return Out<ELFT>::Bss->getVA() + cast<DefinedCommon<ELFT>>(S).OffsetInBSS;
  case SymbolBody::SharedKind:
  case SymbolBody::UndefinedKind:
    return 0;
  case SymbolBody::LazyKind:
    assert(S.isUsedInRegularObj() && "Lazy symbol reached writer");
    return 0;
  }
  llvm_unreachable("Invalid symbol kind");
}

// Returns a VA which a relocatin RI refers to. Used only for local symbols.
// For non-local symbols, use getSymVA instead.
template <class ELFT, bool IsRela>
typename ELFFile<ELFT>::uintX_t
lld::elf2::getLocalRelTarget(const ObjectFile<ELFT> &File,
                             const Elf_Rel_Impl<ELFT, IsRela> &RI) {
  typedef typename ELFFile<ELFT>::Elf_Sym Elf_Sym;
  typedef typename ELFFile<ELFT>::uintX_t uintX_t;

  uintX_t Addend = getAddend<ELFT>(RI);

  // PPC64 has a special relocation representing the TOC base pointer
  // that does not have a corresponding symbol.
  if (Config->EMachine == EM_PPC64 && RI.getType(false) == R_PPC64_TOC)
    return getPPC64TocBase() + Addend;

  const Elf_Sym *Sym =
      File.getObj().getRelocationSymbol(&RI, File.getSymbolTable());

  if (!Sym)
    error("Unsupported relocation without symbol");

  // According to the ELF spec reference to a local symbol from outside
  // the group are not allowed. Unfortunately .eh_frame breaks that rule
  // and must be treated specially. For now we just replace the symbol with
  // 0.
  InputSectionBase<ELFT> *Section = File.getSection(*Sym);
  if (Section == &InputSection<ELFT>::Discarded)
    return Addend;

  uintX_t VA = Section->OutSec->getVA();
  if (isa<InputSection<ELFT>>(Section))
    return VA + Section->getOffset(*Sym) + Addend;

  uintX_t Offset = Sym->st_value;
  if (Sym->getType() == STT_SECTION) {
    Offset += Addend;
    Addend = 0;
  }
  return VA + cast<MergeInputSection<ELFT>>(Section)->getOffset(Offset) +
         Addend;
}

// Returns true if a symbol can be replaced at load-time by a symbol
// with the same name defined in other ELF executable or DSO.
bool lld::elf2::canBePreempted(const SymbolBody *Body, bool NeedsGot) {
  if (!Body)
    return false;  // Body is a local symbol.
  if (Body->isShared())
    return true;

  if (Body->isUndefined()) {
    if (!Body->isWeak())
      return true;

    // This is an horrible corner case. Ideally we would like to say that any
    // undefined symbol can be preempted so that the dynamic linker has a
    // chance of finding it at runtime.
    //
    // The problem is that the code sequence used to test for weak undef
    // functions looks like
    // if (func) func()
    // If the code is -fPIC the first reference is a load from the got and
    // everything works.
    // If the code is not -fPIC there is no reasonable way to solve it:
    // * A relocation writing to the text segment will fail (it is ro).
    // * A copy relocation doesn't work for functions.
    // * The trick of using a plt entry as the address would fail here since
    //   the plt entry would have a non zero address.
    // Since we cannot do anything better, we just resolve the symbol to 0 and
    // don't produce a dynamic relocation.
    //
    // As an extra hack, assume that if we are producing a shared library the
    // user knows what he or she is doing and can handle a dynamic relocation.
    return Config->Shared || NeedsGot;
  }
  if (!Config->Shared)
    return false;
  return Body->getVisibility() == STV_DEFAULT;
}

template <class ELFT> void OutputSection<ELFT>::writeTo(uint8_t *Buf) {
  for (InputSection<ELFT> *C : Sections)
    C->writeTo(Buf);
}

template <class ELFT>
MergeOutputSection<ELFT>::MergeOutputSection(StringRef Name, uint32_t sh_type,
                                             uintX_t sh_flags)
    : OutputSectionBase<ELFT>(Name, sh_type, sh_flags) {}

template <class ELFT> void MergeOutputSection<ELFT>::writeTo(uint8_t *Buf) {
  for (const std::pair<ArrayRef<uint8_t>, uintX_t> &P : Offsets) {
    ArrayRef<uint8_t> Data = P.first;
    memcpy(Buf, Data.data(), Data.size());
    Buf += Data.size();
  }
}

template <class ELFT>
void MergeOutputSection<ELFT>::addSection(MergeInputSection<ELFT> *S) {
  S->OutSec = this;
  uint32_t Align = S->getAlign();
  if (Align > this->Header.sh_addralign)
    this->Header.sh_addralign = Align;

  uintX_t Off = this->Header.sh_size;
  ArrayRef<uint8_t> Data = S->getSectionData();
  uintX_t EntSize = S->getSectionHdr()->sh_entsize;
  if (Data.size() % EntSize)
    error("SHF_MERGE section size must be a multiple of sh_entsize");
  for (unsigned I = 0, N = Data.size(); I != N; I += EntSize) {
    auto P = Offsets.insert(std::make_pair(Data.slice(I, EntSize), Off));
    if (P.second)
      Off += EntSize;
  }
  this->Header.sh_size = Off;
}

template <class ELFT>
unsigned MergeOutputSection<ELFT>::getOffset(ArrayRef<uint8_t> Val) {
  return Offsets.find(Val)->second;
}

template <class ELFT>
StringTableSection<ELFT>::StringTableSection(StringRef Name, bool Dynamic)
    : OutputSectionBase<ELFT>(Name, llvm::ELF::SHT_STRTAB,
                              Dynamic ? (uintX_t)llvm::ELF::SHF_ALLOC : 0),
      Dynamic(Dynamic) {
  this->Header.sh_addralign = 1;
}

template <class ELFT> void StringTableSection<ELFT>::writeTo(uint8_t *Buf) {
  StringRef Data = StrTabBuilder.data();
  memcpy(Buf, Data.data(), Data.size());
}

template <class ELFT> bool lld::elf2::includeInSymtab(const SymbolBody &B) {
  if (!B.isUsedInRegularObj())
    return false;

  // Don't include synthetic symbols like __init_array_start in every output.
  if (auto *U = dyn_cast<DefinedAbsolute<ELFT>>(&B))
    if (&U->Sym == &DefinedAbsolute<ELFT>::IgnoreUndef)
      return false;

  return true;
}

bool lld::elf2::includeInDynamicSymtab(const SymbolBody &B) {
  uint8_t V = B.getVisibility();
  if (V != STV_DEFAULT && V != STV_PROTECTED)
    return false;

  if (Config->ExportDynamic || Config->Shared)
    return true;
  return B.isUsedInDynamicReloc();
}

bool lld::elf2::includeInGnuHashTable(const SymbolBody &B) {
  // Assume that includeInDynamicSymtab() is already checked.
  return !B.isUndefined();
}

template <class ELFT>
bool lld::elf2::shouldKeepInSymtab(const ObjectFile<ELFT> &File,
                                   StringRef SymName,
                                   const typename ELFFile<ELFT>::Elf_Sym &Sym) {
  if (Sym.getType() == STT_SECTION)
    return false;

  // If sym references a section in a discarded group, don't keep it.
  if (File.getSection(Sym) == &InputSection<ELFT>::Discarded)
    return false;

  if (Config->DiscardNone)
    return true;

  // ELF defines dynamic locals as symbols which name starts with ".L".
  return !(Config->DiscardLocals && SymName.startswith(".L"));
}

template <class ELFT>
SymbolTableSection<ELFT>::SymbolData::SymbolData(SymbolBody *Body,
                                                 bool HasGnuHash)
    : Body(Body), HasGnuHash(HasGnuHash),
      GnuHash(HasGnuHash ? hashGnu(Body->getName()) : 0) {}

template <class ELFT>
SymbolTableSection<ELFT>::SymbolTableSection(
    SymbolTable<ELFT> &Table, StringTableSection<ELFT> &StrTabSec)
    : OutputSectionBase<ELFT>(
          StrTabSec.isDynamic() ? ".dynsym" : ".symtab",
          StrTabSec.isDynamic() ? llvm::ELF::SHT_DYNSYM : llvm::ELF::SHT_SYMTAB,
          StrTabSec.isDynamic() ? (uintX_t)llvm::ELF::SHF_ALLOC : 0),
      Table(Table), StrTabSec(StrTabSec) {
  typedef OutputSectionBase<ELFT> Base;
  typename Base::Elf_Shdr &Header = this->Header;

  Header.sh_entsize = sizeof(Elf_Sym);
  Header.sh_addralign = ELFT::Is64Bits ? 8 : 4;
}

template <class ELFT> void SymbolTableSection<ELFT>::finalize() {
  this->Header.sh_size = getNumSymbols() * sizeof(Elf_Sym);
  this->Header.sh_link = StrTabSec.SectionIndex;
  this->Header.sh_info = NumLocals + 1;

  if (!StrTabSec.isDynamic()) {
    std::stable_sort(Symbols.begin(), Symbols.end(),
                     [](const SymbolData &L, const SymbolData &R) {
                       return getSymbolBinding(L.Body) == STB_LOCAL &&
                              getSymbolBinding(R.Body) != STB_LOCAL;
                     });
    return;
  }
  if (NumGnuHashed) {
    unsigned NBuckets = GnuHashTableSection<ELFT>::calcNBuckets(NumGnuHashed);
    std::stable_sort(Symbols.begin(), Symbols.end(),
                     [NBuckets](const SymbolData &L, const SymbolData &R) {
                       if (!L.HasGnuHash || !R.HasGnuHash)
                         return R.HasGnuHash;
                       return L.GnuHash % NBuckets < R.GnuHash % NBuckets;
                     });
  }
  size_t I = 0;
  for (const SymbolData &Item : Symbols)
    Item.Body->setDynamicSymbolTableIndex(++I);
}

template <class ELFT>
void SymbolTableSection<ELFT>::addLocalSymbol(StringRef Name) {
  StrTabSec.add(Name);
  ++NumVisible;
  ++NumLocals;
}

template <class ELFT>
void SymbolTableSection<ELFT>::addSymbol(SymbolBody *Body) {
  StrTabSec.add(Body->getName());
  const bool HasGnuHash = StrTabSec.isDynamic() && Out<ELFT>::GnuHashTab &&
                          includeInGnuHashTable(*Body);
  Symbols.emplace_back(Body, HasGnuHash);
  ++NumVisible;
  if (HasGnuHash)
    ++NumGnuHashed;
}

template <class ELFT> void SymbolTableSection<ELFT>::writeTo(uint8_t *Buf) {
  Buf += sizeof(Elf_Sym);

  // All symbols with STB_LOCAL binding precede the weak and global symbols.
  // .dynsym only contains global symbols.
  if (!Config->DiscardAll && !StrTabSec.isDynamic())
    writeLocalSymbols(Buf);

  writeGlobalSymbols(Buf);
}

template <class ELFT>
void SymbolTableSection<ELFT>::writeLocalSymbols(uint8_t *&Buf) {
  // Iterate over all input object files to copy their local symbols
  // to the output symbol table pointed by Buf.
  for (const std::unique_ptr<ObjectFile<ELFT>> &File : Table.getObjectFiles()) {
    Elf_Sym_Range Syms = File->getLocalSymbols();
    for (const Elf_Sym &Sym : Syms) {
      ErrorOr<StringRef> SymNameOrErr = Sym.getName(File->getStringTable());
      error(SymNameOrErr);
      StringRef SymName = *SymNameOrErr;
      if (!shouldKeepInSymtab<ELFT>(*File, SymName, Sym))
        continue;

      auto *ESym = reinterpret_cast<Elf_Sym *>(Buf);
      uintX_t VA = 0;
      if (Sym.st_shndx == SHN_ABS) {
        ESym->st_shndx = SHN_ABS;
        VA = Sym.st_value;
      } else {
        const InputSectionBase<ELFT> *Section = File->getSection(Sym);
        if (!Section->isLive())
          continue;
        const OutputSectionBase<ELFT> *OutSec = Section->OutSec;
        ESym->st_shndx = OutSec->SectionIndex;
        VA += OutSec->getVA() + Section->getOffset(Sym);
      }
      ESym->st_name = StrTabSec.getFileOff(SymName);
      ESym->st_size = Sym.st_size;
      ESym->setBindingAndType(Sym.getBinding(), Sym.getType());
      ESym->st_value = VA;
      Buf += sizeof(*ESym);
    }
  }
}

template <class ELFT>
void SymbolTableSection<ELFT>::writeGlobalSymbols(uint8_t *Buf) {
  // Write the internal symbol table contents to the output symbol table
  // pointed by Buf.
  auto *ESym = reinterpret_cast<Elf_Sym *>(Buf);
  for (const SymbolData &Item : Symbols) {
    SymbolBody *Body = Item.Body;
    const OutputSectionBase<ELFT> *OutSec = nullptr;

    switch (Body->kind()) {
    case SymbolBody::DefinedSyntheticKind:
      OutSec = &cast<DefinedSynthetic<ELFT>>(Body)->Section;
      break;
    case SymbolBody::DefinedRegularKind: {
      auto *Sym = cast<DefinedRegular<ELFT>>(Body->repl());
      if (!Sym->Section.isLive())
        continue;
      OutSec = Sym->Section.OutSec;
      break;
    }
    case SymbolBody::DefinedCommonKind:
      OutSec = Out<ELFT>::Bss;
      break;
    case SymbolBody::UndefinedKind:
    case SymbolBody::DefinedAbsoluteKind:
    case SymbolBody::SharedKind:
    case SymbolBody::LazyKind:
      break;
    }

    StringRef Name = Body->getName();
    ESym->st_name = StrTabSec.getFileOff(Name);

    unsigned char Type = STT_NOTYPE;
    uintX_t Size = 0;
    if (const auto *EBody = dyn_cast<ELFSymbolBody<ELFT>>(Body)) {
      const Elf_Sym &InputSym = EBody->Sym;
      Type = InputSym.getType();
      Size = InputSym.st_size;
    }

    ESym->setBindingAndType(getSymbolBinding(Body), Type);
    ESym->st_size = Size;
    ESym->setVisibility(Body->getVisibility());
    ESym->st_value = getSymVA<ELFT>(*Body);

    if (isa<DefinedAbsolute<ELFT>>(Body))
      ESym->st_shndx = SHN_ABS;
    else if (OutSec)
      ESym->st_shndx = OutSec->SectionIndex;

    ++ESym;
  }
}

template <class ELFT>
uint8_t SymbolTableSection<ELFT>::getSymbolBinding(SymbolBody *Body) {
  uint8_t Visibility = Body->getVisibility();
  if (Visibility != STV_DEFAULT && Visibility != STV_PROTECTED)
    return STB_LOCAL;
  if (const auto *EBody = dyn_cast<ELFSymbolBody<ELFT>>(Body))
    return EBody->Sym.getBinding();
  return Body->isWeak() ? STB_WEAK : STB_GLOBAL;
}

template <class ELFT>
ArrayRef<typename SymbolTableSection<ELFT>::SymbolData>
SymbolTableSection<ELFT>::getGnuHashSymbols() const {
  return getSymbols().slice(Symbols.size() - NumGnuHashed);
}

namespace lld {
namespace elf2 {
template class OutputSectionBase<ELF32LE>;
template class OutputSectionBase<ELF32BE>;
template class OutputSectionBase<ELF64LE>;
template class OutputSectionBase<ELF64BE>;

template class GotPltSection<ELF32LE>;
template class GotPltSection<ELF32BE>;
template class GotPltSection<ELF64LE>;
template class GotPltSection<ELF64BE>;

template class GotSection<ELF32LE>;
template class GotSection<ELF32BE>;
template class GotSection<ELF64LE>;
template class GotSection<ELF64BE>;

template class PltSection<ELF32LE>;
template class PltSection<ELF32BE>;
template class PltSection<ELF64LE>;
template class PltSection<ELF64BE>;

template class RelocationSection<ELF32LE>;
template class RelocationSection<ELF32BE>;
template class RelocationSection<ELF64LE>;
template class RelocationSection<ELF64BE>;

template class InterpSection<ELF32LE>;
template class InterpSection<ELF32BE>;
template class InterpSection<ELF64LE>;
template class InterpSection<ELF64BE>;

template class GnuHashTableSection<ELF32LE>;
template class GnuHashTableSection<ELF32BE>;
template class GnuHashTableSection<ELF64LE>;
template class GnuHashTableSection<ELF64BE>;

template class HashTableSection<ELF32LE>;
template class HashTableSection<ELF32BE>;
template class HashTableSection<ELF64LE>;
template class HashTableSection<ELF64BE>;

template class DynamicSection<ELF32LE>;
template class DynamicSection<ELF32BE>;
template class DynamicSection<ELF64LE>;
template class DynamicSection<ELF64BE>;

template class OutputSection<ELF32LE>;
template class OutputSection<ELF32BE>;
template class OutputSection<ELF64LE>;
template class OutputSection<ELF64BE>;

template class MergeOutputSection<ELF32LE>;
template class MergeOutputSection<ELF32BE>;
template class MergeOutputSection<ELF64LE>;
template class MergeOutputSection<ELF64BE>;

template class StringTableSection<ELF32LE>;
template class StringTableSection<ELF32BE>;
template class StringTableSection<ELF64LE>;
template class StringTableSection<ELF64BE>;

template class SymbolTableSection<ELF32LE>;
template class SymbolTableSection<ELF32BE>;
template class SymbolTableSection<ELF64LE>;
template class SymbolTableSection<ELF64BE>;

template ELFFile<ELF32LE>::uintX_t getSymVA<ELF32LE>(const SymbolBody &);
template ELFFile<ELF32BE>::uintX_t getSymVA<ELF32BE>(const SymbolBody &);
template ELFFile<ELF64LE>::uintX_t getSymVA<ELF64LE>(const SymbolBody &);
template ELFFile<ELF64BE>::uintX_t getSymVA<ELF64BE>(const SymbolBody &);

template ELFFile<ELF32LE>::uintX_t
getLocalRelTarget(const ObjectFile<ELF32LE> &,
                  const ELFFile<ELF32LE>::Elf_Rel &);
template ELFFile<ELF32BE>::uintX_t
getLocalRelTarget(const ObjectFile<ELF32BE> &,
                  const ELFFile<ELF32BE>::Elf_Rel &);
template ELFFile<ELF64LE>::uintX_t
getLocalRelTarget(const ObjectFile<ELF64LE> &,
                  const ELFFile<ELF64LE>::Elf_Rel &);
template ELFFile<ELF64BE>::uintX_t
getLocalRelTarget(const ObjectFile<ELF64BE> &,
                  const ELFFile<ELF64BE>::Elf_Rel &);

template ELFFile<ELF32LE>::uintX_t
getLocalRelTarget(const ObjectFile<ELF32LE> &,
                  const ELFFile<ELF32LE>::Elf_Rela &);
template ELFFile<ELF32BE>::uintX_t
getLocalRelTarget(const ObjectFile<ELF32BE> &,
                  const ELFFile<ELF32BE>::Elf_Rela &);
template ELFFile<ELF64LE>::uintX_t
getLocalRelTarget(const ObjectFile<ELF64LE> &,
                  const ELFFile<ELF64LE>::Elf_Rela &);
template ELFFile<ELF64BE>::uintX_t
getLocalRelTarget(const ObjectFile<ELF64BE> &,
                  const ELFFile<ELF64BE>::Elf_Rela &);

template bool includeInSymtab<ELF32LE>(const SymbolBody &);
template bool includeInSymtab<ELF32BE>(const SymbolBody &);
template bool includeInSymtab<ELF64LE>(const SymbolBody &);
template bool includeInSymtab<ELF64BE>(const SymbolBody &);

template bool shouldKeepInSymtab<ELF32LE>(const ObjectFile<ELF32LE> &,
                                          StringRef,
                                          const ELFFile<ELF32LE>::Elf_Sym &);
template bool shouldKeepInSymtab<ELF32BE>(const ObjectFile<ELF32BE> &,
                                          StringRef,
                                          const ELFFile<ELF32BE>::Elf_Sym &);
template bool shouldKeepInSymtab<ELF64LE>(const ObjectFile<ELF64LE> &,
                                          StringRef,
                                          const ELFFile<ELF64LE>::Elf_Sym &);
template bool shouldKeepInSymtab<ELF64BE>(const ObjectFile<ELF64BE> &,
                                          StringRef,
                                          const ELFFile<ELF64BE>::Elf_Sym &);
}
}
