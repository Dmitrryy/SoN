#include "gtest/gtest.h"

#include "Function.hpp"
#include "Node.hpp"

#include <iostream>
#include <unistd.h>

using namespace son;

TEST(Peephole, test_peephole_add) {
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
      Callee, std::vector<Node *>{Caller_C0, Caller_C1});

  auto Caller_Ret = Caller.create<RetNode>(Caller_N0, Caller_Call);
  Caller_N1->addCFInput(Caller_Ret);

  {
    EXPECT_TRUE(Caller.verify());
    Function::NamesMapTy Names = {{Caller_C0, "C0"}, {Caller_C1, "C1"}};
    auto DumpNames = Names;
    Caller.nameNodes(DumpNames);
    Caller.dump(std::cout, DumpNames);
  }
}
