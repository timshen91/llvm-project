//===-- llvm-lto: a simple command-line program to link modules with LTO --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This program takes in a list of bitcode files, links them, performs link-time
// optimization, and outputs an object file.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/StringSet.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/CodeGen/CommandFlags.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/LTO/LTOCodeGenerator.h"
#include "llvm/LTO/LTOModule.h"
#include "llvm/LTO/ThinLTOCodeGenerator.h"
#include "llvm/Object/ModuleSummaryIndexObjectFile.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"
#include <list>

using namespace llvm;

static cl::opt<char>
    OptLevel("O", cl::desc("Optimization level. [-O0, -O1, -O2, or -O3] "
                           "(default = '-O2')"),
             cl::Prefix, cl::ZeroOrMore, cl::init('2'));

static cl::opt<bool> DisableVerify(
    "disable-verify", cl::init(false),
    cl::desc("Do not run the verifier during the optimization pipeline"));

static cl::opt<bool> DisableInline("disable-inlining", cl::init(false),
                                   cl::desc("Do not run the inliner pass"));

static cl::opt<bool>
    DisableGVNLoadPRE("disable-gvn-loadpre", cl::init(false),
                      cl::desc("Do not run the GVN load PRE pass"));

static cl::opt<bool> DisableLTOVectorization(
    "disable-lto-vectorization", cl::init(false),
    cl::desc("Do not run loop or slp vectorization during LTO"));

static cl::opt<bool> UseDiagnosticHandler(
    "use-diagnostic-handler", cl::init(false),
    cl::desc("Use a diagnostic handler to test the handler interface"));

static cl::opt<bool>
    ThinLTO("thinlto", cl::init(false),
            cl::desc("Only write combined global index for ThinLTO backends"));

enum ThinLTOModes {
  THINLINK,
  THINDISTRIBUTE,
  THINEMITIMPORTS,
  THINPROMOTE,
  THINIMPORT,
  THININTERNALIZE,
  THINOPT,
  THINCODEGEN,
  THINALL
};

cl::opt<ThinLTOModes> ThinLTOMode(
    "thinlto-action", cl::desc("Perform a single ThinLTO stage:"),
    cl::values(
        clEnumValN(
            THINLINK, "thinlink",
            "ThinLink: produces the index by linking only the summaries."),
        clEnumValN(THINDISTRIBUTE, "distributedindexes",
                   "Produces individual indexes for distributed backends."),
        clEnumValN(THINEMITIMPORTS, "emitimports",
                   "Emit imports files for distributed backends."),
        clEnumValN(THINPROMOTE, "promote",
                   "Perform pre-import promotion (requires -thinlto-index)."),
        clEnumValN(THINIMPORT, "import", "Perform both promotion and "
                                         "cross-module importing (requires "
                                         "-thinlto-index)."),
        clEnumValN(THININTERNALIZE, "internalize",
                   "Perform internalization driven by -exported-symbol "
                   "(requires -thinlto-index)."),
        clEnumValN(THINOPT, "optimize", "Perform ThinLTO optimizations."),
        clEnumValN(THINCODEGEN, "codegen", "CodeGen (expected to match llc)"),
        clEnumValN(THINALL, "run", "Perform ThinLTO end-to-end"),
        clEnumValEnd));

static cl::opt<std::string>
    ThinLTOIndex("thinlto-index",
                 cl::desc("Provide the index produced by a ThinLink, required "
                          "to perform the promotion and/or importing."));

static cl::opt<std::string> ThinLTOModuleId(
    "thinlto-module-id",
    cl::desc("For the module ID for the file to process, useful to "
             "match what is in the index."));

static cl::opt<bool>
    SaveModuleFile("save-merged-module", cl::init(false),
                   cl::desc("Write merged LTO module to file before CodeGen"));

static cl::list<std::string> InputFilenames(cl::Positional, cl::OneOrMore,
                                            cl::desc("<input bitcode files>"));

static cl::opt<std::string> OutputFilename("o", cl::init(""),
                                           cl::desc("Override output filename"),
                                           cl::value_desc("filename"));

static cl::list<std::string> ExportedSymbols(
    "exported-symbol",
    cl::desc("List of symbols to export from the resulting object file"),
    cl::ZeroOrMore);

static cl::list<std::string>
    DSOSymbols("dso-symbol",
               cl::desc("Symbol to put in the symtab in the resulting dso"),
               cl::ZeroOrMore);

static cl::opt<bool> ListSymbolsOnly(
    "list-symbols-only", cl::init(false),
    cl::desc("Instead of running LTO, list the symbols in each IR file"));

static cl::opt<bool> SetMergedModule(
    "set-merged-module", cl::init(false),
    cl::desc("Use the first input module as the merged module"));

static cl::opt<unsigned> Parallelism("j", cl::Prefix, cl::init(1),
                                     cl::desc("Number of backend threads"));

static cl::opt<bool> RestoreGlobalsLinkage(
    "restore-linkage", cl::init(false),
    cl::desc("Restore original linkage of globals prior to CodeGen"));

namespace {
struct ModuleInfo {
  std::vector<bool> CanBeHidden;
};
}

static void handleDiagnostics(lto_codegen_diagnostic_severity_t Severity,
                              const char *Msg, void *) {
  errs() << "llvm-lto: ";
  switch (Severity) {
  case LTO_DS_NOTE:
    errs() << "note: ";
    break;
  case LTO_DS_REMARK:
    errs() << "remark: ";
    break;
  case LTO_DS_ERROR:
    errs() << "error: ";
    break;
  case LTO_DS_WARNING:
    errs() << "warning: ";
    break;
  }
  errs() << Msg << "\n";
}

static std::string CurrentActivity;
static void diagnosticHandler(const DiagnosticInfo &DI) {
  raw_ostream &OS = errs();
  OS << "llvm-lto: ";
  switch (DI.getSeverity()) {
  case DS_Error:
    OS << "error";
    break;
  case DS_Warning:
    OS << "warning";
    break;
  case DS_Remark:
    OS << "remark";
    break;
  case DS_Note:
    OS << "note";
    break;
  }
  if (!CurrentActivity.empty())
    OS << ' ' << CurrentActivity;
  OS << ": ";

  DiagnosticPrinterRawOStream DP(OS);
  DI.print(DP);
  OS << '\n';

  if (DI.getSeverity() == DS_Error)
    exit(1);
}

static void diagnosticHandlerWithContext(const DiagnosticInfo &DI,
                                         void *Context) {
  diagnosticHandler(DI);
}

static void error(const Twine &Msg) {
  errs() << "llvm-lto: " << Msg << '\n';
  exit(1);
}

static void error(std::error_code EC, const Twine &Prefix) {
  if (EC)
    error(Prefix + ": " + EC.message());
}

template <typename T>
static void error(const ErrorOr<T> &V, const Twine &Prefix) {
  error(V.getError(), Prefix);
}

static void maybeVerifyModule(const Module &Mod) {
  if (!DisableVerify && verifyModule(Mod))
    error("Broken Module");
}

static std::unique_ptr<LTOModule>
getLocalLTOModule(StringRef Path, std::unique_ptr<MemoryBuffer> &Buffer,
                  const TargetOptions &Options) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> BufferOrErr =
      MemoryBuffer::getFile(Path);
  error(BufferOrErr, "error loading file '" + Path + "'");
  Buffer = std::move(BufferOrErr.get());
  CurrentActivity = ("loading file '" + Path + "'").str();
  std::unique_ptr<LLVMContext> Context = llvm::make_unique<LLVMContext>();
  Context->setDiagnosticHandler(diagnosticHandlerWithContext, nullptr, true);
  ErrorOr<std::unique_ptr<LTOModule>> Ret = LTOModule::createInLocalContext(
      std::move(Context), Buffer->getBufferStart(), Buffer->getBufferSize(),
      Options, Path);
  CurrentActivity = "";
  maybeVerifyModule((*Ret)->getModule());
  return std::move(*Ret);
}

/// \brief List symbols in each IR file.
///
/// The main point here is to provide lit-testable coverage for the LTOModule
/// functionality that's exposed by the C API to list symbols.  Moreover, this
/// provides testing coverage for modules that have been created in their own
/// contexts.
static void listSymbols(const TargetOptions &Options) {
  for (auto &Filename : InputFilenames) {
    std::unique_ptr<MemoryBuffer> Buffer;
    std::unique_ptr<LTOModule> Module =
        getLocalLTOModule(Filename, Buffer, Options);

    // List the symbols.
    outs() << Filename << ":\n";
    for (int I = 0, E = Module->getSymbolCount(); I != E; ++I)
      outs() << Module->getSymbolName(I) << "\n";
  }
}

/// Create a combined index file from the input IR files and write it.
///
/// This is meant to enable testing of ThinLTO combined index generation,
/// currently available via the gold plugin via -thinlto.
static void createCombinedModuleSummaryIndex() {
  ModuleSummaryIndex CombinedIndex;
  uint64_t NextModuleId = 0;
  for (auto &Filename : InputFilenames) {
    CurrentActivity = "loading file '" + Filename + "'";
    ErrorOr<std::unique_ptr<ModuleSummaryIndex>> IndexOrErr =
        llvm::getModuleSummaryIndexForFile(Filename, diagnosticHandler);
    error(IndexOrErr, "error " + CurrentActivity);
    std::unique_ptr<ModuleSummaryIndex> Index = std::move(IndexOrErr.get());
    CurrentActivity = "";
    // Skip files without a module summary.
    if (!Index)
      continue;
    CombinedIndex.mergeFrom(std::move(Index), ++NextModuleId);
  }
  std::error_code EC;
  assert(!OutputFilename.empty());
  raw_fd_ostream OS(OutputFilename + ".thinlto.bc", EC,
                    sys::fs::OpenFlags::F_None);
  error(EC, "error opening the file '" + OutputFilename + ".thinlto.bc'");
  WriteIndexToFile(CombinedIndex, OS);
  OS.close();
}

namespace thinlto {

std::vector<std::unique_ptr<MemoryBuffer>>
loadAllFilesForIndex(const ModuleSummaryIndex &Index) {
  std::vector<std::unique_ptr<MemoryBuffer>> InputBuffers;

  for (auto &ModPath : Index.modulePaths()) {
    const auto &Filename = ModPath.first();
    auto CurrentActivity = "loading file '" + Filename + "'";
    auto InputOrErr = MemoryBuffer::getFile(Filename);
    error(InputOrErr, "error " + CurrentActivity);
    InputBuffers.push_back(std::move(*InputOrErr));
  }
  return InputBuffers;
}

std::unique_ptr<ModuleSummaryIndex> loadCombinedIndex() {
  if (ThinLTOIndex.empty())
    report_fatal_error("Missing -thinlto-index for ThinLTO promotion stage");
  auto CurrentActivity = "loading file '" + ThinLTOIndex + "'";
  ErrorOr<std::unique_ptr<ModuleSummaryIndex>> IndexOrErr =
      llvm::getModuleSummaryIndexForFile(ThinLTOIndex, diagnosticHandler);
  error(IndexOrErr, "error " + CurrentActivity);
  return std::move(IndexOrErr.get());
}

static std::unique_ptr<Module> loadModule(StringRef Filename,
                                          LLVMContext &Ctx) {
  SMDiagnostic Err;
  std::unique_ptr<Module> M(parseIRFile(Filename, Err, Ctx));
  if (!M) {
    Err.print("llvm-lto", errs());
    report_fatal_error("Can't load module for file " + Filename);
  }
  maybeVerifyModule(*M);

  if (ThinLTOModuleId.getNumOccurrences()) {
    if (InputFilenames.size() != 1)
      report_fatal_error("Can't override the module id for multiple files");
    M->setModuleIdentifier(ThinLTOModuleId);
  }
  return M;
}

static void writeModuleToFile(Module &TheModule, StringRef Filename) {
  std::error_code EC;
  raw_fd_ostream OS(Filename, EC, sys::fs::OpenFlags::F_None);
  error(EC, "error opening the file '" + Filename + "'");
  maybeVerifyModule(TheModule);
  WriteBitcodeToFile(&TheModule, OS, /* ShouldPreserveUseListOrder */ true);
}

class ThinLTOProcessing {
public:
  ThinLTOCodeGenerator ThinGenerator;

  ThinLTOProcessing(const TargetOptions &Options) {
    ThinGenerator.setCodePICModel(RelocModel);
    ThinGenerator.setTargetOptions(Options);

    // Add all the exported symbols to the table of symbols to preserve.
    for (unsigned i = 0; i < ExportedSymbols.size(); ++i)
      ThinGenerator.preserveSymbol(ExportedSymbols[i]);
  }

  void run() {
    switch (ThinLTOMode) {
    case THINLINK:
      return thinLink();
    case THINDISTRIBUTE:
      return distributedIndexes();
    case THINEMITIMPORTS:
      return emitImports();
    case THINPROMOTE:
      return promote();
    case THINIMPORT:
      return import();
    case THININTERNALIZE:
      return internalize();
    case THINOPT:
      return optimize();
    case THINCODEGEN:
      return codegen();
    case THINALL:
      return runAll();
    }
  }

private:
  /// Load the input files, create the combined index, and write it out.
  void thinLink() {
    // Perform "ThinLink": just produce the index
    if (OutputFilename.empty())
      report_fatal_error(
          "OutputFilename is necessary to store the combined index.\n");

    LLVMContext Ctx;
    std::vector<std::unique_ptr<MemoryBuffer>> InputBuffers;
    for (unsigned i = 0; i < InputFilenames.size(); ++i) {
      auto &Filename = InputFilenames[i];
      StringRef CurrentActivity = "loading file '" + Filename + "'";
      auto InputOrErr = MemoryBuffer::getFile(Filename);
      error(InputOrErr, "error " + CurrentActivity);
      InputBuffers.push_back(std::move(*InputOrErr));
      ThinGenerator.addModule(Filename, InputBuffers.back()->getBuffer());
    }

    auto CombinedIndex = ThinGenerator.linkCombinedIndex();
    std::error_code EC;
    raw_fd_ostream OS(OutputFilename, EC, sys::fs::OpenFlags::F_None);
    error(EC, "error opening the file '" + OutputFilename + "'");
    WriteIndexToFile(*CombinedIndex, OS);
    return;
  }

  /// Load the combined index from disk, then compute and generate
  /// individual index files suitable for ThinLTO distributed backend builds
  /// on the files mentioned on the command line (these must match the index
  /// content).
  void distributedIndexes() {
    if (InputFilenames.size() != 1 && !OutputFilename.empty())
      report_fatal_error("Can't handle a single output filename and multiple "
                         "input files, do not provide an output filename and "
                         "the output files will be suffixed from the input "
                         "ones.");

    auto Index = loadCombinedIndex();
    for (auto &Filename : InputFilenames) {
      // Build a map of module to the GUIDs and summary objects that should
      // be written to its index.
      std::map<std::string, GVSummaryMapTy> ModuleToSummariesForIndex;
      ThinLTOCodeGenerator::gatherImportedSummariesForModule(
          Filename, *Index, ModuleToSummariesForIndex);

      std::string OutputName = OutputFilename;
      if (OutputName.empty()) {
        OutputName = Filename + ".thinlto.bc";
      }
      std::error_code EC;
      raw_fd_ostream OS(OutputName, EC, sys::fs::OpenFlags::F_None);
      error(EC, "error opening the file '" + OutputName + "'");
      WriteIndexToFile(*Index, OS, &ModuleToSummariesForIndex);
    }
  }

  /// Load the combined index from disk, compute the imports, and emit
  /// the import file lists for each module to disk.
  void emitImports() {
    if (InputFilenames.size() != 1 && !OutputFilename.empty())
      report_fatal_error("Can't handle a single output filename and multiple "
                         "input files, do not provide an output filename and "
                         "the output files will be suffixed from the input "
                         "ones.");

    auto Index = loadCombinedIndex();
    for (auto &Filename : InputFilenames) {
      std::string OutputName = OutputFilename;
      if (OutputName.empty()) {
        OutputName = Filename + ".imports";
      }
      ThinLTOCodeGenerator::emitImports(Filename, OutputName, *Index);
    }
  }

  /// Load the combined index from disk, then load every file referenced by
  /// the index and add them to the generator, finally perform the promotion
  /// on the files mentioned on the command line (these must match the index
  /// content).
  void promote() {
    if (InputFilenames.size() != 1 && !OutputFilename.empty())
      report_fatal_error("Can't handle a single output filename and multiple "
                         "input files, do not provide an output filename and "
                         "the output files will be suffixed from the input "
                         "ones.");

    auto Index = loadCombinedIndex();
    for (auto &Filename : InputFilenames) {
      LLVMContext Ctx;
      auto TheModule = loadModule(Filename, Ctx);

      ThinGenerator.promote(*TheModule, *Index);

      std::string OutputName = OutputFilename;
      if (OutputName.empty()) {
        OutputName = Filename + ".thinlto.promoted.bc";
      }
      writeModuleToFile(*TheModule, OutputName);
    }
  }

  /// Load the combined index from disk, then load every file referenced by
  /// the index and add them to the generator, then performs the promotion and
  /// cross module importing on the files mentioned on the command line
  /// (these must match the index content).
  void import() {
    if (InputFilenames.size() != 1 && !OutputFilename.empty())
      report_fatal_error("Can't handle a single output filename and multiple "
                         "input files, do not provide an output filename and "
                         "the output files will be suffixed from the input "
                         "ones.");

    auto Index = loadCombinedIndex();
    auto InputBuffers = loadAllFilesForIndex(*Index);
    for (auto &MemBuffer : InputBuffers)
      ThinGenerator.addModule(MemBuffer->getBufferIdentifier(),
                              MemBuffer->getBuffer());

    for (auto &Filename : InputFilenames) {
      LLVMContext Ctx;
      auto TheModule = loadModule(Filename, Ctx);

      ThinGenerator.crossModuleImport(*TheModule, *Index);

      std::string OutputName = OutputFilename;
      if (OutputName.empty()) {
        OutputName = Filename + ".thinlto.imported.bc";
      }
      writeModuleToFile(*TheModule, OutputName);
    }
  }

  void internalize() {
    if (InputFilenames.size() != 1 && !OutputFilename.empty())
      report_fatal_error("Can't handle a single output filename and multiple "
                         "input files, do not provide an output filename and "
                         "the output files will be suffixed from the input "
                         "ones.");

    if (ExportedSymbols.empty())
      errs() << "Warning: -internalize will not perform without "
                "-exported-symbol\n";

    auto Index = loadCombinedIndex();
    auto InputBuffers = loadAllFilesForIndex(*Index);
    for (auto &MemBuffer : InputBuffers)
      ThinGenerator.addModule(MemBuffer->getBufferIdentifier(),
                              MemBuffer->getBuffer());

    for (auto &Filename : InputFilenames) {
      LLVMContext Ctx;
      auto TheModule = loadModule(Filename, Ctx);

      ThinGenerator.internalize(*TheModule, *Index);

      std::string OutputName = OutputFilename;
      if (OutputName.empty()) {
        OutputName = Filename + ".thinlto.internalized.bc";
      }
      writeModuleToFile(*TheModule, OutputName);
    }
  }

  void optimize() {
    if (InputFilenames.size() != 1 && !OutputFilename.empty())
      report_fatal_error("Can't handle a single output filename and multiple "
                         "input files, do not provide an output filename and "
                         "the output files will be suffixed from the input "
                         "ones.");
    if (!ThinLTOIndex.empty())
      errs() << "Warning: -thinlto-index ignored for optimize stage";

    for (auto &Filename : InputFilenames) {
      LLVMContext Ctx;
      auto TheModule = loadModule(Filename, Ctx);

      ThinGenerator.optimize(*TheModule);

      std::string OutputName = OutputFilename;
      if (OutputName.empty()) {
        OutputName = Filename + ".thinlto.imported.bc";
      }
      writeModuleToFile(*TheModule, OutputName);
    }
  }

  void codegen() {
    if (InputFilenames.size() != 1 && !OutputFilename.empty())
      report_fatal_error("Can't handle a single output filename and multiple "
                         "input files, do not provide an output filename and "
                         "the output files will be suffixed from the input "
                         "ones.");
    if (!ThinLTOIndex.empty())
      errs() << "Warning: -thinlto-index ignored for codegen stage";

    for (auto &Filename : InputFilenames) {
      LLVMContext Ctx;
      auto TheModule = loadModule(Filename, Ctx);

      auto Buffer = ThinGenerator.codegen(*TheModule);
      std::string OutputName = OutputFilename;
      if (OutputName.empty()) {
        OutputName = Filename + ".thinlto.o";
      }
      if (OutputName == "-") {
        outs() << Buffer->getBuffer();
        return;
      }

      std::error_code EC;
      raw_fd_ostream OS(OutputName, EC, sys::fs::OpenFlags::F_None);
      error(EC, "error opening the file '" + OutputName + "'");
      OS << Buffer->getBuffer();
    }
  }

  /// Full ThinLTO process
  void runAll() {
    if (!OutputFilename.empty())
      report_fatal_error("Do not provide an output filename for ThinLTO "
                         " processing, the output files will be suffixed from "
                         "the input ones.");

    if (!ThinLTOIndex.empty())
      errs() << "Warning: -thinlto-index ignored for full ThinLTO process";

    LLVMContext Ctx;
    std::vector<std::unique_ptr<MemoryBuffer>> InputBuffers;
    for (unsigned i = 0; i < InputFilenames.size(); ++i) {
      auto &Filename = InputFilenames[i];
      StringRef CurrentActivity = "loading file '" + Filename + "'";
      auto InputOrErr = MemoryBuffer::getFile(Filename);
      error(InputOrErr, "error " + CurrentActivity);
      InputBuffers.push_back(std::move(*InputOrErr));
      ThinGenerator.addModule(Filename, InputBuffers.back()->getBuffer());
    }

    ThinGenerator.run();

    auto &Binaries = ThinGenerator.getProducedBinaries();
    if (Binaries.size() != InputFilenames.size())
      report_fatal_error("Number of output objects does not match the number "
                         "of inputs");

    for (unsigned BufID = 0; BufID < Binaries.size(); ++BufID) {
      auto OutputName = InputFilenames[BufID] + ".thinlto.o";
      std::error_code EC;
      raw_fd_ostream OS(OutputName, EC, sys::fs::OpenFlags::F_None);
      error(EC, "error opening the file '" + OutputName + "'");
      OS << Binaries[BufID]->getBuffer();
    }
  }

  /// Load the combined index from disk, then load every file referenced by
};

} // namespace thinlto

int main(int argc, char **argv) {
  // Print a stack trace if we signal out.
  sys::PrintStackTraceOnErrorSignal();
  PrettyStackTraceProgram X(argc, argv);

  llvm_shutdown_obj Y; // Call llvm_shutdown() on exit.
  cl::ParseCommandLineOptions(argc, argv, "llvm LTO linker\n");

  if (OptLevel < '0' || OptLevel > '3')
    error("optimization level must be between 0 and 3");

  // Initialize the configured targets.
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmPrinters();
  InitializeAllAsmParsers();

  // set up the TargetOptions for the machine
  TargetOptions Options = InitTargetOptionsFromCodeGenFlags();

  if (ListSymbolsOnly) {
    listSymbols(Options);
    return 0;
  }

  if (ThinLTOMode.getNumOccurrences()) {
    if (ThinLTOMode.getNumOccurrences() > 1)
      report_fatal_error("You can't specify more than one -thinlto-action");
    thinlto::ThinLTOProcessing ThinLTOProcessor(Options);
    ThinLTOProcessor.run();
    return 0;
  }

  if (ThinLTO) {
    createCombinedModuleSummaryIndex();
    return 0;
  }

  unsigned BaseArg = 0;

  LLVMContext Context;
  Context.setDiagnosticHandler(diagnosticHandlerWithContext, nullptr, true);

  LTOCodeGenerator CodeGen(Context);

  if (UseDiagnosticHandler)
    CodeGen.setDiagnosticHandler(handleDiagnostics, nullptr);

  CodeGen.setCodePICModel(RelocModel);

  CodeGen.setDebugInfo(LTO_DEBUG_MODEL_DWARF);
  CodeGen.setTargetOptions(Options);
  CodeGen.setShouldRestoreGlobalsLinkage(RestoreGlobalsLinkage);

  llvm::StringSet<llvm::MallocAllocator> DSOSymbolsSet;
  for (unsigned i = 0; i < DSOSymbols.size(); ++i)
    DSOSymbolsSet.insert(DSOSymbols[i]);

  std::vector<std::string> KeptDSOSyms;

  for (unsigned i = BaseArg; i < InputFilenames.size(); ++i) {
    CurrentActivity = "loading file '" + InputFilenames[i] + "'";
    ErrorOr<std::unique_ptr<LTOModule>> ModuleOrErr =
        LTOModule::createFromFile(Context, InputFilenames[i].c_str(), Options);
    std::unique_ptr<LTOModule> &Module = *ModuleOrErr;
    CurrentActivity = "";

    unsigned NumSyms = Module->getSymbolCount();
    for (unsigned I = 0; I < NumSyms; ++I) {
      StringRef Name = Module->getSymbolName(I);
      if (!DSOSymbolsSet.count(Name))
        continue;
      lto_symbol_attributes Attrs = Module->getSymbolAttributes(I);
      unsigned Scope = Attrs & LTO_SYMBOL_SCOPE_MASK;
      if (Scope != LTO_SYMBOL_SCOPE_DEFAULT_CAN_BE_HIDDEN)
        KeptDSOSyms.push_back(Name);
    }

    // We use the first input module as the destination module when
    // SetMergedModule is true.
    if (SetMergedModule && i == BaseArg) {
      // Transfer ownership to the code generator.
      CodeGen.setModule(std::move(Module));
    } else if (!CodeGen.addModule(Module.get())) {
      // Print a message here so that we know addModule() did not abort.
      error("error adding file '" + InputFilenames[i] + "'");
    }
  }

  // Add all the exported symbols to the table of symbols to preserve.
  for (unsigned i = 0; i < ExportedSymbols.size(); ++i)
    CodeGen.addMustPreserveSymbol(ExportedSymbols[i].c_str());

  // Add all the dso symbols to the table of symbols to expose.
  for (unsigned i = 0; i < KeptDSOSyms.size(); ++i)
    CodeGen.addMustPreserveSymbol(KeptDSOSyms[i].c_str());

  // Set cpu and attrs strings for the default target/subtarget.
  CodeGen.setCpu(MCPU.c_str());

  CodeGen.setOptLevel(OptLevel - '0');

  std::string attrs;
  for (unsigned i = 0; i < MAttrs.size(); ++i) {
    if (i > 0)
      attrs.append(",");
    attrs.append(MAttrs[i]);
  }

  if (!attrs.empty())
    CodeGen.setAttr(attrs.c_str());

  if (FileType.getNumOccurrences())
    CodeGen.setFileType(FileType);

  if (!OutputFilename.empty()) {
    if (!CodeGen.optimize(DisableVerify, DisableInline, DisableGVNLoadPRE,
                          DisableLTOVectorization)) {
      // Diagnostic messages should have been printed by the handler.
      error("error optimizing the code");
    }

    if (SaveModuleFile) {
      std::string ModuleFilename = OutputFilename;
      ModuleFilename += ".merged.bc";
      std::string ErrMsg;

      if (!CodeGen.writeMergedModules(ModuleFilename.c_str()))
        error("writing merged module failed.");
    }

    std::list<tool_output_file> OSs;
    std::vector<raw_pwrite_stream *> OSPtrs;
    for (unsigned I = 0; I != Parallelism; ++I) {
      std::string PartFilename = OutputFilename;
      if (Parallelism != 1)
        PartFilename += "." + utostr(I);
      std::error_code EC;
      OSs.emplace_back(PartFilename, EC, sys::fs::F_None);
      if (EC)
        error("error opening the file '" + PartFilename + "': " + EC.message());
      OSPtrs.push_back(&OSs.back().os());
    }

    if (!CodeGen.compileOptimized(OSPtrs))
      // Diagnostic messages should have been printed by the handler.
      error("error compiling the code");

    for (tool_output_file &OS : OSs)
      OS.keep();
  } else {
    if (Parallelism != 1)
      error("-j must be specified together with -o");

    if (SaveModuleFile)
      error(": -save-merged-module must be specified with -o");

    const char *OutputName = nullptr;
    if (!CodeGen.compile_to_file(&OutputName, DisableVerify, DisableInline,
                                 DisableGVNLoadPRE, DisableLTOVectorization))
      error("error compiling the code");
      // Diagnostic messages should have been printed by the handler.

    outs() << "Wrote native object file '" << OutputName << "'\n";
  }

  return 0;
}
