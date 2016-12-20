; RUN: llc -mcpu=core2 -mtriple=i686-pc-win32 < %s -filetype=obj | llvm-readobj -codeview | FileCheck %s --check-prefix=OBJ

; This LL file was generated by running 'clang -O1 -g -gcodeview' on the
; following code:
; int volatile x;
; static inline void f() {
;   ++x;
; }
; static inline void g() {
;   f();
; }
; static inline void h() {
;   g();
; }
; int main() {
;   h();
; }

; OBJ: Subsection [
; OBJ:   SubSectionType: Symbols (0xF1)
; OBJ:   ProcStart {
; OBJ:   InlineSite {
; OBJ:     Inlinee: h (0x1002)
; OBJ:   }
; OBJ:   InlineSite {
; OBJ:     Inlinee: g (0x1003)
; OBJ:   }
; OBJ:   InlineSite {
; OBJ:     Inlinee: f (0x1004)
; OBJ:   }
; OBJ:   InlineSiteEnd {
; OBJ:   }
; OBJ:   InlineSiteEnd {
; OBJ:   }
; OBJ:   InlineSiteEnd {
; OBJ:   }
; OBJ:   ProcEnd
; OBJ: ]

; ModuleID = 't.cpp'
target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc18.0.0"

@"\01?x@@3HC" = global i32 0, align 4, !dbg !14

; Function Attrs: norecurse nounwind uwtable
define i32 @main() #0 !dbg !4 {
entry:
  %0 = load volatile i32, i32* @"\01?x@@3HC", align 4, !dbg !20, !tbaa !24
  %inc.i.i.i = add nsw i32 %0, 1, !dbg !20
  store volatile i32 %inc.i.i.i, i32* @"\01?x@@3HC", align 4, !dbg !20, !tbaa !24
  ret i32 0, !dbg !28
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!16, !17, !18}
!llvm.ident = !{!19}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 3.9.0 ", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, globals: !13)
!1 = !DIFile(filename: "t.cpp", directory: "D:\5Csrc\5Cllvm\5Cbuild")
!2 = !{}
!4 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 12, type: !5, isLocal: false, isDefinition: true, scopeLine: 12, flags: DIFlagPrototyped, isOptimized: true, unit: !0, variables: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!8 = distinct !DISubprogram(name: "h", linkageName: "\01?h@@YAXXZ", scope: !1, file: !1, line: 9, type: !9, isLocal: true, isDefinition: true, scopeLine: 9, flags: DIFlagPrototyped, isOptimized: true, unit: !0, variables: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{null}
!11 = distinct !DISubprogram(name: "g", linkageName: "\01?g@@YAXXZ", scope: !1, file: !1, line: 6, type: !9, isLocal: true, isDefinition: true, scopeLine: 6, flags: DIFlagPrototyped, isOptimized: true, unit: !0, variables: !2)
!12 = distinct !DISubprogram(name: "f", linkageName: "\01?f@@YAXXZ", scope: !1, file: !1, line: 2, type: !9, isLocal: true, isDefinition: true, scopeLine: 2, flags: DIFlagPrototyped, isOptimized: true, unit: !0, variables: !2)
!13 = !{!14}
!14 = !DIGlobalVariableExpression(var: !DIGlobalVariable(name: "x", linkageName: "\01?x@@3HC", scope: !0, file: !1, line: 1, type: !15, isLocal: false, isDefinition: true))
!15 = !DIDerivedType(tag: DW_TAG_volatile_type, baseType: !7)
!16 = !{i32 2, !"CodeView", i32 1}
!17 = !{i32 2, !"Debug Info Version", i32 3}
!18 = !{i32 1, !"PIC Level", i32 2}
!19 = !{!"clang version 3.9.0 "}
!20 = !DILocation(line: 4, column: 3, scope: !12, inlinedAt: !21)
!21 = distinct !DILocation(line: 7, column: 3, scope: !11, inlinedAt: !22)
!22 = distinct !DILocation(line: 10, column: 3, scope: !8, inlinedAt: !23)
!23 = distinct !DILocation(line: 13, column: 3, scope: !4)
!24 = !{!25, !25, i64 0}
!25 = !{!"int", !26, i64 0}
!26 = !{!"omnipotent char", !27, i64 0}
!27 = !{!"Simple C/C++ TBAA"}
!28 = !DILocation(line: 14, column: 1, scope: !4)
