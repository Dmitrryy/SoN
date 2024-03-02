#include "gtest/gtest.h"

#include "Node.hpp"
#include "graphs.hpp"

#include <Analyses/DomTree.hpp>
#include <Analyses/Loop.hpp>
#include <Function.hpp>
#include <iostream>

using namespace son;

TEST(Liveness, test_lecture) {
  ASSERT_TRUE(true);
  FunctionType fnTy(ValueType::Int32, {});
  Function F("Liveness_test_lecture", fnTy);

  // CFG
  auto N0 = F.getStart();
  auto N1 = F.create<RegionNode>();
  auto N2 = F.create<RegionNode>();
  auto N3 = F.create<RegionNode>();
  auto N4 = F.getEnd();

  auto V0 = F.create<ConstantNode>(ValueType::Int32, 1);
  auto V1 = F.create<ConstantNode>(ValueType::Int32, 10);
  auto V2 = F.create<ConstantNode>(ValueType::Int32, 20);

  auto jmp_N0_to_N1 = F.create<JmpNode>(N0);
  auto jmp_N2_to_N1 = F.create<JmpNode>(N2);
  auto jmp_N3_to_N4 = F.create<JmpNode>(N3);

  N1->addCFInput(jmp_N0_to_N1);
  N1->addCFInput(jmp_N2_to_N1);

  N4->addCFInput(jmp_N3_to_N4);

  auto V3_N1Phi0 = F.create<PhiNode>(N1, 2, ValueType::Int32);
  auto V4_N1Phi1 = F.create<PhiNode>(N1, 2, ValueType::Int32);

  auto V5 = F.create<CmpEQNode>(V4_N1Phi1, V0);

  auto N1_if = F.create<IfNode>(N1, V5);
  auto N1_ifTrue = F.create<IfTrueNode>(N1_if);
  auto N1_ifFalse = F.create<IfFalseNode>(N1_if);

  N2->addCFInput(N1_ifTrue);
  N3->addCFInput(N1_ifFalse);

  // N2 vals
  auto V7 = F.create<MulNode>(V3_N1Phi0, V4_N1Phi1);
  auto V8 = F.create<SubNode>(V4_N1Phi1, V0);

  // populate phi nodes
  V3_N1Phi0->setVal(0, V0);
  V3_N1Phi0->setVal(1, V7);
  V4_N1Phi1->setVal(0, V1);
  V4_N1Phi1->setVal(1, V8);

  // N3 vals
  auto V9 = F.create<AddNode>(V2, V3_N1Phi0);

  F.create<RetNode>(N4, V9);

  EXPECT_TRUE(F.verify());

  Function::NamesMapTy Names = {
      {N1, "1"},  {N2, "2"},  {N3, "3"},         {V0, "V0"},
      {V1, "V1"}, {V2, "V2"}, {V3_N1Phi0, "V3"}, {V4_N1Phi1, "V4"},
      {V5, "V5"}, {V7, "V7"}, {V8, "V8"},        {V9, "V9"}};
  F.nameNodes(Names);
  F.dump(std::cout, Names);  

  // live numbers
  DomTree DT(F);
  auto &&DataMap = F.dataSchedule(DT);
  auto &&dfsResult = F.dfs();
  auto &&LI = LoopInfo::loopAnalyze(F, dfsResult, DT);
  auto &&linRegs = F.linearize(DT, dfsResult, LI);
  auto &&liveNums = F.liveNumbers(linRegs, DataMap);

  for (auto &&p: liveNums) {
    p.first->dump(std::cout, Names);
    std::cout << ", live: " << p.second << std::endl;
  }
}
