; RUN: llvm-as %s -o %t1
; RUN: opt %t1 -insert-edge-profiling -o %t2
; RUN: llvm-dis < %t2 | FileCheck --check-prefix=INST %s

; RUN: lli -load %llvmlibsdir/profile_rt%shlibext %t2
; RUN: lli -load %llvmlibsdir/profile_rt%shlibext %t2 1 2
; RUN: llvm-prof -print-all-code %t1 | tee %t3 | FileCheck --check-prefix=PROF %s

; PROF:  1.     2/4 oneblock
; PROF:  2.     2/4 main
; PROF:  1. 15.7895%    12/76	main() - bb6
; PROF:  2. 11.8421%     9/76	main() - bb2
; PROF:  3. 11.8421%     9/76	main() - bb3
; PROF:  4. 11.8421%     9/76	main() - bb5
; PROF:  5. 10.5263%     8/76	main() - bb10
; PROF:  6. 7.89474%     6/76	main() - bb
; PROF:  7. 7.89474%     6/76	main() - bb9
; PROF:  8. 3.94737%     3/76	main() - bb1
; PROF:  9. 3.94737%     3/76	main() - bb7
; PROF: 10. 3.94737%     3/76	main() - bb8
; PROF: 11. 2.63158%     2/76	oneblock() - entry
; PROF: 12. 2.63158%     2/76	main() - entry
; PROF: 13. 2.63158%     2/76	main() - bb11
; PROF: 14. 2.63158%     2/76	main() - return

; ModuleID = '<stdin>'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private constant [12 x i8] c"hello world\00", align 1 ; <[12 x i8]*> [#uses=1]
@.str1 = private constant [6 x i8] c"franz\00", align 1 ; <[6 x i8]*> [#uses=1]
@.str2 = private constant [9 x i8] c"argc > 2\00", align 1 ; <[9 x i8]*> [#uses=1]
@.str3 = private constant [9 x i8] c"argc = 1\00", align 1 ; <[9 x i8]*> [#uses=1]
@.str4 = private constant [6 x i8] c"fritz\00", align 1 ; <[6 x i8]*> [#uses=1]
@.str5 = private constant [10 x i8] c"argc <= 1\00", align 1 ; <[10 x i8]*> [#uses=1]
; INST:@EdgeProfCounters
; INST:[19 x i32] 
; INST:zeroinitializer

; PROF:;;; %oneblock called 2 times.
; PROF:;;;
define void @oneblock() nounwind {
entry:
; PROF:entry:
; PROF:	;;; Basic block executed 2 times.
  %0 = call i32 @puts(i8* getelementptr inbounds ([12 x i8]* @.str, i64 0, i64 0)) nounwind ; <i32> [#uses=0]
  ret void
}

declare i32 @puts(i8*)

; PROF:;;; %main called 2 times.
; PROF:;;;
define i32 @main(i32 %argc, i8** %argv) nounwind {
entry:
; PROF:entry:
; PROF:	;;; Basic block executed 2 times.
  %argc_addr = alloca i32                         ; <i32*> [#uses=4]
  %argv_addr = alloca i8**                        ; <i8***> [#uses=1]
  %retval = alloca i32                            ; <i32*> [#uses=2]
  %j = alloca i32                                 ; <i32*> [#uses=4]
  %i = alloca i32                                 ; <i32*> [#uses=4]
  %0 = alloca i32                                 ; <i32*> [#uses=2]
; INST:call 
; INST:@llvm_start_edge_profiling
; INST:@EdgeProfCounters
  %"alloca point" = bitcast i32 0 to i32          ; <i32> [#uses=0]
  store i32 %argc, i32* %argc_addr
  store i8** %argv, i8*** %argv_addr
  store i32 0, i32* %i, align 4
  br label %bb10
; PROF:	;;; Out-edge counts: [2.000000e+00 -> bb10]

bb:                                               ; preds = %bb10
; PROF:bb:
; PROF:	;;; Basic block executed 6 times.
  %1 = load i32* %argc_addr, align 4              ; <i32> [#uses=1]
  %2 = icmp sgt i32 %1, 1                         ; <i1> [#uses=1]
  br i1 %2, label %bb1, label %bb8
; PROF:	;;; Out-edge counts: [3.000000e+00 -> bb1] [3.000000e+00 -> bb8]

bb1:                                              ; preds = %bb
; PROF:bb1:
; PROF:	;;; Basic block executed 3 times.
  store i32 0, i32* %j, align 4
  br label %bb6
; PROF:	;;; Out-edge counts: [3.000000e+00 -> bb6]

bb2:                                              ; preds = %bb6
; PROF:bb2:
; PROF:	;;; Basic block executed 9 times.
  %3 = call i32 @puts(i8* getelementptr inbounds ([6 x i8]* @.str1, i64 0, i64 0)) nounwind ; <i32> [#uses=0]
  %4 = load i32* %argc_addr, align 4              ; <i32> [#uses=1]
  %5 = icmp sgt i32 %4, 2                         ; <i1> [#uses=1]
  br i1 %5, label %bb3, label %bb4
; PROF:	;;; Out-edge counts: [9.000000e+00 -> bb3]

bb3:                                              ; preds = %bb2
; PROF:bb3:
; PROF:	;;; Basic block executed 9 times.
  %6 = call i32 @puts(i8* getelementptr inbounds ([9 x i8]* @.str2, i64 0, i64 0)) nounwind ; <i32> [#uses=0]
  br label %bb5
; PROF:	;;; Out-edge counts: [9.000000e+00 -> bb5]

bb4:                                              ; preds = %bb2
; PROF:bb4:
; PROF:	;;; Never executed!
  %7 = call i32 @puts(i8* getelementptr inbounds ([9 x i8]* @.str3, i64 0, i64 0)) nounwind ; <i32> [#uses=0]
  br label %bb11

bb5:                                              ; preds = %bb3
; PROF:bb5:
; PROF:	;;; Basic block executed 9 times.
  %8 = call i32 @puts(i8* getelementptr inbounds ([6 x i8]* @.str4, i64 0, i64 0)) nounwind ; <i32> [#uses=0]
  %9 = load i32* %j, align 4                      ; <i32> [#uses=1]
  %10 = add nsw i32 %9, 1                         ; <i32> [#uses=1]
  store i32 %10, i32* %j, align 4
  br label %bb6
; PROF:	;;; Out-edge counts: [9.000000e+00 -> bb6]

bb6:                                              ; preds = %bb5, %bb1
; PROF:bb6:
; PROF:	;;; Basic block executed 12 times.
  %11 = load i32* %j, align 4                     ; <i32> [#uses=1]
  %12 = load i32* %argc_addr, align 4             ; <i32> [#uses=1]
  %13 = icmp slt i32 %11, %12                     ; <i1> [#uses=1]
  br i1 %13, label %bb2, label %bb7
; PROF:	;;; Out-edge counts: [9.000000e+00 -> bb2] [3.000000e+00 -> bb7]

bb7:                                              ; preds = %bb6
; PROF:bb7:
; PROF:	;;; Basic block executed 3 times.
  br label %bb9
; PROF:	;;; Out-edge counts: [3.000000e+00 -> bb9]

bb8:                                              ; preds = %bb
; PROF:bb8:
; PROF:	;;; Basic block executed 3 times.
  %14 = call i32 @puts(i8* getelementptr inbounds ([10 x i8]* @.str5, i64 0, i64 0)) nounwind ; <i32> [#uses=0]
  br label %bb9
; PROF:	;;; Out-edge counts: [3.000000e+00 -> bb9]

bb9:                                              ; preds = %bb8, %bb7
; PROF:bb9:
; PROF:	;;; Basic block executed 6 times.
  %15 = load i32* %i, align 4                     ; <i32> [#uses=1]
  %16 = add nsw i32 %15, 1                        ; <i32> [#uses=1]
  store i32 %16, i32* %i, align 4
  br label %bb10
; PROF:	;;; Out-edge counts: [6.000000e+00 -> bb10]

bb10:                                             ; preds = %bb9, %entry
; PROF:bb10:
; PROF:	;;; Basic block executed 8 times.
  %17 = load i32* %i, align 4                     ; <i32> [#uses=1]
  %18 = icmp ne i32 %17, 3                        ; <i1> [#uses=1]
  br i1 %18, label %bb, label %bb11
; INST:br
; INST:label %bb10.bb11_crit_edge
; PROF:	;;; Out-edge counts: [6.000000e+00 -> bb] [2.000000e+00 -> bb11]

; INST:bb10.bb11_crit_edge:
; INST:br
; INST:label %bb11

bb11:                                             ; preds = %bb10, %bb4
; PROF:bb11:
; PROF:	;;; Basic block executed 2 times.
  call void @oneblock() nounwind
  store i32 0, i32* %0, align 4
  %19 = load i32* %0, align 4                     ; <i32> [#uses=1]
  store i32 %19, i32* %retval, align 4
  br label %return
; PROF:	;;; Out-edge counts: [2.000000e+00 -> return]

return:                                           ; preds = %bb11
; PROF:return:
; PROF:	;;; Basic block executed 2 times.
  %retval12 = load i32* %retval                   ; <i32> [#uses=1]
  ret i32 %retval12
}
