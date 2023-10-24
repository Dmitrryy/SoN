#include "Node.hpp"
#include "gtest/gtest.h"

#include <Function.hpp>
using namespace son;

TEST(Function, FunctionEmpty) {
  ASSERT_TRUE(true);
  FunctionType fnTy(ValueType::Void, {ValueType::Int32});
  Function F("Hello", fnTy);

  EXPECT_NE(F.getStart(), nullptr);
  EXPECT_NE(F.getEnd(), nullptr);
  EXPECT_EQ(F.getNumArgs(), 1);
  EXPECT_NE(F.getArg(0), nullptr);
}

TEST(Function, FunctionTest0) {
  ASSERT_TRUE(true);
  FunctionType fnTy(ValueType::Void, {});
  Function F("Test0", fnTy);

  //                 +-----+
  //                 |  0  |
  //                 +-----+
  //                    |
  //                    V
  //                 +-----+    +-----+
  //          +----->|  1  |--->|  2  |
  //          |      +-----+    +-----+
  //          |         |
  //          |         V
  //          |      +-----+
  //          -------|  3  |
  //                 +-----+

  auto n0 = F.getStart();
  auto n2 = F.getEnd();

  auto range01 = F.createNode<RangeNode>();
  range01->addCFInput(n0);

  auto n1Counter0 = F.createNode<CInt32Node>(0);
  auto n1CounterPhi = F.createNode<PhiNode>(range01, 2, ValueType::Int32);
  n1CounterPhi->setInput(0, n1Counter0);
  auto n1Counter3 =
      F.createNode<AddNode>(n1CounterPhi, F.createNode<CInt32Node>(1));
  n1CounterPhi->setInput(1, n1Counter3);

  // cond (i == 10)
  auto loopLastCount = F.createNode<CInt32Node>(10);
  auto n1Cond = F.createNode<CmpEQNode>(n1CounterPhi, loopLastCount);

  // if (i == 10)
  auto n1If = F.createNode<IfNode>(range01, n1Cond);
  auto ifTrue = F.createNode<IfTrueNode>(n1If);
  auto ifFalse = F.createNode<IfFalseNode>(n1If);
  range01->addCFInput(ifFalse);

  // final
  F.getEnd()->addCFInput(ifTrue);

  //=-------------------------------------------
  // check CFG
  // 0: start
  EXPECT_EQ(F.getStart()->getNumOperands(), 0);
  EXPECT_EQ(F.getStart()->getNumUses(), 1);
  EXPECT_EQ(F.getStart()->getUses().at(0), range01);
  EXPECT_EQ(range01->getOperand(0), F.getStart());
  // 0-3-1: check range + phi
  EXPECT_EQ(range01->getNumOperands(), 2);
  EXPECT_EQ(range01->getOperand(1), ifFalse);
  //                    inpuCF + inputVal1 + inputVal
  EXPECT_EQ(n1CounterPhi->getNumOperands(), 3);
  EXPECT_EQ(n1CounterPhi->getOperand(0), range01);
  EXPECT_EQ(n1CounterPhi->getOperand(1), n1Counter0); // init
  EXPECT_EQ(n1CounterPhi->getOperand(2), n1Counter3); // loop
  //   uses
  EXPECT_EQ(range01->getNumUses(), 2);
  EXPECT_EQ(range01->getUses().at(0), n1CounterPhi);
  EXPECT_EQ(range01->getUses().at(1), n1If);
  EXPECT_EQ(n1CounterPhi->getNumUses(), 2);
  EXPECT_EQ(n1CounterPhi->getUses().at(0), n1Counter3);
  EXPECT_EQ(n1CounterPhi->getUses().at(1), n1Cond);
  // if
  EXPECT_EQ(n1If->getNumOperands(), 2);
  EXPECT_EQ(n1If->getOperand(0), range01);
  EXPECT_EQ(n1If->getOperand(1), n1Cond);
  EXPECT_EQ(n1If->getNumUses(), 2);
  EXPECT_EQ(n1If->getUses().at(0), ifTrue);
  EXPECT_EQ(n1If->getUses().at(1), ifFalse);
  // cond
  EXPECT_EQ(n1Cond->getNumOperands(), 2);
  EXPECT_EQ(n1Cond->getOperand(0), n1CounterPhi);
  EXPECT_EQ(n1Cond->getOperand(1), loopLastCount);
  EXPECT_EQ(n1Cond->getNumUses(), 1);
  EXPECT_EQ(n1Cond->getUses().at(0), n1If);
  // end
  EXPECT_EQ(F.getEnd()->getNumOperands(), 1);
  EXPECT_EQ(F.getEnd()->getOperand(0), ifTrue);
  EXPECT_EQ(F.getEnd()->getNumUses(), 0);
}

TEST(Function, FunctionTest1) {
  ASSERT_TRUE(true);
  FunctionType fnTy(ValueType::Void, {ValueType::Int32});
  Function F("Test1", fnTy);

  //                 +-----+
  //                 |  A  |
  //                 +-----+
  //                    |
  //                    V
  //                 +-----+
  //       ----------|  B  |---------
  //       |         +-----+        |
  //       |                        |
  //       V                        V
  //     +-----+     +-----+     +-----+
  //     |  C  |  ---|  E  |<----|  F  |
  //     +-----+  |  +-----+     +-----+
  //        |     |                 |
  //        |     V                 |
  //        |  +-----+     +-----+  |
  //        +->|  D  |<----|  G  |<-+
  //           +-----+     +-----+
  //
  // A - start
  // D - end
  auto A = F.getStart();
  auto D = F.getEnd();

  auto BIfRhs = F.createNode<CInt32Node>(66);
  auto BCond = F.createNode<CmpLENode>(F.getArg(0), BIfRhs);

  // B if
  auto BIf = F.createNode<IfNode>(A, BCond);
  auto BIfTrue = F.createNode<IfTrueNode>(BIf);
  auto BIfFalse = F.createNode<IfFalseNode>(BIf);
  // C
  auto CMulTimes = F.createNode<CInt32Node>(5);
  auto CRetVal = F.createNode<MulNode>(F.getArg(0), CMulTimes);

  // F cond
  auto FIfRhs = F.createNode<CInt32Node>(-10);
  auto FCond = F.createNode<CmpGENode>(F.getArg(0), BIfRhs);
  // F if
  auto FIf = F.createNode<IfNode>(BIfFalse, FCond);
  auto FIfTrue = F.createNode<IfTrueNode>(FIf);
  auto FIfFalse = F.createNode<IfFalseNode>(FIf);

  // E
  auto ERhs = F.createNode<CInt32Node>(5);
  auto ERetVal = F.createNode<RemNode>(F.getArg(0), ERhs);

  // G
  auto GRhs = F.createNode<CInt32Node>(3);
  auto GRetVal = F.createNode<DivNode>(F.getArg(0), ERhs);

  // final phi for ret value at D node
  //   range
  auto DRange = F.createNode<RangeNode>();
  DRange->addCFInput(BIfTrue);  // C
  DRange->addCFInput(FIfTrue);  // E
  DRange->addCFInput(FIfFalse); // G
  //   phi
  auto DPhi = F.createNode<PhiNode>(DRange, 3, ValueType::Int32);
  DPhi->setInput(0, CRetVal);
  DPhi->setInput(1, ERetVal);
  DPhi->setInput(2, GRetVal);

  // ret
  auto ret = F.createNode<RetNode>(DPhi);

  // final
  F.getEnd()->addCFInput(ret);
}