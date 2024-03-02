#include "gtest/gtest.h"

#include "Node.hpp"
#include "graphs.hpp"

#include <Analyses/DomTree.hpp>
#include <Analyses/Liveness.hpp>
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

  // func i32 Liveness_test_lecture() {
  // entry:
  //   V0 = i32 1
  //   V1 = i32 10
  //   Jmp 1
  //
  // 1: /* Pred: entry, 2 */
  //   V4 = i32 Phi void 1, i32 V1, i32 V8
  //   V3 = i32 Phi void 1, i32 V0, i32 V7
  //   V5 = i1 CmpEQ i32 V4, i32 V0
  //   If i1 V5, T:2, F:3
  //
  // 2: /* Pred: 1 */
  //   V8 = i32 Sub i32 V4, i32 V0
  //   V7 = i32 Mul i32 V3, i32 V4
  //   Jmp 1
  //
  // 3: /* Pred: 1 */
  //   Jmp exit
  //
  // exit: /* Pred: 3 */
  //   V2 = i32 20
  //   V9 = i32 Add i32 V2, i32 V3
  //   Ret void exit, i32 V9
  //
  // }
  auto DumpNames = Names;
  F.nameNodes(DumpNames);
  F.dump(std::cout, DumpNames);

  Liveness LV(F);
  // live numbers:
  // V0 = i32 1, live: 2
  // V1 = i32 10, live: 4
  // V2 = i32 20, live: 18
  // V3 = i32 Phi void 1, i32 V0, i32 V7, live: 6
  // V4 = i32 Phi void 1, i32 V1, i32 V8, live: 6
  // V5 = i1 CmpEQ i32 V4, i32 V0, live: 8
  // V7 = i32 Mul i32 V3, i32 V4, live: 14
  // V8 = i32 Sub i32 V4, i32 V0, live: 12
  // V9 = i32 Add i32 V2, i32 V3, live: 20
  std::cout << "live numbers:" << std::endl;
  for (auto &&n :
       std::vector<Node *>{V0, V1, V2, V3_N1Phi0, V4_N1Phi1, V5, V7, V8, V9}) {
    n->dump(std::cout, DumpNames);
    std::cout << ", live: " << LV.liveNumber(n) << std::endl;
  }
  EXPECT_EQ(LV.liveNumber(V0), 2);
  EXPECT_EQ(LV.liveNumber(V1), 4);
  EXPECT_EQ(LV.liveNumber(V2), 18);
  EXPECT_EQ(LV.liveNumber(V3_N1Phi0), 6);
  EXPECT_EQ(LV.liveNumber(V4_N1Phi1), 6);
  EXPECT_EQ(LV.liveNumber(V5), 8);
  EXPECT_EQ(LV.liveNumber(V7), 14);
  EXPECT_EQ(LV.liveNumber(V8), 12);
  EXPECT_EQ(LV.liveNumber(V9), 20);

  // live intervals:
  // V9, live interval = [20, 22)
  // V8, live interval = [12, 16)
  // V7, live interval = [14, 16)
  // V4, live interval = [6, 14)
  // V3, live interval = [6, 20)
  // V1, live interval = [4, 6)
  // V0, live interval = [2, 16)
  // 3, live interval = [16, 16)
  // V5, live interval = [8, 10)
  // 2, live interval = [10, 16)
  // V2, live interval = [18, 20)
  // 1, live interval = [6, 10)
  std::cout << "live intervals:" << std::endl;
  for (auto name : Names) {
    std::cout << name.second << ", live interval = [";
    auto LInterval = LV.liveInterval(const_cast<Node *>(name.first));
    std::cout << LInterval.first << ", " << LInterval.second << ")"
              << std::endl;
  }
  EXPECT_EQ(LV.liveInterval(V0), std::make_pair(2LU, 16LU));
  EXPECT_EQ(LV.liveInterval(V1), std::make_pair(4LU, 6LU));
  EXPECT_EQ(LV.liveInterval(V2), std::make_pair(18LU, 20LU));
  EXPECT_EQ(LV.liveInterval(V3_N1Phi0), std::make_pair(6LU, 20LU));
  EXPECT_EQ(LV.liveInterval(V4_N1Phi1), std::make_pair(6LU, 14LU));
  EXPECT_EQ(LV.liveInterval(V5), std::make_pair(8LU, 10LU));
  EXPECT_EQ(LV.liveInterval(V7), std::make_pair(14LU, 16LU));
  EXPECT_EQ(LV.liveInterval(V8), std::make_pair(12LU, 16LU));
  EXPECT_EQ(LV.liveInterval(V9), std::make_pair(20LU, 22LU));

  // live in:
  // Inputs to region: entry
  // Inputs to region: 1
  //   V0 = i32 1
  // Inputs to region: 2
  //   V0 = i32 1
  //   V4 = i32 Phi void 1, i32 V1, i32 V8
  //   V3 = i32 Phi void 1, i32 V0, i32 V7
  // Inputs to region: 3
  //   V3 = i32 Phi void 1, i32 V0, i32 V7
  // Inputs to region: exit
  //   V3 = i32 Phi void 1, i32 V0, i32 V7
  std::cout << "live in:" << std::endl;
  for (auto R : std::vector<RegionNodeBase *>{N0, N1, N2, N3, N4}) {
    std::cout << "Inputs to region: " << DumpNames[R] << std::endl;
    for (auto input : LV.liveIn(R)) {
      std::cout << "  ";
      input->dump(std::cout, DumpNames);
      std::cout << std::endl;
    }
  }
  EXPECT_TRUE(LV.liveIn(N0).empty());
  auto &&liveIn = LV.liveIn(N1);
  EXPECT_EQ(std::set(liveIn.begin(), liveIn.end()), std::set<Node *>{V0});
  auto &&liveIn_N2 = LV.liveIn(N2);
  std::set<Node *> ref_N2{V0, V3_N1Phi0, V4_N1Phi1};
  EXPECT_EQ(std::set(liveIn_N2.begin(), liveIn_N2.end()), ref_N2);
  auto &&liveIn_N3 = LV.liveIn(N3);
  EXPECT_EQ(std::set(liveIn_N3.begin(), liveIn_N3.end()), std::set<Node *>{V3_N1Phi0});
  auto &&liveIn_N4 = LV.liveIn(N4);
  EXPECT_EQ(std::set(liveIn_N4.begin(), liveIn_N4.end()), std::set<Node *>{V3_N1Phi0});
}
