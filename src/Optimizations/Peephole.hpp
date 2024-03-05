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
    auto &&workStack = _initialWorkList(F);

    while (!workStack.empty()) {
      Node *node = workStack.back();
      workStack.pop_back();

      if (isa<AddNode>(node)) {
        _tryAdd(static_cast<AddNode *>(node));
      } else if (isa<ShlNode>(node)) {
        _tryShl(static_cast<ShlNode *>(node));
      } else if (isa<XorNode>(node)) {
        _tryXor(static_cast<XorNode *>(node));
      }
    }
  }

private:
  std::vector<Node *> _initialWorkList(Function &F) {
    std::vector<Node *> res;

    for (auto &&node : F) {
      if (isa<AddNode, ShlNode, XorNode>(node)) {
        res.push_back(node.get());
      }
    }

    return res;
  }

  bool _tryAdd(AddNode *node) {
    if (node->usersCount() == 0) {
      return false;
    }

    // add chain: combining constants
    //
    // V1 = add V2, Const1
    // V2 = add V1, Const2
    // ->
    // V1 = add Const1, Const2
    // V2 = add V2, V1
    // NOTE: check that there is not V1 users except next add

    std::vector<Node *> addChain;
    addChain.reserve(4);

    AddNode *nextAdd = nullptr;
    ConstantNode *curConst = nullptr;
    size_t curConstIdx = 0;
    if (isa<AddNode>(node->operand(0)) && isa<ConstantNode>(node->operand(1))) {
      nextAdd = static_cast<AddNode *>(node->operand(0));
      curConst = static_cast<ConstantNode *>(node->operand(1));
      curConstIdx = 1;
    } else if (isa<AddNode>(node->operand(1)) &&
               isa<ConstantNode>(node->operand(0))) {
      nextAdd = static_cast<AddNode *>(node->operand(1));
      curConst = static_cast<ConstantNode *>(node->operand(0));
      curConstIdx = 0;
    }

    if (!nextAdd) {
      return false;
    }

    Node *nonConst = nullptr;
    size_t nonConstIdx = 0;
    ConstantNode *nextConst = nullptr;
    if (!isa<ConstantNode>(nextAdd->operand(0)) &&
        isa<ConstantNode>(nextAdd->operand(1))) {
      nonConst = nextAdd->operand(0);
      nonConstIdx = 0;
      nextConst = static_cast<ConstantNode *>(nextAdd->operand(1));
    } else if (!isa<ConstantNode>(nextAdd->operand(1)) &&
               isa<ConstantNode>(nextAdd->operand(0))) {
      nonConst = nextAdd->operand(1);
      nonConstIdx = 1;
      nextConst = static_cast<ConstantNode *>(nextAdd->operand(0));
    }

    if (!nonConst) {
      return false;
    }

    // check users
    if (nextAdd->usersCount() > 1) {
      return false;
    }

    // reorder
    nextAdd->setOperand(nonConstIdx, curConst);
    node->setOperand(curConstIdx, nonConst);
    return true;
  }

  bool _tryShl(ShlNode *node) {
    // TODO
    return false;
  }
  bool _tryXor(XorNode *node) {
    // TODO
    return false;
  }
};

} // namespace son