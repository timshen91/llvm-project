//===- CIndexer.h - Clang-C Source Indexing Library -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines CIndexer, a subclass of Indexer that provides extra
// functionality needed by the CIndex library.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_CINDEXER_H
#define LLVM_CLANG_CINDEXER_H

#include "clang-c/Index.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/System/Path.h"
#include <vector>

namespace clang {
namespace cxstring {
  CXString createCXString(const char *String, bool DupString = false);
  CXString createCXString(llvm::StringRef String, bool DupString = true);  
}
}

class CIndexer {
  bool OnlyLocalDecls;
  bool DisplayDiagnostics;

  llvm::sys::Path ResourcesPath;
  std::string WorkingDir;

public:
 CIndexer() : OnlyLocalDecls(false), DisplayDiagnostics(false) { }
  
  /// \brief Whether we only want to see "local" declarations (that did not
  /// come from a previous precompiled header). If false, we want to see all
  /// declarations.
  bool getOnlyLocalDecls() const { return OnlyLocalDecls; }
  void setOnlyLocalDecls(bool Local = true) { OnlyLocalDecls = Local; }
  
  bool getDisplayDiagnostics() const { return DisplayDiagnostics; }
  void setDisplayDiagnostics(bool Display = true) {
    DisplayDiagnostics = Display;
  }

  /// \brief Get the path of the clang resource files.
  std::string getClangResourcesPath();

  const std::string &getWorkingDirectory() const { return WorkingDir; }
  void setWorkingDirectory(const std::string &Dir) { WorkingDir = Dir; }
};

namespace clang {
  /**
   * \brief Given a set of "unsaved" files, create temporary files and 
   * construct the clang -cc1 argument list needed to perform the remapping.
   *
   * \returns true if an error occurred.
   */
  bool RemapFiles(unsigned num_unsaved_files,
                  struct CXUnsavedFile *unsaved_files,
                  std::vector<std::string> &RemapArgs,
                  std::vector<llvm::sys::Path> &TemporaryFiles);
}

#endif
