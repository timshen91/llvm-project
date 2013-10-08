//===- lld/Driver/DarwinInputGraph.h - Input Graph Node for Mach-O linker -===//
//
//                             The LLVM Linker
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
///
/// Handles Options for MachO linking and provides InputElements
/// for MachO linker
///
//===----------------------------------------------------------------------===//

#ifndef LLD_DRIVER_DARWIN_INPUT_GRAPH_H
#define LLD_DRIVER_DARWIN_INPUT_GRAPH_H

#include "lld/Driver/InputGraph.h"
#include "lld/ReaderWriter/MachOLinkingContext.h"

#include <map>

namespace lld {

/// \brief Represents a MachO File
class MachOFileNode : public FileNode {
public:
  MachOFileNode(MachOLinkingContext &ctx, StringRef path, bool isWholeArchive)
      : FileNode(path), _ctx(ctx), _isWholeArchive(isWholeArchive) {}

  static inline bool classof(const InputElement *a) {
    return a->kind() == InputElement::Kind::File;
  }

  /// \brief validates the Input Element
  virtual bool validate() {
    (void)_ctx;
    return true;
  }

  /// \brief Parse the input file to lld::File.
  llvm::error_code parse(const LinkingContext &ctx, raw_ostream &diagnostics) {
    ErrorOr<StringRef> filePath = getPath(ctx);
    if (!filePath &&
        error_code(filePath) == llvm::errc::no_such_file_or_directory)
      return make_error_code(llvm::errc::no_such_file_or_directory);

    // Create a memory buffer
    OwningPtr<llvm::MemoryBuffer> opmb;
    llvm::error_code ec;

    if ((ec = llvm::MemoryBuffer::getFileOrSTDIN(*filePath, opmb)))
      return ec;

    std::unique_ptr<MemoryBuffer> mb(opmb.take());
    _buffer = std::move(mb);

    if (ctx.logInputFiles())
      diagnostics << _buffer->getBufferIdentifier() << "\n";

    // YAML file is identified by a .objtxt extension
    // FIXME : Identify YAML files by using a magic
    if (filePath->endswith(".objtxt")) {
      ec = _ctx.getYAMLReader().parseFile(_buffer, _files);
      if (!ec)
        return ec;
    }

    (void) (_isWholeArchive);

    return llvm::error_code::success();
  }

  /// \brief Return the file that has to be processed by the resolver
  /// to resolve atoms. This iterates over all the files thats part
  /// of this node. Returns no_more_files when there are no files to be
  /// processed
  virtual ErrorOr<File &> getNextFile() {
    if (_files.size() == _nextFileIndex)
      return make_error_code(input_graph_error::no_more_files);
    return *_files[_nextFileIndex++];
  }

  /// \brief Dump the Input Element
  virtual bool dump(raw_ostream &) { return true; }

private:
  const MachOLinkingContext &_ctx;
  bool _isWholeArchive;
};

} // namespace lld

#endif
