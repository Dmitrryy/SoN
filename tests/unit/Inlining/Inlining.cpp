#include "gtest/gtest.h"

#include "Function.hpp"
#include "Node.hpp"
#include "Optimizations/Inlining.hpp"

#include <iostream>
#include <unistd.h>

using namespace son;

TEST(Inlining, test_seminar) {
  // Callee
  //=------
  FunctionType CalleeFnTy(ValueType::Int32,
                          {ValueType::Int32, ValueType::Int32});
  Function Callee("Callee", CalleeFnTy);

  auto Callee_N0 = Callee.getStart();
  auto Callee_N1 = Callee.getEnd();

  auto Callee_C0 = Callee.create<ConstantNode>(ValueType::Int32, 1);
  auto Callee_C1 = Callee.create<ConstantNode>(ValueType::Int32, 10);

  auto Callee_Use2 = Callee.create<MulNode>(Callee.getArg(0), Callee_C0);
  auto Callee_Use3 = Callee.create<AddNode>(Callee.getArg(1), Callee_C1);

  auto Callee_Cond_Const = Callee.create<ConstantNode>(ValueType::Int32, 100);
  auto Callee_Cond = Callee.create<CmpLTNode>(Callee_Use2, Callee_Cond_Const);

  auto Callee_If = Callee.create<IfNode>(Callee_N0, Callee_Cond);
  auto Callee_ifTrue = Callee.create<IfTrueNode>(Callee_If);
  auto Callee_ifFalse = Callee.create<IfFalseNode>(Callee_If);

  // block 5
  auto Callee_B5 = Callee.create<RegionNode>();
  Callee_B5->addCFInput(Callee_ifFalse);
  auto Callee_B5_Ret = Callee.create<RetNode>(Callee_B5, Callee_Use2);
  Callee_N1->addCFInput(Callee_B5_Ret);

  // block 4
  auto Callee_B4 = Callee.create<RegionNode>();
  Callee_B4->addCFInput(Callee_ifTrue);
  auto Callee_B4_Ret = Callee.create<RetNode>(Callee_B4, Callee_Use3);
  Callee_N1->addCFInput(Callee_B4_Ret);

  // func i32 Callee(i32 %0, i32 %1) {
  // entry:
  //   %2 = i32 1
  //   %0 = i32 FunctionArg(0)
  //   %6 = i32 100
  //   Use2 = i32 Mul i32 %0, i32 %2
  //   %7 = i1 CmpLT i32 Use2, i32 %6
  //   If i1 %7, T:B4, F:B5
  // 
  // B5: /* Pred: entry */
  //   Ret void B5, i32 Use2
  // 
  // B4: /* Pred: entry */
  //   %3 = i32 10
  //   %1 = i32 FunctionArg(1)
  //   Use3 = i32 Add i32 %1, i32 %3
  //   Ret void B4, i32 Use3
  // 
  // exit:
  // 
  // }
  EXPECT_TRUE(Callee.verify());
  {
    Function::NamesMapTy Names = {{Callee_Use2, "Use2"},
                                  {Callee_Use3, "Use3"},
                                  {Callee_B4, "B4"},
                                  {Callee_B5, "B5"}};
    auto DumpNames = Names;
    Callee.nameNodes(DumpNames);
    Callee.dump(std::cout, DumpNames);
  }

  // Caller
  //=------
  FunctionType CallerFnTy(ValueType::Int32, {});
  Function Caller("Caller", CallerFnTy);

  auto Caller_N0 = Caller.getStart();
  auto Caller_N1 = Caller.getEnd();

  auto Caller_C0 = Caller.create<ConstantNode>(ValueType::Int32, 1);
  auto Caller_C1 = Caller.create<ConstantNode>(ValueType::Int32, 5);

  auto Caller_Call = Caller.create<CallNode>(
      Caller_N0, Callee, std::vector<Node *>{Caller_C0, Caller_C1});

  auto Caller_Rer_Reg = Caller.create<RegionNode>();
  Caller_Rer_Reg->addCFInput(Caller_Call);

  auto Caller_Ret = Caller.create<RetNode>(Caller_Rer_Reg, Caller_Call);
  Caller_N1->addCFInput(Caller_Ret);

  // func i32 Caller() {
  // entry:
  //   C1 = i32 5
  //   C0 = i32 1
  //   %2 = i32 Call i32 C0, i32 C1, void entry
  // 
  // %3: /* Pred: entry */
  //   Ret void %3, i32 %2
  // 
  // exit:
  // 
  // }
  EXPECT_TRUE(Caller.verify());
  {
    Function::NamesMapTy Names = {{Caller_C0, "C0"}, {Caller_C1, "C1"}};
    auto DumpNames = Names;
    Caller.nameNodes(DumpNames);
    Caller.dump(std::cout, DumpNames);
  }

  // Inline
  Inlining Inliner;
  Inliner.run(Caller);

  // func i32 Caller() {
  // entry:
  //   %10 = i32 1
  //   C0 = i32 1
  //   %14 = i32 100
  //   %12 = i32 Mul i32 C0, i32 %10
  //   %15 = i1 CmpLT i32 %12, i32 %14
  //   If i1 %15, T:%20, F:%19
  // 
  // %20: /* Pred: entry */
  //   %11 = i32 10
  //   C1 = i32 5
  //   %13 = i32 Add i32 C1, i32 %11
  //   Jmp %5
  // 
  // %19: /* Pred: entry */
  //   Jmp %5
  // 
  // %5: /* Pred: %19, %20 */
  //   %6 = i32 Phi void %5, i32 %12, i32 %13
  //   Jmp %3
  // 
  // %3: /* Pred: %5 */
  //   Ret void %3, i32 %6
  // 
  // exit:
  // 
  // }
  EXPECT_TRUE(Caller.verify());
  {
    Function::NamesMapTy Names = {{Caller_C0, "C0"}, {Caller_C1, "C1"}};
    auto DumpNames = Names;
    Caller.nameNodes(DumpNames);
    Caller.dump(std::cout, DumpNames);
  }
}
