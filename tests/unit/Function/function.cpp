#include "gtest/gtest.h"

#include "graphs.hpp"

#include <Function.hpp>
#include <memory>
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

std::unique_ptr<Function> buildTest0() {
  FunctionType fnTy(ValueType::Void, {});
  auto &&fPtr = std::make_unique<Function>("Test0", fnTy);
  auto &F = *fPtr;

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

  auto range01 = F.create<RegionNode>();
  auto noJmp = F.create<JmpNode>(n0);
  range01->addCFInput(noJmp);

  auto n1Counter0 = F.create<ConstantNode>(ValueType::Int32, 0);
  auto n1CounterPhi = F.create<PhiNode>(range01, 2, ValueType::Int32);
  n1CounterPhi->setVal(0, n1Counter0);
  auto n1Counter3 = F.create<AddNode>(
      n1CounterPhi, F.create<ConstantNode>(ValueType::Int32, 1));
  n1CounterPhi->setVal(1, n1Counter3);

  // cond (i == 10)
  auto loopLastCount = F.create<ConstantNode>(ValueType::Int32, 10);
  auto n1Cond = F.create<CmpEQNode>(n1CounterPhi, loopLastCount);

  // if (i == 10)
  auto n1If = F.create<IfNode>(range01, n1Cond);
  auto ifTrue = F.create<IfTrueNode>(n1If);
  auto ifFalse = F.create<IfFalseNode>(n1If);
  auto loopBodyRegion = F.create<RegionNode>();
  loopBodyRegion->addCFInput(ifFalse);
  // Jmp loopBody -> loopHeader
  auto backEdge = F.create<JmpNode>(loopBodyRegion);
  range01->addCFInput(backEdge);

  // final
  F.getEnd()->addCFInput(ifTrue);

  //=-------------------------------------------
  // check CFG
  // 0: start
  EXPECT_EQ(F.getStart()->opCount(), 0);
  EXPECT_EQ(F.getStart()->usersCount(), 1);
  EXPECT_EQ(F.getStart()->users().count(noJmp), 1);
  EXPECT_EQ(range01->operand(0), noJmp);
  // 0-3-1: check range + phi
  EXPECT_EQ(range01->opCount(), 2);
  EXPECT_EQ(range01->operand(1), backEdge);
  //                    inpuCF + inputVal1 + inputVal
  EXPECT_EQ(n1CounterPhi->opCount(), 3);
  EXPECT_EQ(n1CounterPhi->operand(0), range01);
  EXPECT_EQ(n1CounterPhi->operand(1), n1Counter0); // init
  EXPECT_EQ(n1CounterPhi->operand(2), n1Counter3); // loop
  //   uses
  EXPECT_EQ(range01->usersCount(), 2);
  EXPECT_EQ(range01->users().count(n1CounterPhi), 1);
  EXPECT_EQ(range01->users().count(n1If), 1);
  EXPECT_EQ(n1CounterPhi->usersCount(), 2);
  EXPECT_EQ(n1CounterPhi->users().count(n1Counter3), 1);
  EXPECT_EQ(n1CounterPhi->users().count(n1Cond), 1);
  // if
  EXPECT_EQ(n1If->opCount(), 2);
  EXPECT_EQ(n1If->operand(0), range01);
  EXPECT_EQ(n1If->operand(1), n1Cond);
  EXPECT_EQ(n1If->usersCount(), 2);
  EXPECT_EQ(n1If->users().count(ifTrue), 1);
  EXPECT_EQ(n1If->users().count(ifFalse), 1);
  // cond
  EXPECT_EQ(n1Cond->opCount(), 2);
  EXPECT_EQ(n1Cond->operand(0), n1CounterPhi);
  EXPECT_EQ(n1Cond->operand(1), loopLastCount);
  EXPECT_EQ(n1Cond->usersCount(), 1);
  EXPECT_EQ(n1Cond->users().count(n1If), 1);
  // end
  EXPECT_EQ(F.getEnd()->opCount(), 1);
  EXPECT_EQ(F.getEnd()->operand(0), ifTrue);
  EXPECT_EQ(F.getEnd()->usersCount(), 0);

  return fPtr;
}

TEST(Function, FunctionTest0) {
  auto &&fPtr = buildTest0();
  auto &F = *fPtr;

  // verify
  EXPECT_TRUE(F.verify());
}

TEST(Function, FunctionTest1) {
  BUILD_GRAPH_TEST1(Func);

  // verify
  EXPECT_TRUE(Func.verify());
}

TEST(Function, dfs) {
  auto &&fPtr = buildTest0();
  auto &F = *fPtr;

  auto &&[order, dfsParents, dfsNumbers, rpo] = F.dfs();
  EXPECT_EQ(order.size(), dfsParents.size());
  EXPECT_EQ(order[0], F.getStart());
  EXPECT_EQ(dfsParents[0], 0);
}

TEST(Function, idom) {
  FunctionType fnTy(ValueType::Void, {ValueType::Int32});
  auto &&fPtr = std::make_unique<Function>("Test1", fnTy);
  auto &F = *fPtr;

  //   A1
  //  / \
  // B2   C5
  //  \ / \
  //   D3<--F6
  //   |
  //   E4
  auto StartJmp = F.create<JmpNode>(F.getStart());

  auto A = F.create<RegionNode>();
  A->addCFInput(StartJmp);
  auto AIf = F.create<IfNode>(A, F.create<ConstantNode>(ValueType::Int1, 1));
  auto AIfTrue = F.create<IfTrueNode>(AIf);
  auto AIfFalse = F.create<IfFalseNode>(AIf);

  auto B = F.create<RegionNode>();
  B->addCFInput(AIfTrue);
  auto BJmp = F.create<JmpNode>(B);

  auto C = F.create<RegionNode>();
  C->addCFInput(AIfFalse);

  auto CIf = F.create<IfNode>(C, F.create<ConstantNode>(ValueType::Int1, 1));
  auto CIfTrue = F.create<IfTrueNode>(CIf);
  auto CIfFalse = F.create<IfFalseNode>(CIf);

  auto FR = F.create<RegionNode>();
  FR->addCFInput(CIfFalse);
  auto FRJmp = F.create<JmpNode>(FR);

  auto D = F.create<RegionNode>();
  D->addCFInput(CIfTrue);
  D->addCFInput(BJmp);
  D->addCFInput(FRJmp);
  auto DJmp = F.create<JmpNode>(D);

  // final
  F.getEnd()->addCFInput(DJmp);

  EXPECT_TRUE(F.verify());

  auto &&dfsResult = F.dfs();
  auto &&semi = F.semiDominators(dfsResult);
  auto &&idom = F.iDominators(semi);

  std::vector<RegionNodeBase *> refOrder = {F.getStart(), A, B, D,
                                            F.getEnd(),   C, FR};
  auto &&[order, dfsParents, dfsNumbers, rpo] = dfsResult;
  EXPECT_EQ(order.size(), refOrder.size());
  EXPECT_EQ(order, refOrder);

  // get pointers to idom
  std::vector<Node *> refIDom = {F.getStart(), F.getStart(), A, A, D, A, C};
  std::vector<Node *> obtaindeIDom(idom.size());
  std::transform(idom.begin(), idom.end(), obtaindeIDom.begin(),
                 [ord = &order](auto id) { return ord->at(id); });

  EXPECT_EQ(obtaindeIDom.size(), refIDom.size());
  EXPECT_EQ(obtaindeIDom, refIDom);
  EXPECT_EQ(semi, idom);
}

TEST(Function, idom_test1) {
  BUILD_GRAPH_TEST1(Func);
  EXPECT_TRUE(Func.verify());

  auto &&dfsResult = Func.dfs();
  auto &&semi = Func.semiDominators(dfsResult);
  auto &&idom = Func.iDominators(semi);

  auto &&[order, dfsParents, dfsNumbers, rpo] = dfsResult;

  const std::vector<RegionNodeBase *> orderRef = {A, B, C, Func.getEnd(),
                                                  F, E, G};
  EXPECT_EQ(orderRef, order);

  const std::vector<Node *> idomRef{A, A, B, B, B, F, F};
  std::vector<Node *> obtaindeIDom(idom.size());
  std::transform(idom.begin(), idom.end(), obtaindeIDom.begin(),
                 [ord = &order](auto id) { return ord->at(id); });
  EXPECT_EQ(idomRef, obtaindeIDom);

  // rpo check
  const std::vector<RegionNodeBase *> rpoRef = {D, C, E, G, F, B, A};
  EXPECT_EQ(rpoRef, rpo);
}

TEST(Function, idom_test2) {
  BUILD_GRAPH_TEST2(Func);

  EXPECT_TRUE(Func.verify());

  auto &&dfsResult = Func.dfs();
  auto &&semi = Func.semiDominators(dfsResult);
  auto &&idom = Func.iDominators(semi);

  auto &&[order, dfsParents, dfsNumbers, rpo] = dfsResult;

  const std::vector<RegionNodeBase *> orderRef = {A, B, C, D, E, F,
                                                  G, H, I, K, J};
  EXPECT_EQ(orderRef, order);

  const std::vector<Node *> idomRef{A, A, B, C, D, E, F, G, G, I, B};
  std::vector<Node *> obtaindeIDom(idom.size());
  std::transform(idom.begin(), idom.end(), obtaindeIDom.begin(),
                 [ord = &order](auto id) { return ord->at(id); });
  EXPECT_EQ(idomRef, obtaindeIDom);

  // rpo check
  const std::vector<RegionNodeBase *> rpoRef = {H, K, I, G, F, E,
                                                D, C, J, B, A};
  EXPECT_EQ(rpoRef, rpo);
}

TEST(Function, idom_test3) {
  BUILD_GRAPH_TEST3(Func);

  EXPECT_TRUE(Func.verify());

  auto &&dfsResult = Func.dfs();
  auto &&semi = Func.semiDominators(dfsResult);
  auto &&idom = Func.iDominators(semi);

  auto &&[order, dfsParents, dfsNumbers, rpo] = dfsResult;

  const std::vector<RegionNodeBase *> orderRef = {A, B, C, D, G, I, E, F, H};
  EXPECT_EQ(orderRef, order);

  const std::vector<Node *> idomRef{A, A, B, B, B, B, B, E, F};
  std::vector<Node *> obtaindeIDom(idom.size());
  std::transform(idom.begin(), idom.end(), obtaindeIDom.begin(),
                 [ord = &order](auto id) { return ord->at(id); });
  EXPECT_EQ(idomRef, obtaindeIDom);

  // rpo check
  const std::vector<RegionNodeBase *> rpoRef = {I, G, D, C, H, F, E, B, A};
  EXPECT_EQ(rpoRef, rpo);
}

TEST(Function, line_test1) {
  BUILD_GRAPH_TEST1(Func);
  EXPECT_TRUE(Func.verify());

  auto &&line = Func.linearilize();
}
