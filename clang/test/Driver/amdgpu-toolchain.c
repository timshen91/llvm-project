// RUN: %clang -### -target amdgcn--amdhsa -x assembler -mcpu=kaveri %s 2>&1 | FileCheck -check-prefix=AS_LINK %s
// AS_LINK: bin/clang {{.*}} "-cc1as"
// AS_LINK: bin/lld {{.*}} "-flavor" "gnu" "-target" "amdgcn--amdhsa"
