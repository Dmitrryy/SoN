#include "gtest/gtest.h"

#include "Analyses/ConstantFolding.hpp"
#include "Node.hpp"

#include <iostream>
#include <unistd.h>

using namespace son;

TEST(Liveness, test_lecture) {
  ASSERT_TRUE(true);
  FunctionType fnTy(ValueType::Int32, {});
  Function F("Liveness_test_lecture", fnTy);

  // CFG
  auto N0 = F.getStart();
  auto N1 = F.getEnd();

  auto V0 = F.create<ConstantNode>(ValueType::Int32, 1);
  auto V1 = F.create<ConstantNode>(ValueType::Int32, 2);
  auto V2 = F.create<ConstantNode>(ValueType::Int32, 3);

  auto jmp_N0_to_N1 = F.create<JmpNode>(N0);
  N1->addCFInput(jmp_N0_to_N1);

  auto V3 = F.create<AddNode>(V0, V1);
  auto V4 = F.create<AddNode>(V3, V2);

  auto V5 = F.create<ShlNode>(V3, V4);
  auto V6 = F.create<XorNode>(V3, V4);

  auto V7 = F.create<AddNode>(V5, V6);

  F.create<RetNode>(N1, V7);

  EXPECT_TRUE(F.verify());

  // func i32 Liveness_test_lecture() {
  // entry:
  //   Jmp exit
  //
  // exit: /* Pred: entry */
  //   V1 = i32 10
  //   V0 = i32 1
  //   V3 = i32 Add i32 V0, i32 V1
  //   V2 = i32 20
  //   V4 = i32 Add i32 V3, i32 V2
  //   Ret void exit, i32 V4
  //
  // }
  Function::NamesMapTy Names = {{V0, "V0"}, {V1, "V1"}, {V2, "V2"}, {V3, "V3"},
                                {V4, "V4"}, {V5, "V5"}, {V6, "V6"}, {V7, "V7"}};
  auto DumpNames = Names;
  F.nameNodes(DumpNames);
  F.dump(std::cout, DumpNames);

  ConstantFolding CF;
  CF.run(F);

  // func i32 Liveness_test_lecture() {
  // entry:
  //   Jmp exit
  // 
  // exit: /* Pred: entry */
  //   %14 = i32 197
  //   Ret void exit, i32 %14
  // 
  // }
  F.nameNodes(DumpNames);
  F.dump(std::cout, DumpNames);
}
