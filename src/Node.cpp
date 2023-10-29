#include "Node.hpp"

namespace son {

std::vector<PhiNode *> RegionNodeBase::phis() const {
  std::vector<PhiNode *> result;
  for (auto &&U : users()) {
    if (isa<PhiNode>(U)) {
      result.push_back(dynamic_cast<PhiNode *>(U));
    }
  }
  return result;
}

std::vector<RegionNodeBase *> RegionNodeBase::successors() const {
  if (isa<EndNode>(this)) {
    return {};
  }
  std::vector<RegionNodeBase *> result;
  auto Term = terminator();
  assert(Term);
  if (isa<JmpNode, RetNode>(Term)) {
    assert(Term->usersCount() == 1);
    auto *S = dynamic_cast<RegionNodeBase *>(*Term->users().begin());
    assert(S);
    result.push_back(S);
  }
  else if (isa<IfNode>(Term)) {
    assert(Term->usersCount() == 2);
    for(auto &&U : Term->users()) {
      assert((isa<IfTrueNode, IfFalseNode>(U)));
      assert(U->usersCount() == 1);
      auto S = dynamic_cast<RegionNodeBase *>(*U->users().begin());
      assert(S);
      result.push_back(S);
    }
  }
  return result;
}

std::vector<RegionNodeBase *> RegionNodeBase::predicessors() const {
  std::vector<RegionNodeBase *> result;

  for (auto &&operand : operands()) {
    if (isa<JmpNode, RetNode>(operand)) {
      assert(operand->opCount() == 1);
      auto *P = dynamic_cast<RegionNodeBase *>(operand->operand(0));
      assert(P);
      result.push_back(P);
    }
    else if (isa<IfTrueNode, IfFalseNode>(operand)) {
      assert(operand->opCount() == 1);
      auto *If = dynamic_cast<IfNode *>(operand->operand(0));
      assert(If);
      result.push_back(If->getInputCF());
    }
  }

  return result;
}

CFNode *RegionNodeBase::terminator() const {
  for (auto &&Operand : users()) {
    if (isa<RetNode, JmpNode, IfNode>(Operand)) {
      return dynamic_cast<CFNode *>(Operand);
    }
  }
  return nullptr;
}

} // namespace son