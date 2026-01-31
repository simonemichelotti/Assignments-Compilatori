; ModuleID = 'test/Esperimento.opt.bc'
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
  store i32 %10, ptr %6, align 4
  %11 = load i32, ptr %2, align 4
  store i32 %11, ptr %7, align 4
  %12 = load i32, ptr %2, align 4
  %13 = shl i32 %12, 4
  %14 = sub i32 %13, %12
  store i32 %14, ptr %8, align 4
  %15 = load i32, ptr %2, align 4
  %16 = ashr i32 %15, 3
  store i32 %16, ptr %9, align 4
  %17 = load i32, ptr %4, align 4
  %18 = add nsw i32 %17, 1
  store i32 %18, ptr %3, align 4
  %19 = load i32, ptr %3, align 4
  %20 = sub nsw i32 %19, 1
  store i32 %20, ptr %5, align 4
  %21 = load i32, ptr %6, align 4
  %22 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %21)
  %23 = load i32, ptr %7, align 4
  %24 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %23)
  %25 = load i32, ptr %8, align 4
  %26 = call i32 (ptr, ...) @printf(ptr noundef @.str.2, i32 noundef %25)
  %27 = load i32, ptr %9, align 4
  %28 = call i32 (ptr, ...) @printf(ptr noundef @.str.3, i32 noundef %27)
  %29 = load i32, ptr %3, align 4
  %30 = call i32 (ptr, ...) @printf(ptr noundef @.str.4, i32 noundef %29)
  %31 = load i32, ptr %5, align 4
  %32 = call i32 (ptr, ...) @printf(ptr noundef @.str.5, i32 noundef %31)
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
