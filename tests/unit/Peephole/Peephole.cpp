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

  auto C0 = F.create<ConstantNode>(ValueType::Int32, 0);
  auto V0 = F.create<ConstantNode>(ValueType::Int32, 1);
  auto V1 = F.create<ConstantNode>(ValueType::Int32, 2);
  auto V2 = F.create<ConstantNode>(ValueType::Int32, 3);

  auto jmp_N0_to_N1 = F.create<JmpNode>(N0);
  N1->addCFInput(jmp_N0_to_N1);

  auto Arg2 = F.create<AddNode>(F.getArg(0), F.getArg(0));
  auto Arg2_0 = F.create<AddNode>(C0, Arg2);
  auto V3 = F.create<AddNode>(Arg2_0, V0);
  auto V4 = F.create<AddNode>(V3, V1);
  auto V5 = F.create<AddNode>(V2, V4);

  auto Ret = F.create<RetNode>(N1, V5);

  EXPECT_TRUE(F.verify());

  // func i32 Liveness_test_lecture(i32 %0) {
  // entry:
  //   Jmp exit
  //
  // exit: /* Pred: entry */
  //   %0 = i32 FunctionArg(0)
  //   Arg2 = i32 Add i32 %0, i32 %0
  //   %1 = i32 0
  //   %7 = i32 Add i32 %1, i32 Arg2
  //   V0 = i32 1
  //   V3 = i32 Add i32 %7, i32 V0
  //   V1 = i32 2
  //   V4 = i32 Add i32 V3, i32 V1
  //   V2 = i32 3
  //   V5 = i32 Add i32 V2, i32 V4
  //   Ret void exit, i32 V5
  //
  // }
  Function::NamesMapTy Names = {{V0, "V0"},    {V1, "V1"}, {V2, "V2"},
                                {V3, "V3"},    {V4, "V4"}, {V5, "V5"},
                                {Arg2, "Arg2"}};
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
  //   %12 = i32 1
  //   V2 = i32 3
  //   V1 = i32 2
  //   V0 = i32 1
  //   %0 = i32 FunctionArg(0)
  //   %13 = i32 Shl i32 %0, i32 %12
  //   V4 = i32 Add i32 V2, i32 V1
  //   V3 = i32 Add i32 V0, i32 %13
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
  //   %12 = i32 1
  //   %0 = i32 FunctionArg(0)
  //   %15 = i32 6
  //   %13 = i32 Shl i32 %0, i32 %12
  //   V5 = i32 Add i32 %15, i32 %13
  //   Ret void exit, i32 V5
  //
  // }
  F.nameNodes(DumpNames);
  F.dump(std::cout, DumpNames);

  EXPECT_TRUE(isa<AddNode>(Ret->getRetValue()));
  auto *retAdd = static_cast<AddNode *>(Ret->getRetValue());
  EXPECT_TRUE(isa<ShlNode>(retAdd->operand(0)) ||
              isa<ShlNode>(retAdd->operand(1)));
  EXPECT_TRUE(isa<ConstantNode>(retAdd->operand(0)) ||
              isa<ConstantNode>(retAdd->operand(1)));
}

TEST(Peephole, test_peephole_xor) {
  ASSERT_TRUE(true);
  FunctionType fnTy(ValueType::Int32, {ValueType::Int32});
  Function F("Liveness_test_lecture", fnTy);

  // CFG
  auto N0 = F.getStart();
  auto N1 = F.getEnd();

  auto C0 = F.create<ConstantNode>(ValueType::Int32, 0);
  auto C3 = F.create<ConstantNode>(ValueType::Int32, 3);

  auto jmp_N0_to_N1 = F.create<JmpNode>(N0);
  N1->addCFInput(jmp_N0_to_N1);

  auto V1 = F.create<XorNode>(F.getArg(0), F.getArg(0));
  auto V2 = F.create<XorNode>(F.getArg(0), V1);
  auto V3 = F.create<XorNode>(V2, C0);
  auto V4 = F.create<XorNode>(V3, C3);

  auto Ret = F.create<RetNode>(N1, V4);

  EXPECT_TRUE(F.verify());

  // func i32 Liveness_test_lecture(i32 %0) {
  // entry:
  //   Jmp exit
  //
  // exit: /* Pred: entry */
  //   %0 = i32 FunctionArg(0)
  //   V1 = i32 Xor i32 %0, i32 %0
  //   V2 = i32 Xor i32 %0, i32 V1
  //   C0 = i32 0
  //   V3 = i32 Xor i32 V2, i32 C0
  //   C3 = i32 3
  //   V4 = i32 Xor i32 V3, i32 C3
  //   Ret void exit, i32 V4
  //
  // }
  Function::NamesMapTy Names = {{C0, "C0"}, {C3, "C3"}, {V1, "V1"},
                                {V2, "V2"}, {V3, "V3"}, {V4, "V4"}};
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
  //   %0 = i32 FunctionArg(0)
  //   %9 = i32 0
  //   V2 = i32 Xor i32 %0, i32 %9
  //   C3 = i32 3
  //   V4 = i32 Xor i32 V2, i32 C3
  //   Ret void exit, i32 V4
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
  //   C3 = i32 3
  //   %0 = i32 FunctionArg(0)
  //   V4 = i32 Xor i32 %0, i32 C3
  //   Ret void exit, i32 V4
  //
  // }
  F.nameNodes(DumpNames);
  F.dump(std::cout, DumpNames);

  EXPECT_TRUE(isa<XorNode>(Ret->getRetValue()));
  auto *retXor = static_cast<XorNode *>(Ret->getRetValue());
  EXPECT_TRUE(isa<FunctionArgNode>(retXor->operand(0)) ||
              isa<FunctionArgNode>(retXor->operand(1)));
  EXPECT_TRUE(isa<ConstantNode>(retXor->operand(0)) ||
              isa<ConstantNode>(retXor->operand(1)));
}
