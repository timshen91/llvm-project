; RUN: opt %s -sroa -verify -S -o - | FileCheck %s
;
; Test that we can partial emit debug info for aggregates repeatedly
; split up by SROA.
;
;    // Compile with -O1
;    typedef struct {
;      int a;
;      long int b;
;    } Inner;
;
;    typedef struct {
;      Inner inner[2];
;    } Outer;
;
;    int foo(Outer outer) {
;      Inner i1 = outer.inner[1];
;      return i1.a;
;    }
;

; Verify that SROA creates a variable piece when splitting i1.
; CHECK: %[[I1:.*]] = alloca [12 x i8], align 4
; CHECK: call void @llvm.dbg.declare(metadata [12 x i8]* %[[I1]], metadata ![[VAR:[0-9]+]], metadata ![[PIECE1:[0-9]+]])
; CHECK: call void @llvm.dbg.value(metadata i32 %[[A:.*]], i64 0, metadata ![[VAR]], metadata ![[PIECE2:[0-9]+]])
; CHECK: ret i32 %[[A]]
; Read Var and Piece:
; CHECK: ![[VAR]] = !MDLocalVariable(tag: DW_TAG_auto_variable, name: "i1",{{.*}} line: 11,
; CHECK: ![[PIECE1]] = !MDExpression(DW_OP_bit_piece, 32, 96)
; CHECK: ![[PIECE2]] = !MDExpression(DW_OP_bit_piece, 0, 32)

target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.9.0"

%struct.Outer = type { [2 x %struct.Inner] }
%struct.Inner = type { i32, i64 }

; Function Attrs: nounwind ssp uwtable
define i32 @foo(%struct.Outer* byval align 8 %outer) #0 {
entry:
  %i1 = alloca %struct.Inner, align 8
  call void @llvm.dbg.declare(metadata %struct.Outer* %outer, metadata !25, metadata !2), !dbg !26
  call void @llvm.dbg.declare(metadata %struct.Inner* %i1, metadata !27, metadata !2), !dbg !28
  %inner = getelementptr inbounds %struct.Outer, %struct.Outer* %outer, i32 0, i32 0, !dbg !28
  %arrayidx = getelementptr inbounds [2 x %struct.Inner], [2 x %struct.Inner]* %inner, i32 0, i64 1, !dbg !28
  %0 = bitcast %struct.Inner* %i1 to i8*, !dbg !28
  %1 = bitcast %struct.Inner* %arrayidx to i8*, !dbg !28
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %0, i8* %1, i64 16, i32 8, i1 false), !dbg !28
  %a = getelementptr inbounds %struct.Inner, %struct.Inner* %i1, i32 0, i32 0, !dbg !29
  %2 = load i32, i32* %a, align 4, !dbg !29
  ret i32 %2, !dbg !29
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i32, i1) #2

attributes #0 = { nounwind ssp uwtable }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!22, !23}
!llvm.ident = !{!24}

!0 = !MDCompileUnit(language: DW_LANG_C99, producer: "clang version 3.5.0 ", isOptimized: false, emissionKind: 1, file: !1, enums: !2, retainedTypes: !2, subprograms: !3, globals: !2, imports: !2)
!1 = !MDFile(filename: "sroasplit-1.c", directory: "")
!2 = !MDExpression()
!3 = !{!4}
!4 = !MDSubprogram(name: "foo", line: 10, isLocal: false, isDefinition: true, virtualIndex: 6, flags: DIFlagPrototyped, isOptimized: false, scopeLine: 10, file: !1, scope: !5, type: !6, function: i32 (%struct.Outer*)* @foo, variables: !2)
!5 = !MDFile(filename: "sroasplit-1.c", directory: "")
!6 = !MDSubroutineType(types: !7)
!7 = !{!8, !9}
!8 = !MDBasicType(tag: DW_TAG_base_type, name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!9 = !MDDerivedType(tag: DW_TAG_typedef, name: "Outer", line: 8, file: !1, baseType: !10)
!10 = !MDCompositeType(tag: DW_TAG_structure_type, line: 6, size: 256, align: 64, file: !1, elements: !11)
!11 = !{!12}
!12 = !MDDerivedType(tag: DW_TAG_member, name: "inner", line: 7, size: 256, align: 64, file: !1, scope: !10, baseType: !13)
!13 = !MDCompositeType(tag: DW_TAG_array_type, size: 256, align: 64, baseType: !14, elements: !20)
!14 = !MDDerivedType(tag: DW_TAG_typedef, name: "Inner", line: 4, file: !1, baseType: !15)
!15 = !MDCompositeType(tag: DW_TAG_structure_type, line: 1, size: 128, align: 64, file: !1, elements: !16)
!16 = !{!17, !18}
!17 = !MDDerivedType(tag: DW_TAG_member, name: "a", line: 2, size: 32, align: 32, file: !1, scope: !15, baseType: !8)
!18 = !MDDerivedType(tag: DW_TAG_member, name: "b", line: 3, size: 64, align: 64, offset: 64, file: !1, scope: !15, baseType: !19)
!19 = !MDBasicType(tag: DW_TAG_base_type, name: "long int", size: 64, align: 64, encoding: DW_ATE_signed)
!20 = !{!21}
!21 = !MDSubrange(count: 2)
!22 = !{i32 2, !"Dwarf Version", i32 2}
!23 = !{i32 1, !"Debug Info Version", i32 3}
!24 = !{!"clang version 3.5.0 "}
!25 = !MDLocalVariable(tag: DW_TAG_arg_variable, name: "outer", line: 10, arg: 1, scope: !4, file: !5, type: !9)
!26 = !MDLocation(line: 10, scope: !4)
!27 = !MDLocalVariable(tag: DW_TAG_auto_variable, name: "i1", line: 11, scope: !4, file: !5, type: !14)
!28 = !MDLocation(line: 11, scope: !4)
!29 = !MDLocation(line: 12, scope: !4)
