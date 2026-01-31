; ModuleID = 'test/Esperimento.c'
source_filename = "test/Esperimento.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [12 x i8] c"x + 0 = %d\0A\00", align 1
@.str.1 = private unnamed_addr constant [12 x i8] c"x * 1 = %d\0A\00", align 1
@.str.2 = private unnamed_addr constant [13 x i8] c"15 * x = %d\0A\00", align 1
@.str.3 = private unnamed_addr constant [12 x i8] c"x / 8 = %d\0A\00", align 1
@.str.4 = private unnamed_addr constant [16 x i8] c"a = b + 1 = %d\0A\00", align 1
@.str.5 = private unnamed_addr constant [16 x i8] c"c = a - 1 = %d\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  store i32 5, ptr %2, align 4
  store i32 3, ptr %4, align 4
  %10 = load i32, ptr %2, align 4
  %11 = add nsw i32 %10, 0
  store i32 %11, ptr %6, align 4
  %12 = load i32, ptr %2, align 4
  %13 = mul nsw i32 %12, 1
  store i32 %13, ptr %7, align 4
  %14 = load i32, ptr %2, align 4
  %15 = mul nsw i32 15, %14
  store i32 %15, ptr %8, align 4
  %16 = load i32, ptr %2, align 4
  %17 = sdiv i32 %16, 8
  store i32 %17, ptr %9, align 4
  %18 = load i32, ptr %4, align 4
  %19 = add nsw i32 %18, 1
  store i32 %19, ptr %3, align 4
  %20 = load i32, ptr %3, align 4
  %21 = sub nsw i32 %20, 1
  store i32 %21, ptr %5, align 4
  %22 = load i32, ptr %6, align 4
  %23 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %22)
  %24 = load i32, ptr %7, align 4
  %25 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %24)
  %26 = load i32, ptr %8, align 4
  %27 = call i32 (ptr, ...) @printf(ptr noundef @.str.2, i32 noundef %26)
  %28 = load i32, ptr %9, align 4
  %29 = call i32 (ptr, ...) @printf(ptr noundef @.str.3, i32 noundef %28)
  %30 = load i32, ptr %3, align 4
  %31 = call i32 (ptr, ...) @printf(ptr noundef @.str.4, i32 noundef %30)
  %32 = load i32, ptr %5, align 4
  %33 = call i32 (ptr, ...) @printf(ptr noundef @.str.5, i32 noundef %32)
  ret i32 0
}

declare i32 @printf(ptr noundef, ...) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 19.1.7 (/home/runner/work/llvm-project/llvm-project/clang cd708029e0b2869e80abe31ddb175f7c35361f90)"}
