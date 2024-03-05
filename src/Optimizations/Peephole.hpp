#pragma once

#include "Function.hpp"
#include "Node.hpp"

#include <algorithm>
#include <stdexcept>
#include <unordered_set>
#include <vector>

namespace son {

class Peephole {

public:
  // Add + Shl + Xor
  void run(Function &F) {
    _canonicalize(F);

    auto &&workStack = _initialWorkList(F);

    while (!workStack.empty()) {
      Node *node = workStack.back();
      workStack.pop_back();

      if (node->usersCount() == 0) {
        continue;
      }

      if (isa<AddNode>(node)) {
        _tryAdd(F, static_cast<AddNode *>(node));
      } else if (isa<ShlNode>(node)) {
        _tryShl(F, static_cast<ShlNode *>(node));
      } else if (isa<XorNode>(node)) {
        _tryXor(F, static_cast<XorNode *>(node));
      }
    }
  }

private:
  void _canonicalize(Function &F) {
    for (auto &&n : F) {
      auto *node = n.get();
      // set constant to rhs
      if (isa<AddNode, XorNode>(node) && node->usersCount() > 0 &&
          isa<ConstantNode>(node->operand(0)) &&
          !isa<ConstantNode>(node->operand(1))) {
        node->swapOperands(0, 1);
      }
    }
  }

  std::vector<Node *> _initialWorkList(Function &F) {
    std::vector<Node *> res;

    for (auto &&node : F) {
      if (isa<AddNode, ShlNode, XorNode>(node)) {
        res.push_back(node.get());
      }
    }

    return res;
  }

  bool _tryAdd(Function &F, AddNode *node) {
    // Try 0:
    // Add V, 0
    if (isa<ConstantNode>(node->operand(1))) {
      auto *c = static_cast<ConstantNode *>(node->operand(1));
      if (c->getConstant() == 0) {
        node->replaceWith(node->operand(0));
        node->detach();
        return true;
      }
    }

    // Try 1:
    // add chain: combining constants
    //
    // V1 = add V2, Const1
    // V2 = add V1, Const2
    // ->
    // V1 = add Const1, Const2
    // V2 = add V2, V1
    // NOTE: check that there is not V1 users except next add
    if (isa<AddNode>(node->operand(0)) && isa<ConstantNode>(node->operand(1))) {
      auto *nextAdd = static_cast<AddNode *>(node->operand(0));
      auto *curConst = static_cast<ConstantNode *>(node->operand(1));

      // check users
      if (nextAdd->usersCount() > 1) {
        return false;
      }

      if (!isa<ConstantNode>(nextAdd->operand(0)) &&
          isa<ConstantNode>(nextAdd->operand(1))) {
        auto *nonConst = nextAdd->operand(0);

        // reorder
        nextAdd->setOperand(0, curConst);
        node->setOperand(1, nonConst);
        return true;
      }
    }

    // Try 2:
    // add V1, V1
    // ->
    // shl V1, 1
    if (node->operand(0) == node->operand(1)) {
      auto *c1 = F.create<ConstantNode>(node->valueTy(), 1);
      auto *shl = F.create<ShlNode>(node->operand(0), c1);

      node->replaceWith(shl);
      node->detach();
      return true;
    }

    return false;
  }

  bool _tryShl(Function &F, ShlNode *node) {
    // Try 1:
    // Shl V, 0
    // ->
    // V
    if (isa<ConstantNode>(node->operand(1))) {
      auto *c = static_cast<ConstantNode *>(node->operand(1));
      if (c->getConstant() == 0) {
        node->replaceWith(node->operand(0));
        node->detach();
        return true;
      }
    }

    return false;
  }

  bool _tryXor(Function &F, XorNode *node) {
    // Try 1:
    // Xor V, V
    // ->
    // 0
    if (node->operand(0) == node->operand(1)) {
      auto *c0 = F.create<ConstantNode>(node->valueTy(), 0);

      node->replaceWith(c0);
      node->detach();
      return true;
    }

    // Try 2:
    // Xor V, 0
    // ->
    // V
    if (isa<ConstantNode>(node->operand(1))) {
      auto *c = static_cast<ConstantNode *>(node->operand(1));
      if (c->getConstant() == 0) {
        node->replaceWith(node->operand(0));
        node->detach();
        return true;
      }
    }

    return false;
  }
};

} // namespace son