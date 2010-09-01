// Test is line- and column-sensitive; see below.

namespace std {
  namespace rel_ops {
    void f();
  }
}

namespace std {
  void g();
}

namespace std98 = std;
namespace std0x = std98;

using namespace std0x;

namespace std {
  int g(int);
}

using std::g;

// RUN: c-index-test -test-load-source all %s | FileCheck %s
// CHECK: load-namespaces.cpp:3:11: Namespace=std:3:11 (Definition) Extent=[3:11 - 7:2]
// CHECK: load-namespaces.cpp:4:13: Namespace=rel_ops:4:13 (Definition) Extent=[4:13 - 6:4]
// CHECK: load-namespaces.cpp:5:10: FunctionDecl=f:5:10 Extent=[5:10 - 5:13]
// CHECK: load-namespaces.cpp:9:11: Namespace=std:9:11 (Definition) Extent=[9:11 - 11:2]
// CHECK: load-namespaces.cpp:10:8: FunctionDecl=g:10:8 Extent=[10:8 - 10:11]
// CHECK: load-namespaces.cpp:13:11: NamespaceAlias=std98:13:11 Extent=[13:1 - 13:22]
// CHECK: load-namespaces.cpp:13:19: NamespaceRef=std:3:11 Extent=[13:19 - 13:22]
// CHECK: load-namespaces.cpp:14:11: NamespaceAlias=std0x:14:11 Extent=[14:1 - 14:24]
// CHECK: load-namespaces.cpp:14:19: NamespaceRef=std98:13:11 Extent=[14:19 - 14:24]
// CHECK: load-namespaces.cpp:16:17: UsingDirective=:16:17 Extent=[16:1 - 16:22]
// CHECK: load-namespaces.cpp:16:17: NamespaceRef=std0x:14:11 Extent=[16:17 - 16:22]
// CHECK: load-namespaces.cpp:18:11: Namespace=std:18:11 (Definition) Extent=[18:11 - 20:2]
// CHECK: load-namespaces.cpp:19:7: FunctionDecl=g:19:7 Extent=[19:7 - 19:13]
// CHECK: load-namespaces.cpp:19:12: ParmDecl=:19:12 (Definition) Extent=[19:9 - 19:13]
// CHECK: load-namespaces.cpp:22:12: UsingDeclaration=g:22:12 Extent=[22:1 - 22:13]
