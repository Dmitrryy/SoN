#include "src/Function.hpp"

using namespace son;

int main () {
  FunctionType fnTy(ValueType::Void, {});
  Function F("Test0", fnTy);


  auto n0 = F.getStart();
  auto n2 = F.getEnd();

  auto range01 = F.createNode<RegionNode>();
  range01->addCFInput(n0);

  auto n1Counter0 = F.createNode<ConstantNode>(ValueType::Int32, 0);
  auto n1CounterPhi = F.createNode<PhiNode>(range01, 2, ValueType::Int32);
  n1CounterPhi->setVal(0, n1Counter0);
  auto n1Counter3 = F.createNode<AddNode>(
      n1CounterPhi, F.createNode<ConstantNode>(ValueType::Int32, 1));
  n1CounterPhi->setVal(1, n1Counter3);

  // cond (i == 10)
  auto loopLastCount = F.createNode<ConstantNode>(ValueType::Int32, 10);
  auto n1Cond = F.createNode<CmpEQNode>(n1CounterPhi, loopLastCount);

  // if (i == 10)
  auto n1If = F.createNode<IfNode>(range01, n1Cond);
  auto ifTrue = F.createNode<IfTrueNode>(n1If);
  auto ifFalse = F.createNode<IfFalseNode>(n1If);
  auto loopBodyRegion = F.createNode<RegionNode>();
  loopBodyRegion->addCFInput(ifFalse);
  // Jmp loopBody -> loopHeader
  auto backEdge = F.createNode<JmpNode>(loopBodyRegion);
  range01->addCFInput(backEdge);

  // final
  F.getEnd()->addCFInput(ifTrue);

  // verify
  F.verify();

  return 0;
}