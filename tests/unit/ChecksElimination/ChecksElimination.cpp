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


  EXPECT_TRUE(F.verify());
  {
    Function::NamesMapTy Names = {
        {F_Use2, "Use2"}, {F_Use3, "Use3"}, {F_B4, "B4"}, {F_B5, "B5"}};
    auto DumpNames = Names;
    F.nameNodes(DumpNames);
    F.dump(std::cout, DumpNames);
  }
}
