#include "gtest/gtest.h"

#include "Node.hpp"

#include <Analyses/DomTree.hpp>
#include <Analyses/Liveness.hpp>
#include <Analyses/Loop.hpp>
#include <Analyses/RegAlloc.hpp>
#include <Function.hpp>
#include <iostream>
#include <unistd.h>

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
  Function::NamesMapTy Names = {
      {N1, "1"},  {N2, "2"},  {N3, "3"},         {V0, "V0"},
      {V1, "V1"}, {V2, "V2"}, {V3_N1Phi0, "V3"}, {V4_N1Phi1, "V4"},
      {V5, "V5"}, {V7, "V7"}, {V8, "V8"},        {V9, "V9"}};
  auto DumpNames = Names;
  F.nameNodes(DumpNames);
  F.dump(std::cout, DumpNames);

  Liveness LV(F);
  RegAlloc RA(F, 3, LV);

  // clang-format off
  //  |  0|  1|  2|  3|  4|  5|  6|  7|  8|  9| 10| 11| 12| 13| 14| 15| 16| 17| 18| 19| 20| 21|
  // 0|   |   | r0| r0| r0| r0| r0| r0| r0| r0| r0| r0| r0| r0| r0| r0| r0|   |   |   |   |   |V0 = i32 1
  // 1|   |   |   |   | r1| r1| r1|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |V1 = i32 10
  // 2|   |   |   |   |   |   | s0| s0| s0| s0| s0| s0| s0| s0| s0| s0| s0| s0| s0| s0| s0|   |V3 = i32 Phi void 1, i32 V0, i32 V7
  // 3|   |   |   |   |   |   | r1| r1| r1| r1| r1| r1| r1| r1| r1|   |   |   |   |   |   |   |V4 = i32 Phi void 1, i32 V1, i32 V8
  // 4|   |   |   |   |   |   |   |   | r2| r2| r2|   |   |   |   |   |   |   |   |   |   |   |V5 = i1 CmpEQ i32 V4, i32 V0
  // 5|   |   |   |   |   |   |   |   |   |   |   |   | r2| r2| r2| r2| r2|   |   |   |   |   |V8 = i32 Sub i32 V4, i32 V0
  // 6|   |   |   |   |   |   |   |   |   |   |   |   |   |   | r1| r1| r1|   |   |   |   |   |V7 = i32 Mul i32 V3, i32 V4
  // 7|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | r1| r1| r1|   |V2 = i32 20
  // 8|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | r1| r1|V9 = i32 Add i32 V2, i32 V3
  // clang-format on
  RA.dump(DumpNames);

  auto &&V0_info = RA.info(V0);
  auto &&V1_info = RA.info(V1);
  auto &&V2_info = RA.info(V2);
  auto &&V3_info = RA.info(V3_N1Phi0);
  auto &&V4_info = RA.info(V4_N1Phi1);
  auto &&V5_info = RA.info(V5);
  auto &&V7_info = RA.info(V7);
  auto &&V8_info = RA.info(V8);
  auto &&V9_info = RA.info(V9);

  EXPECT_FALSE(V0_info.isSpill);
  EXPECT_FALSE(V1_info.isSpill);
  EXPECT_FALSE(V2_info.isSpill);
  EXPECT_TRUE(V3_info.isSpill);
  EXPECT_FALSE(V4_info.isSpill);
  EXPECT_FALSE(V5_info.isSpill);
  EXPECT_FALSE(V7_info.isSpill);
  EXPECT_FALSE(V8_info.isSpill);
  EXPECT_FALSE(V9_info.isSpill);

  // stack locations
  EXPECT_EQ(V3_info.regIdxOrStackLockation, 0);

  // reg idxs
  EXPECT_EQ(V0_info.regIdxOrStackLockation, 0);
  EXPECT_EQ(V1_info.regIdxOrStackLockation, 1);
  EXPECT_EQ(V2_info.regIdxOrStackLockation, 1);
  EXPECT_EQ(V4_info.regIdxOrStackLockation, 1);
  EXPECT_EQ(V5_info.regIdxOrStackLockation, 2);
  EXPECT_EQ(V7_info.regIdxOrStackLockation, 1);
  EXPECT_EQ(V8_info.regIdxOrStackLockation, 2);
  EXPECT_EQ(V9_info.regIdxOrStackLockation, 1);
}
