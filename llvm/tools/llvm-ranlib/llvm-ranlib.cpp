//===-- llvm-ranlib.cpp - LLVM archive index generator --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by Reid Spencer and is distributed under the
// University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Adds or updates an index (symbol table) for an LLVM archive file.
//
//===----------------------------------------------------------------------===//

#include "llvm/Module.h"
#include "llvm/Bytecode/Archive.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/System/Signals.h"
#include <iostream>
#include <iomanip>

using namespace llvm;

// llvm-ar operation code and modifier flags
static cl::opt<std::string>
ArchiveName(cl::Positional, cl::Optional, cl::desc("<archive-file>"));

static cl::opt<bool>
Verbose("verbose",cl::Optional,cl::init(false),
        cl::desc("Print the symbol table"));

// printSymbolTable - print out the archive's symbol table.
void printSymbolTable(Archive* TheArchive) {
  std::cout << "\nArchive Symbol Table:\n";
  const Archive::SymTabType& symtab = TheArchive->getSymbolTable();
  for (Archive::SymTabType::const_iterator I=symtab.begin(), E=symtab.end();
       I != E; ++I ) {
    unsigned offset = TheArchive->getFirstFileOffset() + I->second;
    std::cout << " " << std::setw(9) << offset << "\t" << I->first <<"\n";
  }
}

int main(int argc, char **argv) {

  // Have the command line options parsed and handle things
  // like --help and --version.
  cl::ParseCommandLineOptions(argc, argv,
    " LLVM Archive Index Generator (llvm-ranlib)\n\n"
    "  This program adds or updates an index of bytecode symbols\n"
    "  to an LLVM archive file."
  );

  // Print a stack trace if we signal out.
  sys::PrintStackTraceOnErrorSignal();

  int exitCode = 0;

  // Make sure we don't exit with "unhandled exception".
  try {

    // Check the path name of the archive
    sys::Path ArchivePath;
    if (!ArchivePath.setFile(ArchiveName))
      throw std::string("Archive name invalid: ") + ArchiveName;

    // Make sure it exists, we don't create empty archives
    if (!ArchivePath.exists())
      throw std::string("Archive file does not exist");

    std::string err_msg;
    std::auto_ptr<Archive>
      AutoArchive(Archive::OpenAndLoad(ArchivePath,&err_msg));
    Archive* TheArchive = AutoArchive.get();
    if (!TheArchive)
      throw err_msg;

    TheArchive->writeToDisk(true, false, false );

    if (Verbose)
      printSymbolTable(TheArchive);

  } catch (const char*msg) {
    std::cerr << argv[0] << ": " << msg << "\n\n";
    exitCode = 1;
  } catch (const std::string& msg) {
    std::cerr << argv[0] << ": " << msg << "\n";
    exitCode = 2;
  } catch (...) {
    std::cerr << argv[0] << ": An unexpected unknown exception occurred.\n";
    exitCode = 3;
  }
  return exitCode;
}
