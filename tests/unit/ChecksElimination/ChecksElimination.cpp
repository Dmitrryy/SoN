#include "gtest/gtest.h"

#include "Function.hpp"
#include "Node.hpp"
#include "Opcodes.hpp"

#include <iostream>
#include <unistd.h>

using namespace son;

TEST(ChecksElimination, zerocheck_0) {
  // F
  //=------
  FunctionType FFnTy(ValueType::Int32, {ValueType::Int32, ValueType::Int32});
  Function F("F", FFnTy);

  auto F_N0 = F.getStart();
  auto F_N1 = F.getEnd();

  auto F_C0 = F.create<ConstantNode>(ValueType::Int32, 1);
  auto F_C1 = F.create<ConstantNode>(ValueType::Int32, 10);

  auto F_Use2 = F.create<MulNode>(F.getArg(0), F_C0);
  auto F_Use3 = F.create<AddNode>(F.getArg(1), F_C1);

  auto F_Cond_Const = F.create<ConstantNode>(ValueType::Int32, 100);
  auto F_Cond = F.create<CmpLTNode>(F_Use2, F_Cond_Const);

  auto firstCheck = F.create<CallBuiltinNode>(
      F_N0, "nullCheck", ValueType::Void, std::vector<Node *>{F_Use3});
  auto FirstCheckReg = F.create<RegionNode>();
  FirstCheckReg->addCFInput(firstCheck);

  auto F_If = F.create<IfNode>(FirstCheckReg, F_Cond);
  auto F_ifTrue = F.create<IfTrueNode>(F_If);
  auto F_ifFalse = F.create<IfFalseNode>(F_If);

  // block 5
  auto F_B5_0 = F.create<RegionNode>();
  F_B5_0->addCFInput(F_ifFalse);
  auto secondCheck = F.create<CallBuiltinNode>(
      F_B5_0, "nullCheck", ValueType::Void, std::vector<Node *>{F_Use3});

  auto F_B5 = F.create<RegionNode>();
  F_B5->addCFInput(secondCheck);
  auto F_B5_Ret = F.create<RetNode>(F_B5, F_Use2);
  F_N1->addCFInput(F_B5_Ret);

  // block 4
  auto F_B4 = F.create<RegionNode>();
  F_B4->addCFInput(F_ifTrue);
  auto F_B4_Ret = F.create<RetNode>(F_B4, F_Use3);
  F_N1->addCFInput(F_B4_Ret);

  // func i32 F(i32 %0, i32 %1) {
  // entry:
  //   %3 = i32 10
  //   %1 = i32 FunctionArg(1)
  //   Use3 = i32 Add i32 %1, i32 %3
  //   CallBuiltin "nullCheck"( i32 Use3) |-> %9
  // 
  // %9: /* Pred: entry */
  //   %2 = i32 1
  //   %0 = i32 FunctionArg(0)
  //   %6 = i32 100
  //   Use2 = i32 Mul i32 %0, i32 %2
  //   %7 = i1 CmpLT i32 Use2, i32 %6
  //   If i1 %7, T:B4, F:%13
  // 
  // %13: /* Pred: %9 */
  //   CallBuiltin "nullCheck"( i32 Use3) |-> B5
  // 
  // B5: /* Pred: %13 */
  //   Ret void B5, i32 Use2
  // 
  // B4: /* Pred: %9 */
  //   Ret void B4, i32 Use3
  // 
  // exit:
  // 
  // }
  EXPECT_TRUE(F.verify());
  {
    Function::NamesMapTy Names = {
        {F_Use2, "Use2"}, {F_Use3, "Use3"}, {F_B4, "B4"}, {F_B5, "B5"}};
    auto DumpNames = Names;
    F.nameNodes(DumpNames);
    F.dump(std::cout, DumpNames);
  }
}
