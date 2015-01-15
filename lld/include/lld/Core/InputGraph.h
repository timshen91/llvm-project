//===- lld/Core/InputGraph.h - Input Graph --------------------------------===//
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
/// Inputs to the linker in the form of a Graph.
///
//===----------------------------------------------------------------------===//

#ifndef LLD_CORE_INPUT_GRAPH_H
#define LLD_CORE_INPUT_GRAPH_H

#include "lld/Core/File.h"
#include "lld/Core/range.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include <functional>
#include <memory>
#include <stack>
#include <vector>

namespace lld {

class Node;
class LinkingContext;

class InputGraph {
public:
  std::vector<std::unique_ptr<Node>> &members() {
    return _members;
  }

protected:
  std::vector<std::unique_ptr<Node>> _members;
};

// A Node represents a FileNode or other type of Node. In the latter case,
// the node contains meta information about the input file list.
// Currently only GroupEnd node is defined as a meta node.
class Node {
public:
  enum class Kind { File, GroupEnd };
  Node(Kind type) : _kind(type) {}
  virtual ~Node() {}
  virtual Kind kind() const { return _kind; }

private:
  Kind _kind;
};

// This is a marker for --end-group. getSize() returns the number of
// files between the corresponding --start-group and this marker.
class GroupEnd : public Node {
public:
  GroupEnd(int size) : Node(Kind::GroupEnd), _size(size) {}

  int getSize() const { return _size; }

  static inline bool classof(const Node *a) {
    return a->kind() == Kind::GroupEnd;
  }

private:
  int _size;
};

// A container of File.
class FileNode : public Node {
public:
  explicit FileNode(std::unique_ptr<File> f)
      : Node(Node::Kind::File), _file(std::move(f)) {}

  virtual ~FileNode() {}

  static inline bool classof(const Node *a) {
    return a->kind() == Node::Kind::File;
  }

  File *getFile() { return _file.get(); }

protected:
  std::unique_ptr<File> _file;
};

} // namespace lld

#endif // LLD_CORE_INPUT_GRAPH_H
