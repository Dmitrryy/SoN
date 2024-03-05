#pragma once

#include "Function.hpp"
#include "Node.hpp"

#include <algorithm>
#include <stdexcept>
#include <unordered_set>
#include <vector>

namespace son {

class ConstantFolding {

public:
  ConstantFolding() = default;

  // Add + Shl + Xor
  void run(Function &F) {
    auto &&workStack = _initialWork(F);
    std::unordered_set<Node *> visited;

    while (!workStack.empty()) {
      Node *node = workStack.back();
      workStack.pop_back();
      if (visited.contains(node)) {
        continue;
      }

      auto *folded = _folde(F, node);
      for (auto &&U : folded->users()) {
        if (canFold(U)) {
          workStack.push_back(U);
        }
      }
      visited.emplace(node);
    }
  }

private:
  std::vector<Node *> _initialWork(Function &F) {
    std::vector<Node *> res;

    for (auto &&n : F) {
      auto *node = n.get();
      if (node->usersCount() > 0 && canFold(node)) {
        res.push_back(node);
      }
    }

    return res;
  }

  ConstantNode *_folde(Function &F, Node *node) {
    // create new node
    auto *folded = F.create<ConstantNode>(node->valueTy(), 0);
    if (isa<AddNode>(node)) {
      auto *lhs = node->operand(0);
      assert(isa<ConstantNode>(lhs));
      auto *rhs = node->operand(1);
      assert(isa<ConstantNode>(rhs));
      folded->setConstant(static_cast<ConstantNode *>(lhs)->getConstant() +
                          static_cast<ConstantNode *>(rhs)->getConstant());
    } else if (isa<ShlNode>(node)) {
      auto *lhs = node->operand(0);
      assert(isa<ConstantNode>(lhs));
      auto *rhs = node->operand(1);
      assert(isa<ConstantNode>(rhs));
      folded->setConstant(static_cast<ConstantNode *>(lhs)->getConstant()
                          << static_cast<ConstantNode *>(rhs)->getConstant());
    } else if (isa<XorNode>(node)) {
      auto *lhs = node->operand(0);
      assert(isa<ConstantNode>(lhs));
      auto *rhs = node->operand(1);
      assert(isa<ConstantNode>(rhs));
      folded->setConstant(static_cast<ConstantNode *>(lhs)->getConstant() ^
                          static_cast<ConstantNode *>(rhs)->getConstant());
    } else {
      throw std::runtime_error("Can't fold node! =(");
    }

    // replace users
    node->replaceWith(folded);
    node->detach();

    return folded;
  }

  bool canFold(Node *node) const {
    if (isa<AddNode, ShlNode, XorNode>(node)) {
      // check constant operands
      if (isa<ConstantNode>(node->operand(0)) &&
          isa<ConstantNode>(node->operand(1))) {
        return true;
      }
    }
    return false;
  }
};

} // namespace son