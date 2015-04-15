; RUN: llc %s -mtriple=i386-apple-macosx10.6.7 -o /dev/null

; This used to crash because early dup was not ignoring debug instructions.

%struct.cpp_dir = type { %struct.cpp_dir*, i8*, i32, i8, i8**, i8*, i8* (i8*, %struct.cpp_dir*)*, i64, i32, i8 }

declare void @llvm.dbg.value(metadata, i64, metadata, metadata) nounwind readnone

define internal i8* @framework_construct_pathname(i8* %fname, %struct.cpp_dir* %dir) nounwind ssp {
entry:
  br i1 undef, label %bb33, label %bb

bb:                                               ; preds = %entry
  %tmp = icmp eq i32 undef, 0
  %tmp1 = add i32 0, 11
  call void @llvm.dbg.value(metadata i32 %tmp1, i64 0, metadata !0, metadata !MDExpression()), !dbg !MDLocation(scope: !1)
  br i1 undef, label %bb18, label %bb31.preheader

bb31.preheader:                                   ; preds = %bb19, %bb
  %tmp2 = getelementptr inbounds i8, i8* %fname, i32 0
  br label %bb31

bb18:                                             ; preds = %bb
  %tmp3 = icmp eq i32 undef, 0
  br i1 %tmp3, label %bb19, label %bb33

bb19:                                             ; preds = %bb18
  call void @foobar(i32 0)
  br label %bb31.preheader

bb22:                                             ; preds = %bb31
  %tmp4 = add i32 0, %tmp1
  call void @foobar(i32 %tmp4)
  br i1 undef, label %bb33, label %bb31

bb31:                                             ; preds = %bb22, %bb31.preheader
  br i1 false, label %bb33, label %bb22

bb33:                                             ; preds = %bb31, %bb22, %bb18, %entry
  ret i8* undef
}

declare void @foobar(i32)

!llvm.dbg.cu = !{!4}
!llvm.module.flags = !{!47}
!0 = !MDLocalVariable(tag: DW_TAG_auto_variable, name: "frname_len", line: 517, scope: !1, file: !3, type: !38)
!1 = distinct !MDLexicalBlock(line: 515, column: 0, file: !44, scope: !2)
!2 = !MDSubprogram(name: "framework_construct_pathname", line: 515, isLocal: true, isDefinition: true, virtualIndex: 6, flags: DIFlagPrototyped, isOptimized: true, file: !44, scope: null, type: !5, function: i8* (i8*, %struct.cpp_dir*)* @framework_construct_pathname)
!3 = !MDFile(filename: "darwin-c.c", directory: "/Users/espindola/llvm/build-llvm-gcc/gcc/../../llvm-gcc-4.2/gcc/config")
!4 = !MDCompileUnit(language: DW_LANG_C89, producer: "4.2.1 (Based on Apple Inc. build 5658) (LLVM build)", isOptimized: true, emissionKind: 0, file: !44, enums: !46, retainedTypes: !46, subprograms: !45)
!5 = !MDSubroutineType(types: !6)
!6 = !{!7, !9, !11}
!7 = !MDDerivedType(tag: DW_TAG_pointer_type, size: 32, align: 32, file: !44, scope: !3, baseType: !8)
!8 = !MDBasicType(tag: DW_TAG_base_type, name: "char", size: 8, align: 8, encoding: DW_ATE_signed_char)
!9 = !MDDerivedType(tag: DW_TAG_pointer_type, size: 32, align: 32, file: !44, scope: !3, baseType: !10)
!10 = !MDDerivedType(tag: DW_TAG_const_type, size: 8, align: 8, file: !44, scope: !3, baseType: !8)
!11 = !MDDerivedType(tag: DW_TAG_pointer_type, size: 32, align: 32, file: !44, scope: !3, baseType: !12)
!12 = !MDDerivedType(tag: DW_TAG_typedef, name: "cpp_dir", line: 45, file: !41, scope: !13, baseType: !14)
!13 = !MDFile(filename: "cpplib.h", directory: "/Users/espindola/llvm/build-llvm-gcc/gcc/../../llvm-gcc-4.2/gcc/../libcpp/include")
!14 = !MDCompositeType(tag: DW_TAG_structure_type, name: "cpp_dir", line: 43, size: 352, align: 32, file: !41, scope: !3, elements: !15)
!15 = !{!16, !18, !19, !21, !23, !25, !27, !29, !33, !36}
!16 = !MDDerivedType(tag: DW_TAG_member, name: "next", line: 572, size: 32, align: 32, file: !41, scope: !14, baseType: !17)
!17 = !MDDerivedType(tag: DW_TAG_pointer_type, size: 32, align: 32, file: !44, scope: !3, baseType: !14)
!18 = !MDDerivedType(tag: DW_TAG_member, name: "name", line: 575, size: 32, align: 32, offset: 32, file: !41, scope: !14, baseType: !7)
!19 = !MDDerivedType(tag: DW_TAG_member, name: "len", line: 576, size: 32, align: 32, offset: 64, file: !41, scope: !14, baseType: !20)
!20 = !MDBasicType(tag: DW_TAG_base_type, name: "unsigned int", size: 32, align: 32, encoding: DW_ATE_unsigned)
!21 = !MDDerivedType(tag: DW_TAG_member, name: "sysp", line: 580, size: 8, align: 8, offset: 96, file: !41, scope: !14, baseType: !22)
!22 = !MDBasicType(tag: DW_TAG_base_type, name: "unsigned char", size: 8, align: 8, encoding: DW_ATE_unsigned_char)
!23 = !MDDerivedType(tag: DW_TAG_member, name: "name_map", line: 584, size: 32, align: 32, offset: 128, file: !41, scope: !14, baseType: !24)
!24 = !MDDerivedType(tag: DW_TAG_pointer_type, size: 32, align: 32, file: !44, scope: !3, baseType: !9)
!25 = !MDDerivedType(tag: DW_TAG_member, name: "header_map", line: 590, size: 32, align: 32, offset: 160, file: !41, scope: !14, baseType: !26)
!26 = !MDDerivedType(tag: DW_TAG_pointer_type, size: 32, align: 32, file: !44, scope: !3, baseType: null)
!27 = !MDDerivedType(tag: DW_TAG_member, name: "construct", line: 597, size: 32, align: 32, offset: 192, file: !41, scope: !14, baseType: !28)
!28 = !MDDerivedType(tag: DW_TAG_pointer_type, size: 32, align: 32, file: !44, scope: !3, baseType: !5)
!29 = !MDDerivedType(tag: DW_TAG_member, name: "ino", line: 601, size: 64, align: 64, offset: 224, file: !41, scope: !14, baseType: !30)
!30 = !MDDerivedType(tag: DW_TAG_typedef, name: "ino_t", line: 141, file: !42, scope: !31, baseType: !32)
!31 = !MDFile(filename: "types.h", directory: "/usr/include/sys")
!32 = !MDBasicType(tag: DW_TAG_base_type, name: "long long unsigned int", size: 64, align: 64, encoding: DW_ATE_unsigned)
!33 = !MDDerivedType(tag: DW_TAG_member, name: "dev", line: 602, size: 32, align: 32, offset: 288, file: !41, scope: !14, baseType: !34)
!34 = !MDDerivedType(tag: DW_TAG_typedef, name: "dev_t", line: 107, file: !42, scope: !31, baseType: !35)
!35 = !MDBasicType(tag: DW_TAG_base_type, name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!36 = !MDDerivedType(tag: DW_TAG_member, name: "user_supplied_p", line: 605, size: 8, align: 8, offset: 320, file: !41, scope: !14, baseType: !37)
!37 = !MDBasicType(tag: DW_TAG_base_type, name: "_Bool", size: 8, align: 8, encoding: DW_ATE_boolean)
!38 = !MDDerivedType(tag: DW_TAG_typedef, name: "size_t", line: 326, file: !43, scope: !39, baseType: !40)
!39 = !MDFile(filename: "stddef.h", directory: "/Users/espindola/llvm/build-llvm-gcc/./prev-gcc/include")
!40 = !MDBasicType(tag: DW_TAG_base_type, name: "long unsigned int", size: 32, align: 32, encoding: DW_ATE_unsigned)
!41 = !MDFile(filename: "cpplib.h", directory: "/Users/espindola/llvm/build-llvm-gcc/gcc/../../llvm-gcc-4.2/gcc/../libcpp/include")
!42 = !MDFile(filename: "types.h", directory: "/usr/include/sys")
!43 = !MDFile(filename: "stddef.h", directory: "/Users/espindola/llvm/build-llvm-gcc/./prev-gcc/include")
!44 = !MDFile(filename: "darwin-c.c", directory: "/Users/espindola/llvm/build-llvm-gcc/gcc/../../llvm-gcc-4.2/gcc/config")
!45 = !{!2}
!46 = !{}
!47 = !{i32 1, !"Debug Info Version", i32 3}
