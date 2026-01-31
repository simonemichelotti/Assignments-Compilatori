; ModuleID = 'test/Loop.opt.bc'
source_filename = "loop.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

define dso_local void @foo(i32 noundef %0, ptr noundef %1, ptr noundef %2, ptr noundef %3, ptr noundef %4) {
  br label %6

6:                                                ; preds = %18, %5
  %.02 = phi i32 [ 0, %5 ], [ %19, %18 ]
  %7 = icmp slt i32 %.02, %0
  br i1 %7, label %8, label %37

8:                                                ; preds = %6
  %9 = sext i32 %.02 to i64
  %10 = getelementptr inbounds i32, ptr %2, i64 %9
  %11 = load i32, ptr %10, align 4
  %12 = sext i32 %.02 to i64
  %13 = getelementptr inbounds i32, ptr %3, i64 %12
  %14 = load i32, ptr %13, align 4
  %15 = mul nsw i32 %11, %14
  %16 = sext i32 %.02 to i64
  %17 = getelementptr inbounds i32, ptr %1, i64 %16
  store i32 %15, ptr %17, align 4
  br label %20

18:                                               ; preds = %30
  %19 = add nsw i32 %.02, 1
  br label %6, !llvm.loop !4

20:                                               ; preds = %8
  %21 = sext i32 %.02 to i64
  %22 = getelementptr inbounds i32, ptr %1, i64 %21
  %23 = load i32, ptr %22, align 4
  %24 = sext i32 %.02 to i64
  %25 = getelementptr inbounds i32, ptr %3, i64 %24
  %26 = load i32, ptr %25, align 4
  %27 = add nsw i32 %23, %26
  %28 = sext i32 %.02 to i64
  %29 = getelementptr inbounds i32, ptr %4, i64 %28
  store i32 %27, ptr %29, align 4
  br label %30

30:                                               ; preds = %20
  %31 = sext i32 %.02 to i64
  %32 = getelementptr inbounds i32, ptr %3, i64 %31
  %33 = load i32, ptr %32, align 4
  %34 = sub nsw i32 %33, 1
  %35 = sext i32 %.02 to i64
  %36 = getelementptr inbounds i32, ptr %1, i64 %35
  store i32 %34, ptr %36, align 4
  br label %18

37:                                               ; preds = %6
  ret void
}

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"clang version 18.1.6 (Fedora 18.1.6-3.fc40)"}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.mustprogress"}
