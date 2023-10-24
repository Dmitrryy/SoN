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

TEST(Function, FunctionTest1) {
  ASSERT_TRUE(true);
  FunctionType fnTy(ValueType::Void, {});
  Function F("Hello", fnTy);

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
  auto n1Cond =
      F.createNode<CmpEQNode>(n1CounterPhi, F.createNode<CInt32Node>(10));

  // if (i == 10)
  auto n1If = F.createNode<IfNode>(range01, n1Cond);
  auto ifTrue = F.createNode<IfTrueNode>(n1If);
  auto ifFalse = F.createNode<IfFalseNode>(n1If);
  range01->addCFInput(ifFalse);

  // final
  F.getEnd()->addCFInput(ifTrue);
}

TEST(Function, FunctionTest2) {
  ASSERT_TRUE(true);
  FunctionType fnTy(ValueType::Void, {});
  Function F("Hello", fnTy);

  //                 +-----+
  //                 |  0  |
  //                 +-----+
  //                    |
  //                    V
  //                 +-----+
  //       ----------|  1  |---------
  //       |         +-----+        |
  //       |                        |
  //       V                        V
  //     +-----+     +-----+     +-----+
  //     |  2  |  ---|  3  |<----|  4  |
  //     +-----+  |  +-----+     +-----+
  //        |     |                 |
  //        |     V                 |
  //        |  +-----+     +-----+  |
  //        +->|  5  |<----|  6  |<-+
  //           +-----+     +-----+
  //

}