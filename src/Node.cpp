#include "Node.hpp"

namespace son {

std::vector<PhiNode *> RegionNode::phis() const {
  std::vector<PhiNode *> result;
  for (auto &&U : users()) {
    if (isa<PhiNode>(U)) {
      result.push_back(dynamic_cast<PhiNode *>(U));
    }
  }
  return result;
}

std::vector<CFNode *> RegionNode::successors() const {
  std::vector<CFNode *> result;
  auto Term = terminator();
  for (auto &&U : Term->users()) {
    // expected that all users of terminator is CFNode
    auto *S = dynamic_cast<CFNode *>(U);
    assert(S);
    result.emplace_back(S);
  }
  return result;
}

CFNode *RegionNode::terminator() const {
  for (auto &&Operand : operands()) {
    if (isa<RetNode, JmpNode, IfNode>(Operand)) {
      return dynamic_cast<CFNode *>(Operand);
    }
  }
  return nullptr;
}

} // namespace son