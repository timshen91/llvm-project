//===--------- lib/ReaderWriter/ELF/ARM/ARMTargetHandler.cpp --------------===//
//
//                             The LLVM Linker
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Atoms.h"
#include "ARMExecutableWriter.h"
#include "ARMTargetHandler.h"
#include "ARMLinkingContext.h"

using namespace lld;
using namespace elf;

ARMTargetHandler::ARMTargetHandler(ARMLinkingContext &ctx)
    : _ctx(ctx), _armTargetLayout(new ARMTargetLayout<ARMELFType>(ctx)),
      _armRelocationHandler(
          new ARMTargetRelocationHandler(*_armTargetLayout.get())) {}

void ARMTargetHandler::registerRelocationNames(Registry &registry) {
  registry.addKindTable(Reference::KindNamespace::ELF, Reference::KindArch::ARM,
                        kindStrings);
}

std::unique_ptr<Writer> ARMTargetHandler::getWriter() {
  switch (this->_ctx.getOutputELFType()) {
  case llvm::ELF::ET_EXEC:
    return llvm::make_unique<ARMExecutableWriter<ARMELFType>>(
        _ctx, *_armTargetLayout.get());
  default:
    llvm_unreachable("unsupported output type");
  }
}

#define ELF_RELOC(name, value) LLD_KIND_STRING_ENTRY(name),

const Registry::KindStrings ARMTargetHandler::kindStrings[] = {
#include "llvm/Support/ELFRelocs/ARM.def"
    LLD_KIND_STRING_END
};
