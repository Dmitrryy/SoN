#include "gtest/gtest.h"

#include "Node.hpp"
#include "Optimizations/ConstantFolding.hpp"
#include "Optimizations/Peephole.hpp"

#include <iostream>
#include <unistd.h>

using namespace son;

TEST(Peephole, test_peephole_add) {
  ASSERT_TRUE(true);
  FunctionType fnTy(ValueType::Int32, {ValueType::Int32});
  Function F("Liveness_test_lecture", fnTy);

  // CFG
  auto N0 = F.getStart();
  auto N1 = F.getEnd();

  auto V0 = F.create<ConstantNode>(ValueType::Int32, 1);
  auto V1 = F.create<ConstantNode>(ValueType::Int32, 2);
  auto V2 = F.create<ConstantNode>(ValueType::Int32, 3);

  auto jmp_N0_to_N1 = F.create<JmpNode>(N0);
  N1->addCFInput(jmp_N0_to_N1);

  auto V3 = F.create<AddNode>(F.getArg(0), V0);
  auto V4 = F.create<AddNode>(V3, V1);
  auto V5 = F.create<AddNode>(V4, V2);

  auto Ret = F.create<RetNode>(N1, V5);

  EXPECT_TRUE(F.verify());

  // func i32 Liveness_test_lecture(i32 %0) {
  // entry:
  //   Jmp exit
  //
  // exit: /* Pred: entry */
  //   V0 = i32 1
  //   %0 = i32 FunctionArg(0)
  //   V3 = i32 Add i32 %0, i32 V0
  //   V1 = i32 2
  //   V4 = i32 Add i32 V3, i32 V1
  //   V2 = i32 3
  //   V5 = i32 Add i32 V4, i32 V2
  //   Ret void exit, i32 V5
  //
  // }
  Function::NamesMapTy Names = {{V0, "V0"}, {V1, "V1"}, {V2, "V2"},
                                {V3, "V3"}, {V4, "V4"}, {V5, "V5"}};
  auto DumpNames = Names;
  F.nameNodes(DumpNames);
  F.dump(std::cout, DumpNames);

  Peephole PH;
  ConstantFolding CF;
  PH.run(F);

  // func i32 Liveness_test_lecture(i32 %0) {
  // entry:
  //   Jmp exit
  //
  // exit: /* Pred: entry */
  //   V2 = i32 3
  //   V1 = i32 2
  //   V0 = i32 1
  //   %0 = i32 FunctionArg(0)
  //   V4 = i32 Add i32 V2, i32 V1
  //   V3 = i32 Add i32 %0, i32 V0
  //   V5 = i32 Add i32 V4, i32 V3
  //   Ret void exit, i32 V5
  //
  // }
  F.nameNodes(DumpNames);
  F.dump(std::cout, DumpNames);

  CF.run(F);
  PH.run(F);
  CF.run(F);

  // func i32 Liveness_test_lecture(i32 %0) {
  // entry:
  //   Jmp exit
  //
  // exit: /* Pred: entry */
  //   %0 = i32 FunctionArg(0)
  //   %10 = i32 6
  //   V5 = i32 Add i32 %0, i32 %10
  //   Ret void exit, i32 V5
  //
  // }
  F.nameNodes(DumpNames);
  F.dump(std::cout, DumpNames);

  EXPECT_TRUE(isa<AddNode>(Ret->getRetValue()));
  auto *retAdd = static_cast<AddNode *>(Ret->getRetValue());
  EXPECT_TRUE(isa<FunctionArgNode>(retAdd->operand(0)) || isa<FunctionArgNode>(retAdd->operand(1)));
  EXPECT_TRUE(isa<ConstantNode>(retAdd->operand(0)) || isa<ConstantNode>(retAdd->operand(1)));
}
